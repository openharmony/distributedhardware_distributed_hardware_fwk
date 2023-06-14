/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "softbus_channel_adapter.h"

#include <securec.h>
#include <thread>
#include <unistd.h>

#include "av_trans_constants.h"
#include "av_trans_errno.h"
#include "av_trans_log.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(SoftbusChannelAdapter);

static int32_t OnSessionOpened(int32_t sessionId, int32_t result)
{
    return SoftbusChannelAdapter::GetInstance().OnSoftbusChannelOpened(sessionId, result);
}

static void OnSessionClosed(int32_t sessionId)
{
    SoftbusChannelAdapter::GetInstance().OnSoftbusChannelClosed(sessionId);
}

static void OnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    SoftbusChannelAdapter::GetInstance().OnSoftbusBytesReceived(sessionId, data, dataLen);
}

static void OnStreamReceived(int32_t sessionId, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *frameInfo)
{
    SoftbusChannelAdapter::GetInstance().OnSoftbusStreamReceived(sessionId, data, ext, frameInfo);
}

static void onTimeSyncResult(const TimeSyncResultInfo *info, int32_t result)
{
    SoftbusChannelAdapter::GetInstance().OnSoftbusTimeSyncResult(info, result);
}

SoftbusChannelAdapter::SoftbusChannelAdapter()
{
    DHLOGI("SoftbusChannelAdapter ctor.");
    sessListener_.OnSessionOpened = OnSessionOpened;
    sessListener_.OnSessionClosed = OnSessionClosed;
    sessListener_.OnBytesReceived = OnBytesReceived;
    sessListener_.OnStreamReceived = OnStreamReceived;
    sessListener_.OnMessageReceived = nullptr;
    sessListener_.OnQosEvent = nullptr;
}

SoftbusChannelAdapter::~SoftbusChannelAdapter()
{
    DHLOGI("~SoftbusChannelAdapter dctor.");
    listenerMap_.clear();
    sessionNameSet_.clear();
    timeSyncSessNames_.clear();
    devId2SessIdMap_.clear();
}

int32_t SoftbusChannelAdapter::CreateChannelServer(const std::string& ownerName, const std::string &sessName)
{
    DHLOGI("Create session server for sessionName:%s.", sessName.c_str());
    TRUE_RETURN_V_MSG_E(ownerName.empty(), ERR_DH_AVT_INVALID_PARAM, "input ownerName is empty.");
    TRUE_RETURN_V_MSG_E(sessName.empty(), ERR_DH_AVT_INVALID_PARAM, "input sessionName is empty.");

    std::lock_guard<std::mutex> setLock(name2IdMtx_);
    if (sessionNameSet_.find(sessName) == sessionNameSet_.end()) {
        int32_t ret = CreateSessionServer(ownerName.c_str(), sessName.c_str(), &sessListener_);
        TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_CREATE_CHANNEL_FAILED, "create session server failed");
    } else {
        DHLOGE("Session already create for name:%s", sessName.c_str());
    }
    sessionNameSet_.insert(sessName);

    DHLOGI("Create session server success for sessionName:%s.", sessName.c_str());
    return DH_AVT_SUCCESS;
}

int32_t SoftbusChannelAdapter::RemoveChannelServer(const std::string& ownerName, const std::string &sessName)
{
    DHLOGI("Remove session server for sessionName:%s.", sessName.c_str());
    TRUE_RETURN_V_MSG_E(ownerName.empty(), ERR_DH_AVT_INVALID_PARAM, "input ownerName is empty.");
    TRUE_RETURN_V_MSG_E(sessName.empty(), ERR_DH_AVT_INVALID_PARAM, "input sessionName is empty.");

    std::lock_guard<std::mutex> setLock(name2IdMtx_);
    if (sessionNameSet_.find(sessName) == sessionNameSet_.end()) {
        DHLOGE("Can not found session name:%s", sessName.c_str());
        return ERR_DH_AVT_INVALID_PARAM_VALUE;
    }
    RemoveSessionServer(ownerName.c_str(), sessName.c_str());
    sessionNameSet_.erase(sessName);

    DHLOGI("Remove session server success for sessionName:%s.", sessName.c_str());
    return DH_AVT_SUCCESS;
}

