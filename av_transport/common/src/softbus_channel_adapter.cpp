/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include <algorithm>
#include <securec.h>
#include <thread>
#include <unistd.h>

#include "av_trans_constants.h"
#include "av_trans_errno.h"
#include "av_trans_log.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "SoftbusChannelAdapter"

IMPLEMENT_SINGLE_INSTANCE(SoftbusChannelAdapter);

namespace {
const static std::pair<std::string, std::string> LOCAL_TO_PEER_SESSION_NAME_MAP[] = {
    {OWNER_NAME_D_MIC + "_" + SENDER_CONTROL_SESSION_NAME_SUFFIX,
     OWNER_NAME_D_MIC + "_" + RECEIVER_CONTROL_SESSION_NAME_SUFFIX},
    {OWNER_NAME_D_MIC + "_" + RECEIVER_CONTROL_SESSION_NAME_SUFFIX,
     OWNER_NAME_D_MIC + "_" + SENDER_CONTROL_SESSION_NAME_SUFFIX},
    {OWNER_NAME_D_SPEAKER + "_" + SENDER_CONTROL_SESSION_NAME_SUFFIX,
     OWNER_NAME_D_SPEAKER + "_" + RECEIVER_CONTROL_SESSION_NAME_SUFFIX},
    {OWNER_NAME_D_SPEAKER + "_" + RECEIVER_CONTROL_SESSION_NAME_SUFFIX,
     OWNER_NAME_D_SPEAKER + "_" + SENDER_CONTROL_SESSION_NAME_SUFFIX},
    {OWNER_NAME_D_SCREEN + "_" + SENDER_CONTROL_SESSION_NAME_SUFFIX,
     OWNER_NAME_D_SCREEN + "_" + RECEIVER_CONTROL_SESSION_NAME_SUFFIX},
    {OWNER_NAME_D_SCREEN + "_" + RECEIVER_CONTROL_SESSION_NAME_SUFFIX,
     OWNER_NAME_D_SCREEN + "_" + SENDER_CONTROL_SESSION_NAME_SUFFIX},
    {OWNER_NAME_D_VIRMODEM_MIC + "_" + SENDER_CONTROL_SESSION_NAME_SUFFIX,
     OWNER_NAME_D_VIRMODEM_MIC + "_" + RECEIVER_CONTROL_SESSION_NAME_SUFFIX},
    {OWNER_NAME_D_VIRMODEM_MIC + "_" + RECEIVER_CONTROL_SESSION_NAME_SUFFIX,
     OWNER_NAME_D_VIRMODEM_MIC + "_" + SENDER_CONTROL_SESSION_NAME_SUFFIX},
    {OWNER_NAME_D_VIRMODEM_SPEAKER + "_" + SENDER_CONTROL_SESSION_NAME_SUFFIX,
     OWNER_NAME_D_VIRMODEM_SPEAKER + "_" + RECEIVER_CONTROL_SESSION_NAME_SUFFIX},
    {OWNER_NAME_D_VIRMODEM_SPEAKER + "_" + RECEIVER_CONTROL_SESSION_NAME_SUFFIX,
     OWNER_NAME_D_VIRMODEM_SPEAKER + "_" + SENDER_CONTROL_SESSION_NAME_SUFFIX},

    {AV_SYNC_SENDER_CONTROL_SESSION_NAME, AV_SYNC_RECEIVER_CONTROL_SESSION_NAME},
    {AV_SYNC_RECEIVER_CONTROL_SESSION_NAME, AV_SYNC_SENDER_CONTROL_SESSION_NAME},

    {OWNER_NAME_D_MIC + "_" + SENDER_DATA_SESSION_NAME_SUFFIX,
     OWNER_NAME_D_MIC + "_" + RECEIVER_DATA_SESSION_NAME_SUFFIX},
    {OWNER_NAME_D_MIC + "_" + RECEIVER_DATA_SESSION_NAME_SUFFIX,
     OWNER_NAME_D_MIC + "_" + SENDER_DATA_SESSION_NAME_SUFFIX},
    {OWNER_NAME_D_SPEAKER + "_" + SENDER_DATA_SESSION_NAME_SUFFIX,
     OWNER_NAME_D_SPEAKER + "_" + RECEIVER_DATA_SESSION_NAME_SUFFIX},
    {OWNER_NAME_D_SPEAKER + "_" + RECEIVER_DATA_SESSION_NAME_SUFFIX,
     OWNER_NAME_D_SPEAKER + "_" + SENDER_DATA_SESSION_NAME_SUFFIX},
    {OWNER_NAME_D_SCREEN + "_" + SENDER_DATA_SESSION_NAME_SUFFIX,
     OWNER_NAME_D_SCREEN + "_" + RECEIVER_DATA_SESSION_NAME_SUFFIX},
    {OWNER_NAME_D_SCREEN + "_" + RECEIVER_DATA_SESSION_NAME_SUFFIX,
     OWNER_NAME_D_SCREEN + "_" + SENDER_DATA_SESSION_NAME_SUFFIX},
    {OWNER_NAME_D_VIRMODEM_MIC + "_" + SENDER_DATA_SESSION_NAME_SUFFIX,
     OWNER_NAME_D_VIRMODEM_MIC + "_" + RECEIVER_DATA_SESSION_NAME_SUFFIX},
    {OWNER_NAME_D_VIRMODEM_MIC + "_" + RECEIVER_DATA_SESSION_NAME_SUFFIX,
     OWNER_NAME_D_VIRMODEM_MIC + "_" + SENDER_DATA_SESSION_NAME_SUFFIX},
    {OWNER_NAME_D_VIRMODEM_SPEAKER + "_" + SENDER_DATA_SESSION_NAME_SUFFIX,
     OWNER_NAME_D_VIRMODEM_SPEAKER + "_" + RECEIVER_DATA_SESSION_NAME_SUFFIX},
    {OWNER_NAME_D_VIRMODEM_SPEAKER + "_" + RECEIVER_DATA_SESSION_NAME_SUFFIX,
     OWNER_NAME_D_VIRMODEM_SPEAKER + "_" + SENDER_DATA_SESSION_NAME_SUFFIX},
};
} // namespace

