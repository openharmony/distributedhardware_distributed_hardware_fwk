/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "av_sync_manager.h"

#include <set>
#include "nlohmann/json.hpp"

#include "av_trans_control_center.h"
#include "av_trans_log.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "AVSyncManager"

AVSyncManager::AVSyncManager()
{
    AVTRANS_LOGI("AVSyncManager ctor.");
    sourceMemory_ = { 0, 0, "" };
    sinkMemory_ = { 0, 0, "" };
}

AVSyncManager::~AVSyncManager()
{
    AVTRANS_LOGI("AVSyncManager dctor.");
    streamInfoList_.clear();
    CloseAVTransSharedMemory(sourceMemory_);
    CloseAVTransSharedMemory(sinkMemory_);
}

void AVSyncManager::AddStreamInfo(const AVStreamInfo &stream)
{
    AVTRANS_LOGI("add new stream info: sceneType=%s, peerDevId=%s", stream.sceneType.c_str(),
        GetAnonyString(stream.peerDevId).c_str());
    {
        std::lock_guard<std::mutex> lock(listMutex_);
        streamInfoList_.push_back(stream);

        if (streamInfoList_.size() < AV_SYNC_STREAM_COUNT) {
            AVTRANS_LOGI("No need enable sender av sync, stream info list size=%zu", streamInfoList_.size());
            return;
        }
    }
    EnableSenderAVSync();
}

void AVSyncManager::RemoveStreamInfo(const AVStreamInfo &stream)
{
    AVTRANS_LOGI("remove stream info: sceneType=%s, peerDevId=%s", stream.sceneType.c_str(),
        GetAnonyString(stream.peerDevId).c_str());
    {
        std::lock_guard<std::mutex> lock(listMutex_);
        for (auto iter = streamInfoList_.begin(); iter != streamInfoList_.end();) {
            if (((*iter).sceneType == stream.sceneType) && ((*iter).peerDevId == stream.peerDevId)) {
                iter = streamInfoList_.erase(iter);
            } else {
                iter++;
            }
        }
    }
    DisableSenderAVSync();
}

void AVSyncManager::EnableSenderAVSync()
{
    std::string syncGroupInfo;
    if (!MergeGroupInfo(syncGroupInfo)) {
        AVTRANS_LOGI("No need start av sync.");
        return;
    }
    AVTRANS_LOGI("merged av sync group info=%s", syncGroupInfo.c_str());
    {
        std::lock_guard<std::mutex> lock(listMutex_);
        for (const auto &item : streamInfoList_) {
            auto avMessage = std::make_shared<AVTransMessage>((uint32_t)AVTransTag::START_AV_SYNC,
                syncGroupInfo, item.peerDevId);
            AVTransControlCenter::GetInstance().SendMessage(avMessage);
        }
    }

    sourceMemory_ = CreateAVTransSharedMemory("sourceSharedMemory", sizeof(uint32_t) + sizeof(int64_t));
    AVTransControlCenter::GetInstance().SetParam2Engines(sourceMemory_);
}

void AVSyncManager::DisableSenderAVSync()
{
    {
        std::lock_guard<std::mutex> lock(listMutex_);
        if (streamInfoList_.size() >= AV_SYNC_STREAM_COUNT) {
            AVTRANS_LOGI("Cannot disable sender av sync, stream info list size=%zu", streamInfoList_.size());
            return;
        }
        for (const auto &item : streamInfoList_) {
            auto avMessage = std::make_shared<AVTransMessage>((uint32_t)AVTransTag::STOP_AV_SYNC, "", item.peerDevId);
            AVTransControlCenter::GetInstance().SendMessage(avMessage);
        }
    }
    CloseAVTransSharedMemory(sourceMemory_);
}

void AVSyncManager::HandleAvSyncMessage(const std::shared_ptr<AVTransMessage> &message)
{
    if (message->type_ == (uint32_t)AVTransTag::START_AV_SYNC) {
        EnableReceiverAVSync(message->content_);
    } else if (message->type_ == (uint32_t)AVTransTag::STOP_AV_SYNC) {
        DisableReceiverAVSync(message->content_);
    }
}