int32_t SoftbusChannelAdapter::OpenSoftbusChannel(const std::string& mySessName, const std::string &peerSessName,
    const std::string &peerDevId)
{
    DHLOGI("Open softbus channel for mySessName:%s, peerSessName:%s, peerDevId:%s.",
        mySessName.c_str(), peerSessName.c_str(), GetAnonyString(peerDevId).c_str());
    TRUE_RETURN_V_MSG_E(mySessName.empty(), ERR_DH_AVT_INVALID_PARAM, "input mySessName is empty.");
    TRUE_RETURN_V_MSG_E(peerSessName.empty(), ERR_DH_AVT_INVALID_PARAM, "input peerSessName is empty.");
    TRUE_RETURN_V_MSG_E(peerDevId.empty(), ERR_DH_AVT_INVALID_PARAM, "input peerDevId is empty.");

    int dataType = TYPE_BYTES;
    int streamType = INVALID;
    if (mySessName.find(SENDER_DATA_SESSION_NAME_SUFFIX) != std::string::npos) {
        dataType = TYPE_STREAM;
        streamType = COMMON_VIDEO_STREAM;
    }

    SessionAttribute attr = { 0 };
    attr.dataType = dataType;
    attr.linkTypeNum = LINK_TYPE_MAX;
    LinkType linkTypeList[LINK_TYPE_MAX] = {
        LINK_TYPE_WIFI_P2P,
        LINK_TYPE_WIFI_WLAN_5G,
        LINK_TYPE_WIFI_WLAN_2G,
        LINK_TYPE_BR,
    };
    int32_t ret = memcpy_s(attr.linkType, sizeof(attr.linkType), linkTypeList, sizeof(linkTypeList));
    if (ret != EOK) {
        DHLOGE("Data copy failed.");
        return ERR_DH_AVT_NO_MEMORY;
    }
    attr.attr.streamAttr.streamType = streamType;
    int32_t sessionId = OpenSession(mySessName.c_str(), peerSessName.c_str(), peerDevId.c_str(), "0", &attr);
    if (sessionId < 0) {
        DHLOGE("Open softbus session failed for sessionId:%" PRId32, sessionId);
        return ERR_DH_AVT_CHANNEL_ERROR;
    }
    {
        std::lock_guard<std::mutex> lock(idMapMutex_);
        devId2SessIdMap_.insert(std::make_pair(mySessName + "_" + peerDevId, sessionId));
    }
    DHLOGI("Open softbus channel finished, sessionId:%" PRId32, sessionId);
    return DH_AVT_SUCCESS;
}

int32_t SoftbusChannelAdapter::CloseSoftbusChannel(const std::string& sessName, const std::string &peerDevId)
{
    DHLOGI("Close softbus channel for sessName:%s, peerDevId:%s.", sessName.c_str(), GetAnonyString(peerDevId).c_str());
    TRUE_RETURN_V_MSG_E(sessName.empty(), ERR_DH_AVT_INVALID_PARAM, "input sessName is empty.");
    TRUE_RETURN_V_MSG_E(peerDevId.empty(), ERR_DH_AVT_INVALID_PARAM, "input peerDevId is empty.");

    int32_t sessionId = GetSessIdBySessName(sessName, peerDevId);
    CloseSession(sessionId);
    {
        std::lock_guard<std::mutex> lock(idMapMutex_);
        devId2SessIdMap_.erase(sessName + "_" + peerDevId);
    }

    DHLOGI("Close softbus channel success, sessionId:%" PRId32, sessionId);
    return DH_AVT_SUCCESS;
}

int32_t SoftbusChannelAdapter::SendBytesData(const std::string& sessName, const std::string &peerDevId,
    const std::string &data)
{
    DHLOGI("Send bytes data for sessName:%s, peerDevId:%s.", sessName.c_str(), GetAnonyString(peerDevId).c_str());
    TRUE_RETURN_V_MSG_E(sessName.empty(), ERR_DH_AVT_INVALID_PARAM, "input sessName is empty.");
    TRUE_RETURN_V_MSG_E(peerDevId.empty(), ERR_DH_AVT_INVALID_PARAM, "input peerDevId is empty.");
    TRUE_RETURN_V_MSG_E(data.empty(), ERR_DH_AVT_INVALID_PARAM, "input data string is empty.");

    int32_t ret = SendBytes(GetSessIdBySessName(sessName, peerDevId), data.c_str(), strlen(data.c_str()));
    if (ret != DH_AVT_SUCCESS) {
        DHLOGE("Send bytes data failed ret:%" PRId32, ret);
        return ERR_DH_AVT_SEND_DATA_FAILED;
    }
    return DH_AVT_SUCCESS;
}

