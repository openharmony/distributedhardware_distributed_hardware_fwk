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

#include "cJSON.h"

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
    AVTRANS_LOGI("add new stream info: sceneType=%{public}s, peerDevId=%{public}s", stream.sceneType.c_str(),
        GetAnonyString(stream.peerDevId).c_str());
    {
        std::lock_guard<std::mutex> lock(listMutex_);
        streamInfoList_.push_back(stream);

        if (streamInfoList_.size() < AV_SYNC_STREAM_COUNT) {
            AVTRANS_LOGI("No need enable sender av sync, stream info list size=%{public}zu", streamInfoList_.size());
            return;
        }
    }
    EnableSenderAVSync();
}

void AVSyncManager::RemoveStreamInfo(const AVStreamInfo &stream)
{
    AVTRANS_LOGI("remove stream info: sceneType=%{public}s, peerDevId=%{public}s", stream.sceneType.c_str(),
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
    AVTRANS_LOGI("merged av sync group info=%{public}s", GetAnonyString(syncGroupInfo).c_str());
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
            AVTRANS_LOGI("Cannot disable sender av sync, stream info list size=%{public}zu", streamInfoList_.size());
            return;
        }
        for (const auto &item : streamInfoList_) {
            auto avMessage = std::make_shared<AVTransMessage>((uint32_t)AVTransTag::STOP_AV_SYNC, "", item.peerDevId);
            AVTransControlCenter::GetInstance().SendMessage(avMessage);
        }
    }
    CloseAVTransSharedMemory(sourceMemory_);
    AVTransControlCenter::GetInstance().SetParam2Engines(AVTransSharedMemory{0, 0, "sourceSharedMemory"});
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
    (void)groupInfo;
    CloseAVTransSharedMemory(sinkMemory_);
    AVTransControlCenter::GetInstance().SetParam2Engines(AVTransTag::STOP_AV_SYNC, "");
    AVTransControlCenter::GetInstance().SetParam2Engines(AVTransSharedMemory{0, 0, "sinkSharedMemory"});
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
    if (!MergeGroupInfoInner(groupInfoSet)) {
        return false;
    }
    std::string info = GetsyncGroupInfo(groupInfoSet);
    if (info == "") {
        return false;
    }
    syncGroupInfo = info;
    return true;
}

bool AVSyncManager::MergeGroupInfoInner(std::set<std::string> &groupInfoSet)
{
    for (const auto &item : streamInfoList_) {
        if ((item.sceneType == SCENE_TYPE_D_MIC) || (item.sceneType == SCENE_TYPE_D_SPEAKER)) {
            cJSON *masterStr = cJSON_CreateObject();
            if (masterStr == nullptr) {
                return false;
            }
            cJSON_AddStringToObject(masterStr, KEY_SCENE_TYPE.c_str(), item.sceneType.c_str());
            cJSON_AddStringToObject(masterStr, KEY_PEER_DEV_ID.c_str(), item.peerDevId.c_str());
            cJSON_AddNumberToObject(masterStr, KEY_START_FRAME_NUM.c_str(), 0);
            cJSON_AddNumberToObject(masterStr, KEY_AV_SYNC_FLAG.c_str(), static_cast<uint32_t>(AvSyncFlag::MASTER));
            char *jsonstr = cJSON_PrintUnformatted(masterStr);
            if (jsonstr == nullptr) {
                cJSON_Delete(masterStr);
                return false;
            }
            groupInfoSet.insert(std::string(jsonstr));
            cJSON_Delete(masterStr);
            cJSON_free(jsonstr);
        } else if ((item.sceneType == SCENE_TYPE_D_SCREEN) || (item.sceneType == SCENE_TYPE_D_CAMERA_STR)) {
            cJSON *slaveStr = cJSON_CreateObject();
            if (slaveStr == nullptr) {
                return false;
            }
            cJSON_AddStringToObject(slaveStr, KEY_SCENE_TYPE.c_str(), item.sceneType.c_str());
            cJSON_AddStringToObject(slaveStr, KEY_PEER_DEV_ID.c_str(), item.peerDevId.c_str());
            cJSON_AddNumberToObject(slaveStr, KEY_START_FRAME_NUM.c_str(), 0);
            cJSON_AddNumberToObject(slaveStr, KEY_AV_SYNC_FLAG.c_str(), static_cast<uint32_t>(AvSyncFlag::SLAVE));
            char *jsonstr = cJSON_PrintUnformatted(slaveStr);
            if (jsonstr == nullptr) {
                cJSON_Delete(slaveStr);
                return false;
            }
            groupInfoSet.insert(std::string(jsonstr));
            cJSON_Delete(slaveStr);
            cJSON_free(jsonstr);
        } else {
            continue;
        }
    }
    return true;
}

std::string AVSyncManager::GetsyncGroupInfo(std::set<std::string> &groupInfoSet)
{
    cJSON *jsonStr = cJSON_CreateObject();
    if (jsonStr == nullptr) {
        return "";
    }
    cJSON_AddStringToObject(jsonStr, KEY_MY_DEV_ID.c_str(), "");
    cJSON *array = cJSON_CreateArray();
    if (array == nullptr) {
        cJSON_Delete(jsonStr);
        return "";
    }
    for (auto &info : groupInfoSet) {
        cJSON_AddItemToArray(array, cJSON_CreateString(info.c_str()));
    }
    cJSON_AddItemToObject(jsonStr, KEY_GROUP_INFO_ARRAY.c_str(), array);
    char *data = cJSON_PrintUnformatted(jsonStr);
    if (data == nullptr) {
        cJSON_Delete(jsonStr);
        return "";
    }
    std::string info = std::string(data);
    cJSON_free(data);
    cJSON_Delete(jsonStr);
    return info;
}
}
}