void AVSyncManager::EnableReceiverAVSync(const std::string &groupInfo)
{
    size_t size = (sizeof(uint32_t) + sizeof(int64_t)) * MAX_CLOCK_UNIT_COUNT;
    sinkMemory_ = CreateAVTransSharedMemory("sinkSharedMemory", size);

    AVTransControlCenter::GetInstance().SetParam2Engines(sinkMemory_);
    AVTransControlCenter::GetInstance().SetParam2Engines(AVTransTag::START_AV_SYNC, groupInfo);
}

void AVSyncManager::DisableReceiverAVSync(const std::string &groupInfo)
{
    CloseAVTransSharedMemory(sinkMemory_);
    AVTransControlCenter::GetInstance().SetParam2Engines(AVTransTag::STOP_AV_SYNC, groupInfo);
}

bool AVSyncManager::MergeGroupInfo(std::string &syncGroupInfo)
{
    std::set<std::string> sceneTypeSet;
    {
        std::lock_guard<std::mutex> lock(listMutex_);
        for (const auto &item : streamInfoList_) {
            sceneTypeSet.insert(item.sceneType);
        }
    }
    if (sceneTypeSet.size() < AV_SYNC_STREAM_COUNT) {
        AVTRANS_LOGI("Can not merge av sync group info, because scene type count less than threshold.");
        return false;
    }

    if ((sceneTypeSet.find(SCENE_TYPE_D_MIC) != sceneTypeSet.end()) &&
        (sceneTypeSet.find(SCENE_TYPE_D_SPEAKER) != sceneTypeSet.end())) {
        AVTRANS_LOGI("Can not merge av sync group info, because scene type are conflicting.");
        return false;
    }

    bool source2Sink = (sceneTypeSet.find(SCENE_TYPE_D_SCREEN) != sceneTypeSet.end()) &&
        (sceneTypeSet.find(SCENE_TYPE_D_SPEAKER) != sceneTypeSet.end());
    bool sink2Source = (sceneTypeSet.find(SCENE_TYPE_D_CAMERA_STR) != sceneTypeSet.end()) &&
        (sceneTypeSet.find(SCENE_TYPE_D_MIC) != sceneTypeSet.end());
    if (!source2Sink && !sink2Source) {
        AVTRANS_LOGI("Can not merge av sync group info, because scene type do not meet conditions.");
        return false;
    }

    std::set<std::string> groupInfoSet;
    for (const auto &item : streamInfoList_) {
        if ((item.sceneType == SCENE_TYPE_D_MIC) || (item.sceneType == SCENE_TYPE_D_SPEAKER)) {
            nlohmann::json masterStr;
            masterStr[KEY_SCENE_TYPE] = item.sceneType;
            masterStr[KEY_PEER_DEV_ID] = item.peerDevId;
            masterStr[KEY_START_FRAME_NUM] = 0;
            masterStr[KEY_AV_SYNC_FLAG] = AvSyncFlag::MASTER;
            groupInfoSet.insert(masterStr.dump());
        } else if ((item.sceneType == SCENE_TYPE_D_SCREEN) || (item.sceneType == SCENE_TYPE_D_CAMERA_STR)) {
            nlohmann::json slaveStr;
            slaveStr[KEY_SCENE_TYPE] = item.sceneType;
            slaveStr[KEY_PEER_DEV_ID] = item.peerDevId;
            slaveStr[KEY_START_FRAME_NUM] = 0;
            slaveStr[KEY_AV_SYNC_FLAG] = AvSyncFlag::SLAVE;
            groupInfoSet.insert(slaveStr.dump());
        } else {
            continue;
        }
    }

    nlohmann::json jsonStr = {
        { KEY_MY_DEV_ID, "" },
        { KEY_GROUP_INFO_ARRAY, groupInfoSet },
    };
    syncGroupInfo = jsonStr.dump();
    return true;
}
}
}