int32_t SoftbusChannelAdapter::SendStreamData(const std::string& sessName, const std::string &peerDevId,
    const StreamData *data, const StreamData *ext)
{
    DHLOGI("Send stream data for sessName:%s, peerDevId:%s.", sessName.c_str(), GetAnonyString(peerDevId).c_str());
    TRUE_RETURN_V_MSG_E(sessName.empty(), ERR_DH_AVT_INVALID_PARAM, "input sessName is empty.");
    TRUE_RETURN_V_MSG_E(peerDevId.empty(), ERR_DH_AVT_INVALID_PARAM, "input peerDevId is empty.");
    TRUE_RETURN_V_MSG_E(data == nullptr, ERR_DH_AVT_INVALID_PARAM, "input data is nullptr.");
    TRUE_RETURN_V_MSG_E(ext == nullptr, ERR_DH_AVT_INVALID_PARAM, "input ext data is nullptr.");

    StreamFrameInfo frameInfo = {0};
    int32_t ret = SendStream(GetSessIdBySessName(sessName, peerDevId), data, ext, &frameInfo);
    if (ret != DH_AVT_SUCCESS) {
        DHLOGE("Send stream data failed ret:%" PRId32, ret);
        return ERR_DH_AVT_SEND_DATA_FAILED;
    }
    return DH_AVT_SUCCESS;
}

int32_t SoftbusChannelAdapter::RegisterChannelListener(const std::string& sessName, const std::string &peerDevId,
    ISoftbusChannelListener *listener)
{
    DHLOGI("Register channel listener for sessName:%s, peerDevId:%s.",
        sessName.c_str(), GetAnonyString(peerDevId).c_str());
    TRUE_RETURN_V_MSG_E(sessName.empty(), ERR_DH_AVT_INVALID_PARAM, "input sessName is empty.");
    TRUE_RETURN_V_MSG_E(peerDevId.empty(), ERR_DH_AVT_INVALID_PARAM, "input peerDevId is empty.");
    TRUE_RETURN_V_MSG_E(listener == nullptr, ERR_DH_AVT_INVALID_PARAM, "input callback is nullptr.");

    std::lock_guard<std::mutex> lock(listenerMtx_);
    listenerMap_.insert(std::make_pair(sessName + "_" + peerDevId, listener));

    return DH_AVT_SUCCESS;
}

int32_t SoftbusChannelAdapter::UnRegisterChannelListener(const std::string& sessName, const std::string &peerDevId)
{
    DHLOGI("Unregister channel listener for peerDeviceId:%s.", GetAnonyString(peerDevId).c_str());
    TRUE_RETURN_V_MSG_E(sessName.empty(), ERR_DH_AVT_INVALID_PARAM, "input sessName is empty.");
    TRUE_RETURN_V_MSG_E(peerDevId.empty(), ERR_DH_AVT_INVALID_PARAM, "input peerDevId is empty.");

    std::lock_guard<std::mutex> lock(listenerMtx_);
    listenerMap_.erase(sessName + "_" + peerDevId);

    return DH_AVT_SUCCESS;
}

int32_t SoftbusChannelAdapter::StartDeviceTimeSync(const std::string &ownerName, const std::string& sessName,
    const std::string &peerDevId)
{
    DHLOGI("Start device time sync for peerDeviceId:%s.", GetAnonyString(peerDevId).c_str());
    TRUE_RETURN_V_MSG_E(peerDevId.empty(), ERR_DH_AVT_INVALID_PARAM, "input peerDevId is empty.");

    ITimeSyncCb timeSyncCbk = {.onTimeSyncResult = onTimeSyncResult};
    int32_t ret = StartTimeSync(ownerName.c_str(), peerDevId.c_str(), TimeSyncAccuracy::SUPER_HIGH_ACCURACY,
        TimeSyncPeriod::SHORT_PERIOD, &timeSyncCbk);
    if (ret != 0) {
        DHLOGE("StartTimeSync failed ret:%" PRId32, ret);
        return ERR_DH_AVT_TIME_SYNC_FAILED;
    }

    std::lock_guard<std::mutex> lock(timeSyncMtx_);
    timeSyncSessNames_.insert(sessName + "_" + peerDevId);

    return DH_AVT_SUCCESS;
}