static void OnSessionOpened(int32_t sessionId, PeerSocketInfo info)
{
    std::string peerDevId(info.networkId);
    std::string peerSessionName(info.name);
    SoftbusChannelAdapter::GetInstance().OnSoftbusChannelOpened(peerSessionName, sessionId, peerDevId, 0);
}

static void OnSessionClosed(int32_t sessionId, ShutdownReason reason)
{
    SoftbusChannelAdapter::GetInstance().OnSoftbusChannelClosed(sessionId, reason);
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

static void onDevTimeSyncResult(const TimeSyncResultInfo *info, int32_t result)
{
    SoftbusChannelAdapter::GetInstance().OnSoftbusTimeSyncResult(info, result);
}

SoftbusChannelAdapter::SoftbusChannelAdapter()
{
    sessListener_.OnBind = OnSessionOpened;
    sessListener_.OnShutdown = OnSessionClosed;
    sessListener_.OnBytes = OnBytesReceived;
    sessListener_.OnStream = OnStreamReceived;
    sessListener_.OnMessage = nullptr;
    sessListener_.OnFile = nullptr;
    sessListener_.OnQos = nullptr;
    sessListener_.OnError = nullptr;
    sessListener_.OnNegotiate = nullptr;
}

SoftbusChannelAdapter::~SoftbusChannelAdapter()
{
    listenerMap_.clear();
    timeSyncSessNames_.clear();
    devId2SessIdMap_.clear();
    serverMap_.clear();
}

std::string SoftbusChannelAdapter::TransName2PkgName(const std::string &ownerName)
{
    const static std::pair<std::string, std::string> mapArray[] = {
        {OWNER_NAME_D_MIC, PKG_NAME_D_AUDIO},
        {OWNER_NAME_D_VIRMODEM_MIC, PKG_NAME_D_CALL},
        {OWNER_NAME_D_CAMERA, PKG_NAME_D_CAMERA},
        {OWNER_NAME_D_SCREEN, PKG_NAME_D_SCREEN},
        {OWNER_NAME_D_SPEAKER, PKG_NAME_D_AUDIO},
        {OWNER_NAME_D_VIRMODEM_SPEAKER, PKG_NAME_D_CALL},
        {AV_SYNC_SENDER_CONTROL_SESSION_NAME, PKG_NAME_DH_FWK},
        {AV_SYNC_RECEIVER_CONTROL_SESSION_NAME, PKG_NAME_DH_FWK},
    };
    auto foundItem = std::find_if(std::begin(mapArray), std::end(mapArray),
        [&](const auto& item) { return item.first == ownerName; });
    if (foundItem != std::end(mapArray)) {
        return foundItem->second;
    }
    return EMPTY_STRING;
}

std::string SoftbusChannelAdapter::FindSessNameByPeerSessName(const std::string peerSessionName)
{
    auto foundItem = std::find_if(std::begin(LOCAL_TO_PEER_SESSION_NAME_MAP), std::end(LOCAL_TO_PEER_SESSION_NAME_MAP),
        [&](const auto& item) { return item.first == peerSessionName; });
    if (foundItem != std::end(LOCAL_TO_PEER_SESSION_NAME_MAP)) {
        return foundItem->second;
    }
    return EMPTY_STRING;
}

int32_t SoftbusChannelAdapter::CreateChannelServer(const std::string& pkgName, const std::string &sessName)
{
    AVTRANS_LOGI("Create session server for sessionName:%{public}s.", sessName.c_str());
    TRUE_RETURN_V_MSG_E(pkgName.empty(), ERR_DH_AVT_INVALID_PARAM, "input pkgName is empty.");
    TRUE_RETURN_V_MSG_E(sessName.empty(), ERR_DH_AVT_INVALID_PARAM, "input sessionName is empty.");

    {
        std::lock_guard<std::mutex> lock(serverMapMtx_);
        if (serverMap_.count(pkgName + "_" + sessName)) {
            AVTRANS_LOGI("Session has already created for name:%{public}s", sessName.c_str());
            return DH_AVT_SUCCESS;
        }
    }

    TransDataType dataType = TransDataType::DATA_TYPE_BYTES;
    if (sessName.find("avtrans.data") != std::string::npos) {
        dataType = TransDataType::DATA_TYPE_VIDEO_STREAM;
    }

    SocketInfo serverInfo = {
        .name = const_cast<char*>(sessName.c_str()),
        .pkgName = const_cast<char*>(pkgName.c_str()),
        .dataType = dataType,
    };
    int32_t socketId = Socket(serverInfo);
    if (socketId < 0) {
        AVTRANS_LOGE("Create Socket fail socketId:%{public}" PRId32, socketId);
        return ERR_DH_AVT_SESSION_ERROR;
    }
    QosTV qos[] = {
        {.qos = QOS_TYPE_MIN_BW,        .value = 40 * 1024 * 1024},
        {.qos = QOS_TYPE_MAX_LATENCY,       .value = 4000},
        {.qos = QOS_TYPE_MIN_LATENCY,       .value = 2000},
    };

    int32_t ret = Listen(socketId, qos, sizeof(qos) / sizeof(qos[0]), &sessListener_);
    if (ret != 0) {
        AVTRANS_LOGE("Listen socket error for sessionName:%{public}s", sessName.c_str());
        return ERR_DH_AVT_SESSION_ERROR;
    }
    {
        std::lock_guard<std::mutex> lock(serverMapMtx_);
        serverMap_.insert(std::make_pair(pkgName + "_" + sessName, socketId));
    }
    AVTRANS_LOGI("Create session server success for sessionName:%{public}s.", sessName.c_str());
    return DH_AVT_SUCCESS;
}

int32_t SoftbusChannelAdapter::RemoveChannelServer(const std::string& pkgName, const std::string &sessName)
{
    AVTRANS_LOGI("Remove session server for sessionName:%{public}s.", sessName.c_str());
    TRUE_RETURN_V_MSG_E(pkgName.empty(), ERR_DH_AVT_INVALID_PARAM, "input pkgName is empty.");
    TRUE_RETURN_V_MSG_E(sessName.empty(), ERR_DH_AVT_INVALID_PARAM, "input sessionName is empty.");
   
    std::string serverMapKey = pkgName + "_" + sessName;
    int32_t serverSocketId = INVALID_SESSION_ID;
    {
        std::lock_guard<std::mutex> lock(serverMapMtx_);
        for (auto it = serverMap_.begin(); it != serverMap_.end(); it++) {
            if (((it->first).find(serverMapKey) != std::string::npos)) {
                serverSocketId = it->second;
                serverMap_.erase(it->first);
                break;
            }
        }
    }
    AVTRANS_LOGI("Remove session server success for serverSocketId:%{public}" PRId32, serverSocketId);
    Shutdown(serverSocketId);
    int32_t sessionId = INVALID_SESSION_ID;
    {
        std::lock_guard<std::mutex> lock(idMapMutex_);
        for (auto it = devId2SessIdMap_.begin(); it != devId2SessIdMap_.end(); it++) {
            if ((it->first).find(sessName) != std::string::npos) {
                sessionId = it->second;
                devId2SessIdMap_.erase(it->first);
                break;
            }
        }
    }
    Shutdown(sessionId);
    AVTRANS_LOGI("Remove session server success for sessionName:%{public}s.", sessName.c_str());
    return DH_AVT_SUCCESS;
}

void OHOS::DistributedHardware::SoftbusChannelAdapter::SendEventChannelOPened(const std::string & mySessName,
    const std::string & peerDevId)
{
    EventType type = EventType::EVENT_CHANNEL_OPENED;
    AVTransEvent event = {type, mySessName, peerDevId};
    std::lock_guard<std::mutex> lock(listenerMtx_);
    {
        for (auto it = listenerMap_.begin(); it != listenerMap_.end(); it++) {
            if (((it->first).find(mySessName) != std::string::npos) && (it->second != nullptr)) {
                std::thread(&SoftbusChannelAdapter::SendChannelEvent, this, it->first, event).detach();
            }
        }
    }
}

int32_t SoftbusChannelAdapter::OpenSoftbusChannel(const std::string &mySessName, const std::string &peerSessName,
    const std::string &peerDevId)
{
    AVTRANS_LOGI("Open softbus channel for mySessName:%{public}s, peerSessName:%{public}s, peerDevId:%{public}s.",
        mySessName.c_str(), peerSessName.c_str(), GetAnonyString(peerDevId).c_str());
    TRUE_RETURN_V_MSG_E(mySessName.empty(), ERR_DH_AVT_INVALID_PARAM, "input mySessName is empty.");
    TRUE_RETURN_V_MSG_E(peerSessName.empty(), ERR_DH_AVT_INVALID_PARAM, "input peerSessName is empty.");
    TRUE_RETURN_V_MSG_E(peerDevId.empty(), ERR_DH_AVT_INVALID_PARAM, "input peerDevId is empty.");
    std::string ownerName = GetOwnerFromSessName(mySessName);
    std::string PkgName = TransName2PkgName(ownerName);
    int32_t existSessId = GetSessIdBySessName(mySessName, peerDevId);
    if (existSessId > 0) {
        AVTRANS_LOGI("Softbus channel already opened, sessionId:%{public}" PRId32, existSessId);
        return ERR_DH_AVT_SESSION_HAS_OPENED;
    }

    QosTV qos[] = {
        {.qos = QOS_TYPE_MIN_BW,        .value = 40 * 1024 * 1024},
        {.qos = QOS_TYPE_MAX_LATENCY,       .value = 4000},
        {.qos = QOS_TYPE_MIN_LATENCY,       .value = 2000},
    };
    
    TransDataType dataType = TransDataType::DATA_TYPE_BYTES;
    if (mySessName.find("avtrans.data") != std::string::npos) {
        dataType = TransDataType::DATA_TYPE_VIDEO_STREAM;
    }

    SocketInfo clientInfo = {
        .name = const_cast<char*>((mySessName.c_str())),
        .peerName = const_cast<char*>(peerSessName.c_str()),
        .peerNetworkId = const_cast<char*>(peerDevId.c_str()),
        .pkgName = const_cast<char*>(PkgName.c_str()),
        .dataType = dataType,
    };

    int32_t socketId = Socket(clientInfo);
    if (socketId <0) {
        AVTRANS_LOGE("Create OpenSoftbusChannel Socket error");
        return ERR_DH_AVT_SESSION_ERROR;
    }
    int32_t ret = Bind(socketId, qos, sizeof(qos) / sizeof(qos[0]), &sessListener_);
    if (ret != DH_AVT_SUCCESS) {
        AVTRANS_LOGE("Bind SocketClient error");
        return ERR_DH_AVT_SESSION_ERROR;
    }
    {
        std::lock_guard<std::mutex> lock(idMapMutex_);
        devId2SessIdMap_.insert(std::make_pair(mySessName + "_" + peerDevId, socketId));
    }
    SendEventChannelOPened(mySessName, peerDevId);
    AVTRANS_LOGI("Open softbus channel finished for mySessName:%{public}s.", mySessName.c_str());
    return DH_AVT_SUCCESS;
}

int32_t SoftbusChannelAdapter::CloseSoftbusChannel(const std::string& sessName, const std::string &peerDevId)
{
    AVTRANS_LOGI("Close softbus channel for sessName:%{public}s, peerDevId:%{public}s.", sessName.c_str(),
        GetAnonyString(peerDevId).c_str());
    TRUE_RETURN_V_MSG_E(sessName.empty(), ERR_DH_AVT_INVALID_PARAM, "input sessName is empty.");
    TRUE_RETURN_V_MSG_E(peerDevId.empty(), ERR_DH_AVT_INVALID_PARAM, "input peerDevId is empty.");

    int32_t sessionId = GetSessIdBySessName(sessName, peerDevId);
    Shutdown(sessionId);
    {
        std::lock_guard<std::mutex> lock(idMapMutex_);
        devId2SessIdMap_.erase(sessName + "_" + peerDevId);
    }

    AVTRANS_LOGI("Close softbus channel success, sessionId:%{public}" PRId32, sessionId);
    return DH_AVT_SUCCESS;
}

int32_t SoftbusChannelAdapter::SendBytesData(const std::string& sessName, const std::string &peerDevId,
    const std::string &data)
{
    AVTRANS_LOGI("Send bytes data for sessName:%{public}s, peerDevId:%{public}s.",
        sessName.c_str(), GetAnonyString(peerDevId).c_str());
    TRUE_RETURN_V_MSG_E(sessName.empty(), ERR_DH_AVT_INVALID_PARAM, "input sessName is empty.");
    TRUE_RETURN_V_MSG_E(peerDevId.empty(), ERR_DH_AVT_INVALID_PARAM, "input peerDevId is empty.");
    TRUE_RETURN_V_MSG_E(data.empty(), ERR_DH_AVT_INVALID_PARAM, "input data string is empty.");

    int32_t existSessId = GetSessIdBySessName(sessName, peerDevId);
    if (existSessId < 0) {
        AVTRANS_LOGI("Can not find sessionId for mySessName:%{public}s, peerDevId:%{public}s.",
            sessName.c_str(), GetAnonyString(peerDevId).c_str());
        return ERR_DH_AVT_SEND_DATA_FAILED;
    }
    int32_t ret = SendBytes(existSessId, data.c_str(), strlen(data.c_str()));
    if (ret != DH_AVT_SUCCESS) {
        AVTRANS_LOGE("Send bytes data failed ret:%{public}" PRId32, ret);
        return ERR_DH_AVT_SEND_DATA_FAILED;
    }
    return DH_AVT_SUCCESS;
}

int32_t SoftbusChannelAdapter::SendStreamData(const std::string& sessName, const std::string &peerDevId,
    const StreamData *data, const StreamData *ext)
{
    TRUE_RETURN_V_MSG_E(sessName.empty(), ERR_DH_AVT_INVALID_PARAM, "input sessName is empty.");
    TRUE_RETURN_V_MSG_E(peerDevId.empty(), ERR_DH_AVT_INVALID_PARAM, "input peerDevId is empty.");
    TRUE_RETURN_V_MSG_E(data == nullptr, ERR_DH_AVT_INVALID_PARAM, "input data is nullptr.");
    TRUE_RETURN_V_MSG_E(ext == nullptr, ERR_DH_AVT_INVALID_PARAM, "input ext data is nullptr.");

    StreamFrameInfo frameInfo = {0};
    int32_t existSessId = GetSessIdBySessName(sessName, peerDevId);
    if (existSessId < 0) {
        AVTRANS_LOGI("Can not find sessionId for mySessName:%{public}s, peerDevId:%{public}s.",
            sessName.c_str(), GetAnonyString(peerDevId).c_str());
        return ERR_DH_AVT_SEND_DATA_FAILED;
    }
    int32_t ret = SendStream(existSessId, data, ext, &frameInfo);
    if (ret != DH_AVT_SUCCESS) {
        AVTRANS_LOGE("Send stream data failed ret:%{public}" PRId32, ret);
        return ERR_DH_AVT_SEND_DATA_FAILED;
    }
    return DH_AVT_SUCCESS;
}

int32_t SoftbusChannelAdapter::RegisterChannelListener(const std::string& sessName, const std::string &peerDevId,
    ISoftbusChannelListener *listener)
{
    AVTRANS_LOGI("Register channel listener for sessName:%{public}s, peerDevId:%{public}s.",
        sessName.c_str(), GetAnonyString(peerDevId).c_str());
    TRUE_RETURN_V_MSG_E(sessName.empty(), ERR_DH_AVT_INVALID_PARAM, "input sessName is empty.");
    TRUE_RETURN_V_MSG_E(peerDevId.empty(), ERR_DH_AVT_INVALID_PARAM, "input peerDevId is empty.");
    TRUE_RETURN_V_MSG_E(listener == nullptr, ERR_DH_AVT_INVALID_PARAM, "input callback is nullptr.");

    std::lock_guard<std::mutex> lock(listenerMtx_);
    listenerMap_[sessName + "_" + peerDevId] = listener;

    return DH_AVT_SUCCESS;
}

int32_t SoftbusChannelAdapter::UnRegisterChannelListener(const std::string& sessName, const std::string &peerDevId)
{
    AVTRANS_LOGI("Unregister channel listener for sessName:%{public}s, peerDevId:%{public}s.",
        sessName.c_str(), GetAnonyString(peerDevId).c_str());
    TRUE_RETURN_V_MSG_E(sessName.empty(), ERR_DH_AVT_INVALID_PARAM, "input sessName is empty.");
    TRUE_RETURN_V_MSG_E(peerDevId.empty(), ERR_DH_AVT_INVALID_PARAM, "input peerDevId is empty.");

    std::lock_guard<std::mutex> lock(listenerMtx_);
    listenerMap_.erase(sessName + "_" + peerDevId);

    return DH_AVT_SUCCESS;
}

int32_t SoftbusChannelAdapter::StartDeviceTimeSync(const std::string &pkgName, const std::string& sessName,
    const std::string &peerDevId)
{
    AVTRANS_LOGI("Start device time sync for peerDeviceId:%{public}s.", GetAnonyString(peerDevId).c_str());
    TRUE_RETURN_V_MSG_E(peerDevId.empty(), ERR_DH_AVT_INVALID_PARAM, "input peerDevId is empty.");

    ITimeSyncCb timeSyncCbk = {.onTimeSyncResult = onDevTimeSyncResult};
    int32_t ret = StartTimeSync(pkgName.c_str(), peerDevId.c_str(), TimeSyncAccuracy::SUPER_HIGH_ACCURACY,
        TimeSyncPeriod::SHORT_PERIOD, &timeSyncCbk);
    if (ret != 0) {
        AVTRANS_LOGE("StartTimeSync failed ret:%{public}" PRId32, ret);
        return ERR_DH_AVT_TIME_SYNC_FAILED;
    }

    std::lock_guard<std::mutex> lock(timeSyncMtx_);
    timeSyncSessNames_.insert(sessName + "_" + peerDevId);

    return DH_AVT_SUCCESS;
}

int32_t SoftbusChannelAdapter::StopDeviceTimeSync(const std::string &pkgName, const std::string& sessName,
    const std::string &peerDevId)
{
    AVTRANS_LOGI("Stop device time sync for peerDeviceId:%{public}s.", GetAnonyString(peerDevId).c_str());
    TRUE_RETURN_V_MSG_E(peerDevId.empty(), ERR_DH_AVT_INVALID_PARAM, "input peerDevId is empty.");

    int32_t ret = StopTimeSync(pkgName.c_str(), peerDevId.c_str());
    if (ret != 0) {
        AVTRANS_LOGE("StopTimeSync failed ret:%{public}" PRId32, ret);
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
        AVTRANS_LOGI("Can not find sessionId for sessName:%{public}s, peerDevId:%{public}s.",
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

    AVTRANS_LOGE("No available channel or invalid sessionId:%{public}" PRId32, sessionId);
    return EMPTY_STRING;
}

int32_t SoftbusChannelAdapter::OnSoftbusChannelOpened(std::string peerSessionName, int32_t sessionId,
    std::string peerDevId, int32_t result)
{
    AVTRANS_LOGI("On softbus channel opened, sessionId: %{public}" PRId32", result: %{public}" PRId32
        " peerSessionName: %{public}s", sessionId, result, peerSessionName.c_str());
    TRUE_RETURN_V_MSG_E(peerSessionName.empty(), ERR_DH_AVT_INVALID_PARAM, "peerSessionName is empty().");
    TRUE_RETURN_V_MSG_E(peerDevId.empty(), ERR_DH_AVT_INVALID_PARAM, "peerDevId is empty().");

    std::lock_guard<std::mutex> lock(idMapMutex_);
    std::string mySessionName = FindSessNameByPeerSessName(peerSessionName);
    TRUE_RETURN_V_MSG_E(mySessionName.empty(), ERR_DH_AVT_INVALID_PARAM, "mySessionName is empty().");
    EventType type = (result == 0) ? EventType::EVENT_CHANNEL_OPENED : EventType::EVENT_CHANNEL_OPEN_FAIL;
    AVTransEvent event = {type, mySessionName, peerDevId};
    {
        std::lock_guard<std::mutex> lock(listenerMtx_);
        for (auto it = listenerMap_.begin(); it != listenerMap_.end(); it++) {
            if (((it->first).find(mySessionName) != std::string::npos) && (it->second != nullptr)) {
                std::thread(&SoftbusChannelAdapter::SendChannelEvent, this, it->first, event).detach();
                devId2SessIdMap_.erase(it->first);
                devId2SessIdMap_.insert(std::make_pair(it->first, sessionId));
            }
        }
    }
    std::string idMapKey = mySessionName + "_" + peerDevId;
    if (devId2SessIdMap_.find(idMapKey) == devId2SessIdMap_.end()) {
        AVTRANS_LOGI("Can not find sessionId for mySessionName:%{public}s, peerDevId:%{public}s. try to insert it.",
            mySessionName.c_str(), GetAnonyString(peerDevId).c_str());
            devId2SessIdMap_.insert(std::make_pair(idMapKey, sessionId));
    }
    return DH_AVT_SUCCESS;
}

void SoftbusChannelAdapter::OnSoftbusChannelClosed(int32_t sessionId, ShutdownReason reason)
{
    (void)reason;
    AVTRANS_LOGI("On softbus channel closed, sessionId:%{public}" PRId32, sessionId);

    std::string peerDevId = GetPeerDevIdBySessId(sessionId);
    AVTransEvent event = {EventType::EVENT_CHANNEL_CLOSED, "", peerDevId};

    std::lock_guard<std::mutex> lock(idMapMutex_);
    for (auto it = devId2SessIdMap_.begin(); it != devId2SessIdMap_.end();) {
        if (it->second == sessionId) {
            event.content = GetOwnerFromSessName(it->first);
            std::thread(&SoftbusChannelAdapter::SendChannelEvent, this, it->first, event).detach();
            it = devId2SessIdMap_.erase(it);
        } else {
            it++;
        }
    }
}

void SoftbusChannelAdapter::OnSoftbusBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    AVTRANS_LOGI("On softbus channel bytes received, sessionId:%{public}" PRId32, sessionId);
    TRUE_RETURN(data == nullptr, "input data is nullptr.");
    TRUE_RETURN(dataLen == 0, "input dataLen is 0.");
    TRUE_RETURN(dataLen > DSOFTBUS_INPUT_MAX_RECV_DATA_LEN, "input dataLen is over size.");

    std::string peerDevId = GetPeerDevIdBySessId(sessionId);
    AVTRANS_LOGI("OnSoftbusBytesReceived peerDevId:%{public}s.", GetAnonyString(peerDevId).c_str());

    std::string dataStr = std::string(reinterpret_cast<const char *>(data), dataLen);
    AVTransEvent event = {EventType::EVENT_DATA_RECEIVED, dataStr, peerDevId};

    std::lock_guard<std::mutex> lock(idMapMutex_);
    for (auto it = devId2SessIdMap_.begin(); it != devId2SessIdMap_.end(); it++) {
        if (it->second == sessionId) {
            std::thread(&SoftbusChannelAdapter::SendChannelEvent, this, it->first, event).detach();
        }
    }
}

void SoftbusChannelAdapter::OnSoftbusStreamReceived(int32_t sessionId, const StreamData *data,
    const StreamData *ext, const StreamFrameInfo *frameInfo)
{
    (void)frameInfo;
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
    AVTRANS_LOGI("On softbus channel time sync result:%{public}" PRId32, result);
    TRUE_RETURN(result == 0, "On softbus channel time sync failed");

    if (info == nullptr) {
        AVTRANS_LOGE("info id nullptr");
        return;
    }
    int32_t millisecond = info->result.millisecond;
    int32_t microsecond = info->result.microsecond;
    TimeSyncAccuracy accuracy  = info->result.accuracy;
    AVTRANS_LOGI("Time sync success, flag:%{public}" PRId32", millisecond:%{public}" PRId32 ", microsecond:%{public}"
        PRId32 ", accuracy:%{public}" PRId32, info->flag, millisecond, microsecond, accuracy);

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

std::string SoftbusChannelAdapter::GetPeerDevIdBySessId(int32_t sessionId)
{
    std::lock_guard<std::mutex> lock(idMapMutex_);
    for (auto it = devId2SessIdMap_.begin(); it != devId2SessIdMap_.end(); it++) {
        if (it->second != sessionId) {
            continue;
        }
        std::string::size_type position = (it->first).find_last_of("_");
        if (position == std::string::npos) {
            continue;
        }
        std::string peerDevId = (it->first).substr(position + 1);
        if (peerDevId != AV_TRANS_SPECIAL_DEVICE_ID) {
            return peerDevId;
        }
    }
    return EMPTY_STRING;
}

std::string SoftbusChannelAdapter::GetOwnerFromSessName(const std::string &sessName)
{
    std::string::size_type position = sessName.find_first_of("_");
    if (position != std::string::npos) {
        return sessName.substr(0, position);
    }
    if (sessName == AV_SYNC_SENDER_CONTROL_SESSION_NAME || sessName == AV_SYNC_RECEIVER_CONTROL_SESSION_NAME) {
        return sessName;
    }
    return EMPTY_STRING;
}

void SoftbusChannelAdapter::SendChannelEvent(const std::string &sessName, const AVTransEvent event)
{
    AVTRANS_LOGI("SendChannelEvent event.type_%{public}" PRId32, event.type);
    pthread_setname_np(pthread_self(), SEND_CHANNEL_EVENT);

    ISoftbusChannelListener *listener = nullptr;
    {
        std::lock_guard<std::mutex> lock(listenerMtx_);
        listener = listenerMap_[sessName];
        TRUE_RETURN(listener == nullptr, "input listener is nullptr.");
    }
    listener->OnChannelEvent(event);
}
} // namespace DistributedHardware
} // namespace OHOS