int32_t SoftbusChannelAdapter::StopDeviceTimeSync(const std::string &ownerName, const std::string& sessName,
    const std::string &peerDevId)
{
    DHLOGI("Stop device time sync for peerDeviceId:%s.", GetAnonyString(peerDevId).c_str());
    TRUE_RETURN_V_MSG_E(peerDevId.empty(), ERR_DH_AVT_INVALID_PARAM, "input peerDevId is empty.");

    int32_t ret = StopTimeSync(ownerName.c_str(), peerDevId.c_str());
    if (ret != 0) {
        DHLOGE("StopTimeSync failed ret:%" PRId32, ret);
        return ERR_DH_AVT_TIME_SYNC_FAILED;
    }

    std::lock_guard<std::mutex> lock(timeSyncMtx_);
    timeSyncSessNames_.erase(sessName + "_" + peerDevId);

    return DH_AVT_SUCCESS;
}

int32_t SoftbusChannelAdapter::GetSessIdBySessName(const std::string& sessName, const std::string &peerDevId)
{
    std::lock_guard<std::mutex> lock(idMapMutex_);
    std::string idMapKey = sessName + "_" + peerDevId;
    if (devId2SessIdMap_.find(idMapKey) == devId2SessIdMap_.end()) {
        DHLOGE("Can not find sessionId for sessName:%s, peerDevId:%s.",
            sessName.c_str(), GetAnonyString(peerDevId).c_str());
        return -1;
    }
    return devId2SessIdMap_[idMapKey];
}

std::string SoftbusChannelAdapter::GetSessionNameById(int32_t sessionId)
{
    std::lock_guard<std::mutex> lock(idMapMutex_);
    for (auto it = devId2SessIdMap_.begin(); it != devId2SessIdMap_.end(); it++) {
        if (it->second == sessionId) {
            return it->first;
        }
    }

    DHLOGE("No avaliable channel or invalid sessionId:%" PRId32, sessionId);
    return EMPTY_STRING;
}

int32_t SoftbusChannelAdapter::OnSoftbusChannelOpened(int32_t sessionId, int32_t result)
{
    DHLOGI("On softbus channel opened, sessionId:%" PRId32", result:%" PRId32, sessionId, result);

    char peerDevIdChar[MAX_DEVICE_ID_LEN] = "";
    int32_t ret = GetPeerDeviceId(sessionId, peerDevIdChar, sizeof(peerDevIdChar));
    TRUE_RETURN_V_MSG_E(ret != 0, ret, "Get peer device id from softbus failed.");
    std::string peerDevId(peerDevIdChar);

    char sessNameChar[MAX_SESSION_NAME_LEN] = "";
    ret = GetMySessionName(sessionId, sessNameChar, sizeof(sessNameChar));
    TRUE_RETURN_V_MSG_E(ret != 0, ret, "Get my session name from softbus failed.");
    std::string sessName(sessNameChar);

    EventType type = (result == 0) ? EventType::EVENT_CHANNEL_OPENED : EventType::EVENT_CHANNEL_OPEN_FAIL;
    AVTransEvent event = {type, "", peerDevId};

    {
        std::lock_guard<std::mutex> lock(listenerMtx_);
        for (auto it = listenerMap_.begin(); it != listenerMap_.end(); it++) {
            if (((it->first).find(sessName) != std::string::npos) && (it->second != nullptr)) {
                std::thread(&SoftbusChannelAdapter::SendChannelEvent, this, it->second, event).detach();
                {
                    std::lock_guard<std::mutex> lock(idMapMutex_);
                    devId2SessIdMap_.erase(it->first);
                    devId2SessIdMap_.insert(std::make_pair(it->first, sessionId));
                }
            }
        }
    }

    int32_t existSessId = GetSessIdBySessName(sessName, peerDevId);
    if (existSessId == -1) {
        std::lock_guard<std::mutex> lock(idMapMutex_);
        devId2SessIdMap_.insert(std::make_pair(sessName + "_" + peerDevId, sessionId));
    }

    return DH_AVT_SUCCESS;
}

void SoftbusChannelAdapter::OnSoftbusChannelClosed(int32_t sessionId)
{
    DHLOGI("On softbus channel closed, sessionId:%" PRId32, sessionId);
    AVTransEvent event = {EventType::EVENT_CHANNEL_CLOSED, "", ""};

    std::lock_guard<std::mutex> lock(idMapMutex_);
    for (auto it = devId2SessIdMap_.begin(); it != devId2SessIdMap_.end();) {
        if (it->second == sessionId) {
            std::lock_guard<std::mutex> lock(listenerMtx_);
            std::thread(&SoftbusChannelAdapter::SendChannelEvent, this, listenerMap_[it->first], event).detach();
            devId2SessIdMap_.erase(it++);
        }
        it++;
    }
}

void SoftbusChannelAdapter::OnSoftbusBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    DHLOGI("On softbus channel bytes received, sessionId:%" PRId32, sessionId);
    TRUE_RETURN(data == nullptr, "input data is nullptr.");
    TRUE_RETURN(dataLen == 0, "input dataLen is 0.");

    char peerDevIdChar[MAX_DEVICE_ID_LEN] = "";
    int32_t ret = GetPeerDeviceId(sessionId, peerDevIdChar, sizeof(peerDevIdChar));
    TRUE_RETURN(ret != 0, "Get peer device id from softbus failed.");
    std::string peerDevId(peerDevIdChar);

    std::string dataStr = std::string(reinterpret_cast<const char *>(data), dataLen);
    AVTransEvent event = {EventType::EVENT_DATA_RECEIVED, dataStr, peerDevId};

    std::lock_guard<std::mutex> lock(idMapMutex_);
    for (auto it = devId2SessIdMap_.begin(); it != devId2SessIdMap_.end(); it++) {
        if (it->second == sessionId) {
            std::lock_guard<std::mutex> lock(listenerMtx_);
            std::thread(&SoftbusChannelAdapter::SendChannelEvent, this, listenerMap_[it->first], event).detach();
        }
    }
}

void SoftbusChannelAdapter::OnSoftbusStreamReceived(int32_t sessionId, const StreamData *data,
    const StreamData *ext, const StreamFrameInfo *frameInfo)
{
    (void)frameInfo;
    DHLOGI("On softbus channel stream received, sessionId:%" PRId32, sessionId);
    TRUE_RETURN(data == nullptr, "input data is nullptr.");
    TRUE_RETURN(ext == nullptr, "input ext data is nullptr.");

    std::lock_guard<std::mutex> lock(idMapMutex_);
    for (auto it = devId2SessIdMap_.begin(); it != devId2SessIdMap_.end(); it++) {
        if (it->second == sessionId) {
            std::lock_guard<std::mutex> lock(listenerMtx_);
            ISoftbusChannelListener *listener = listenerMap_[it->first];
            TRUE_RETURN(listener == nullptr, "Can not find channel listener.");
            listener->OnStreamReceived(data, ext);
        }
    }
}

void SoftbusChannelAdapter::OnSoftbusTimeSyncResult(const TimeSyncResultInfo *info, int32_t result)
{
    DHLOGI("On softbus channel time sync result:%" PRId32, result);
    TRUE_RETURN(result == 0, "On softbus channel time sync failed");

    int32_t millisecond = info->result.millisecond;
    int32_t microsecond = info->result.microsecond;
    TimeSyncAccuracy accuracy  = info->result.accuracy;
    DHLOGI("Time sync success, flag:%" PRId32", millisecond:%" PRId32", microsecond:%" PRId32", accuracy:%" PRId32,
        info->flag, millisecond, microsecond, accuracy);
    DHLOGI("renguang debug: targetNetworkId:%s, masterNetworkId:%s", info->target.targetNetworkId,
        info->target.masterNetworkId);

    std::string targetDevId(info->target.targetNetworkId);
    std::string masterDevId(info->target.masterNetworkId);
    std::lock_guard<std::mutex> lock(timeSyncMtx_);
    for (auto sessName : timeSyncSessNames_) {
        std::lock_guard<std::mutex> lock(listenerMtx_);
        ISoftbusChannelListener *listener = listenerMap_[sessName];
        if (listener != nullptr) {
            listener->OnChannelEvent({EventType::EVENT_TIME_SYNC_RESULT, std::to_string(millisecond), targetDevId});
        }
    }
}

void SoftbusChannelAdapter::SendChannelEvent(ISoftbusChannelListener *listener, const AVTransEvent &event)
{
    pthread_setname_np(pthread_self(), SEND_CHANNEL_EVENT);

    TRUE_RETURN(listener == nullptr, "input listener is nullptr.");
    listener->OnChannelEvent(event);
}
} // namespace DistributedHardware
} // namespace OHOS