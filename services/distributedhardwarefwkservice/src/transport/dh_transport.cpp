/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "dh_transport.h"

#include <cinttypes>

#include "cJSON.h"
#include <securec.h>

#include "anonymous_string.h"
#include "constants.h"
#include "dh_comm_tool.h"
#include "dh_context.h"
#include "dh_transport_obj.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "DHTransport"
namespace {
// Dsoftbus sendBytes max message length: 4MB
const uint32_t MAX_SEND_MSG_LENGTH = 4 * 1024 * 1024;
const uint32_t INTERCEPT_STRING_LENGTH = 20;
static QosTV g_qosInfo[] = {
    { .qos = QOS_TYPE_MIN_BW, .value = 80 * 1024 * 1024},
    { .qos = QOS_TYPE_MAX_LATENCY, .value = 8000 },
    { .qos = QOS_TYPE_MIN_LATENCY, .value = 2000 }
};
static uint32_t g_QosTV_Param_Index = static_cast<uint32_t>(sizeof(g_qosInfo) / sizeof(g_qosInfo[0]));
}

DHTransport::DHTransport(std::shared_ptr<DHCommTool> dhCommToolPtr) : remoteDevSocketIds_({}), localServerSocket_(-1),
    localSocketName_(""), isSocketSvrCreateFlag_(false), dhCommToolWPtr_(dhCommToolPtr)
{
    DHLOGI("Ctor DHTransport");
}

int32_t DHTransport::OnSocketOpened(int32_t socketId, const PeerSocketInfo &info)
{
    DHLOGI("OnSocketOpened, socket: %{public}d, peerSocketName: %{public}s, peerNetworkId: %{public}s, "
        "peerPkgName: %{public}s", socketId, info.name, GetAnonyString(info.networkId).c_str(), info.pkgName);
    std::lock_guard<std::mutex> lock(rmtSocketIdMtx_);
    remoteDevSocketIds_[info.networkId] = socketId;
    return DH_FWK_SUCCESS;
}

void DHTransport::OnSocketClosed(int32_t socketId, ShutdownReason reason)
{
    DHLOGI("OnSocketClosed, socket: %{public}d, reason: %{public}d", socketId, (int32_t)reason);
    std::lock_guard<std::mutex> lock(rmtSocketIdMtx_);
    for (auto iter = remoteDevSocketIds_.begin(); iter != remoteDevSocketIds_.end(); ++iter) {
        if (iter->second == socketId) {
            remoteDevSocketIds_.erase(iter);
            break;
        }
    }
}

void DHTransport::OnBytesReceived(int32_t socketId, const void *data, uint32_t dataLen)
{
    if (socketId < 0 || data == nullptr || dataLen == 0 || dataLen > MAX_SEND_MSG_LENGTH) {
        DHLOGE("OnBytesReceived param check failed");
        return;
    }

    std::string remoteNeworkId = GetRemoteNetworkIdBySocketId(socketId);
    if (remoteNeworkId.empty()) {
        DHLOGE("Can not find the remote network id by socketId: %{public}d", socketId);
        return;
    }

    uint8_t *buf = reinterpret_cast<uint8_t *>(calloc(dataLen + 1, sizeof(uint8_t)));
    if (buf == nullptr) {
        DHLOGE("OnBytesReceived: malloc memory failed");
        return;
    }

    if (memcpy_s(buf, dataLen + 1,  reinterpret_cast<const uint8_t *>(data), dataLen) != EOK) {
        DHLOGE("OnBytesReceived: memcpy memory failed");
        free(buf);
        return;
    }

    std::string message(buf, buf + dataLen);
    DHLOGI("Receive message size: %{public}" PRIu32, dataLen);
    HandleReceiveMessage(message);
    free(buf);
    return;
}

void DHTransport::HandleReceiveMessage(const std::string &payload)
{
    std::string rawPayload = Decompress(payload);

    cJSON *root = cJSON_Parse(rawPayload.c_str());
    if (root == NULL) {
        DHLOGE("the msg is not json format");
        return;
    }
    std::shared_ptr<CommMsg> commMsg = std::make_shared<CommMsg>();
    FromJson(root, *commMsg);
    cJSON_Delete(root);

    DHLOGI("Receive DH msg, code: %{public}d, msg: %{public}s", commMsg->code, GetAnonyString(commMsg->msg).c_str());
    AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(commMsg->code, commMsg);
    std::shared_ptr<DHCommTool> dhCommToolSPtr = dhCommToolWPtr_.lock();
    if (dhCommToolSPtr == nullptr) {
        DHLOGE("Can not get DHCommTool ptr");
        return;
    }
    dhCommToolSPtr->GetEventHandler()->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void OnBind(int32_t socket, PeerSocketInfo info)
{
    if (DHCommTool::GetInstance()->GetDHTransportPtr() == nullptr) {
        DHLOGE("get DHTransport ptr return null");
        return;
    }
    DHCommTool::GetInstance()->GetDHTransportPtr()->OnSocketOpened(socket, info);
}

void OnShutdown(int32_t socket, ShutdownReason reason)
{
    if (DHCommTool::GetInstance()->GetDHTransportPtr() == nullptr) {
        DHLOGE("get DHTransport ptr return null");
        return;
    }
    DHCommTool::GetInstance()->GetDHTransportPtr()->OnSocketClosed(socket, reason);
}

void OnBytes(int32_t socket, const void *data, uint32_t dataLen)
{
    if (DHCommTool::GetInstance()->GetDHTransportPtr() == nullptr) {
        DHLOGE("get DHTransport ptr return null");
        return;
    }
    DHCommTool::GetInstance()->GetDHTransportPtr()->OnBytesReceived(socket, data, dataLen);
}

void OnMessage(int32_t socket, const void *data, uint32_t dataLen)
{
    (void)socket;
    (void)data;
    (void)dataLen;
    DHLOGI("socket: %{public}d, dataLen:%{public}" PRIu32, socket, dataLen);
}

void OnStream(int32_t socket, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *param)
{
    (void)socket;
    (void)data;
    (void)ext;
    (void)param;
    DHLOGI("socket: %{public}d", socket);
}

void OnFile(int32_t socket, FileEvent *event)
{
    (void)event;
    DHLOGI("socket: %{public}d", socket);
}

void OnQos(int32_t socket, QoSEvent eventId, const QosTV *qos, uint32_t qosCount)
{
    DHLOGI("OnQos, socket: %{public}d, QoSEvent: %{public}d, qosCount: %{public}" PRIu32,
        socket, (int32_t)eventId, qosCount);
    for (uint32_t idx = 0; idx < qosCount; idx++) {
        DHLOGI("QosTV: type: %{public}d, value: %{public}d", (int32_t)qos[idx].qos, qos[idx].value);
    }
}

ISocketListener iSocketListener = {
    .OnBind = OnBind,
    .OnShutdown = OnShutdown,
    .OnBytes = OnBytes,
    .OnMessage = OnMessage,
    .OnStream = OnStream,
    .OnFile = OnFile,
    .OnQos = OnQos
};

int32_t DHTransport::CreateServerSocket()
{
    DHLOGI("CreateServerSocket start");
    std::string networkId = GetLocalNetworkId();
    localSocketName_ = DH_FWK_SESSION_NAME + networkId.substr(0, INTERCEPT_STRING_LENGTH);
    DHLOGI("CreateServerSocket local networkId is %{public}s, local socketName: %{public}s",
        GetAnonyString(networkId).c_str(), localSocketName_.c_str());
    SocketInfo info = {
        .name = const_cast<char*>(localSocketName_.c_str()),
        .pkgName = const_cast<char*>(DH_FWK_PKG_NAME.c_str()),
        .dataType = DATA_TYPE_BYTES
    };
    int32_t socket = Socket(info);
    DHLOGI("CreateServerSocket Finish, socket: %{public}d", socket);
    return socket;
}

int32_t DHTransport::CreateClientSocket(const std::string &remoteNetworkId)
{
    DHLOGI("CreateClientSocket start, peerNetworkId: %{public}s", GetAnonyString(remoteNetworkId).c_str());
    std::string peerSocketName = DH_FWK_SESSION_NAME + remoteNetworkId.substr(0, INTERCEPT_STRING_LENGTH);
    SocketInfo info = {
        .name = const_cast<char*>(localSocketName_.c_str()),
        .peerName = const_cast<char*>(peerSocketName.c_str()),
        .peerNetworkId = const_cast<char*>(remoteNetworkId.c_str()),
        .pkgName = const_cast<char*>(DH_FWK_PKG_NAME.c_str()),
        .dataType = DATA_TYPE_BYTES
    };
    int32_t socket = Socket(info);
    DHLOGI("Bind Socket server, socket: %{public}d, localSocketName: %{public}s, peerSocketName: %{public}s",
        socket, localSocketName_.c_str(), peerSocketName.c_str());
    return socket;
}

int32_t DHTransport::Init()
{
    DHLOGI("Init DHTransport");
    if (isSocketSvrCreateFlag_.load()) {
        DHLOGI("SocketServer already create success.");
        return DH_FWK_SUCCESS;
    }
    int32_t socket = CreateServerSocket();
    if (socket < DH_FWK_SUCCESS) {
        DHLOGE("CreateSocketServer failed, ret: %{public}d", socket);
        return ERR_DH_FWK_COMPONENT_TRANSPORT_OPT_FAILED;
    }

    int32_t ret = Listen(socket, g_qosInfo, g_QosTV_Param_Index, &iSocketListener);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Socket Listen failed, error code %{public}d.", ret);
        return ERR_DH_FWK_COMPONENT_TRANSPORT_OPT_FAILED;
    }
    isSocketSvrCreateFlag_.store(true);
    localServerSocket_ = socket;
    DHLOGI("Finish Init DSoftBus Server Socket, socket: %{public}d", socket);
    return DH_FWK_SUCCESS;
}

int32_t DHTransport::UnInit()
{
    {
        std::lock_guard<std::mutex> lock(rmtSocketIdMtx_);
        for (auto iter = remoteDevSocketIds_.begin(); iter != remoteDevSocketIds_.end(); ++iter) {
            DHLOGI("Shutdown client socket: %{public}d to remote dev: %{public}s", iter->second,
                GetAnonyString(iter->first).c_str());
            Shutdown(iter->second);
        }
        remoteDevSocketIds_.clear();
    }

    if (!isSocketSvrCreateFlag_.load()) {
        DHLOGI("DSoftBus Server Socket already remove success.");
    } else {
        DHLOGI("Shutdown DSoftBus Server Socket, socket: %{public}d", localServerSocket_.load());
        Shutdown(localServerSocket_.load());
        localServerSocket_ = -1;
        isSocketSvrCreateFlag_.store(false);
    }
    return DH_FWK_SUCCESS;
}

bool DHTransport::IsDeviceSessionOpened(const std::string &remoteNetworkId, int32_t &socketId)
{
    std::lock_guard<std::mutex> lock(rmtSocketIdMtx_);
    if (remoteDevSocketIds_.find(remoteNetworkId) == remoteDevSocketIds_.end()) {
        return false;
    }
    socketId = remoteDevSocketIds_.at(remoteNetworkId);
    DHLOGI("DeviceSession has opened, remoteNetworkId: %{public}s, socketId: %{public}d",
        GetAnonyString(remoteNetworkId).c_str(), socketId);
    return true;
}

std::string DHTransport::GetRemoteNetworkIdBySocketId(int32_t socketId)
{
    std::lock_guard<std::mutex> lock(rmtSocketIdMtx_);
    std::string networkId = "";
    for (auto const &item : remoteDevSocketIds_) {
        if (item.second == socketId) {
            networkId = item.first;
            break;
        }
    }
    return networkId;
}

void DHTransport::ClearDeviceSocketOpened(const std::string &remoteDevId)
{
    std::lock_guard<std::mutex> lock(rmtSocketIdMtx_);
    remoteDevSocketIds_.erase(remoteDevId);
}

int32_t DHTransport::StartSocket(const std::string &remoteNetworkId)
{
    int32_t socketId = -1;
    if (IsDeviceSessionOpened(remoteNetworkId, socketId)) {
        DHLOGE("Softbus session has already opened, deviceId: %{public}s", GetAnonyString(remoteNetworkId).c_str());
        return DH_FWK_SUCCESS;
    }

    int32_t socket = CreateClientSocket(remoteNetworkId);
    if (socket < DH_FWK_SUCCESS) {
        DHLOGE("StartSocket failed, ret: %{public}d", socket);
        return ERR_DH_FWK_COMPONENT_TRANSPORT_OPT_FAILED;
    }

    int32_t ret = Bind(socket, g_qosInfo, g_QosTV_Param_Index, &iSocketListener);
    if (ret < DH_FWK_SUCCESS) {
        DHLOGE("OpenSession fail, remoteNetworkId: %{public}s, socket: %{public}d, ret: %{public}d",
            GetAnonyString(remoteNetworkId).c_str(), socket, ret);
        Shutdown(socket);
        return ERR_DH_FWK_COMPONENT_TRANSPORT_OPT_FAILED;
    }

    DHLOGI("Bind Socket success, remoteNetworkId:%{public}s, socketId: %{public}d",
        GetAnonyString(remoteNetworkId).c_str(), socket);
    std::string peerSocketName = DH_FWK_SESSION_NAME + remoteNetworkId.substr(0, INTERCEPT_STRING_LENGTH);
    PeerSocketInfo peerSocketInfo = {
        .name = const_cast<char*>(peerSocketName.c_str()),
        .networkId = const_cast<char*>(remoteNetworkId.c_str()),
        .pkgName = const_cast<char*>(DH_FWK_PKG_NAME.c_str()),
        .dataType = DATA_TYPE_BYTES
    };
    OnSocketOpened(socket, peerSocketInfo);
    return DH_FWK_SUCCESS;
}

int32_t DHTransport::StopSocket(const std::string &remoteNetworkId)
{
    int32_t socketId = -1;
    if (!IsDeviceSessionOpened(remoteNetworkId, socketId)) {
        DHLOGI("remote dev may be not opened, remoteNetworkId: %{public}s", GetAnonyString(remoteNetworkId).c_str());
        return ERR_DH_FWK_COMPONENT_TRANSPORT_OPT_FAILED;
    }

    DHLOGI("StopSocket remoteNetworkId: %{public}s, socketId: %{public}d",
        GetAnonyString(remoteNetworkId).c_str(), socketId);
    Shutdown(socketId);
    ClearDeviceSocketOpened(remoteNetworkId);
    return DH_FWK_SUCCESS;
}

int32_t DHTransport::Send(const std::string &remoteNetworkId, const std::string &payload)
{
    int32_t socketId = -1;
    if (!IsDeviceSessionOpened(remoteNetworkId, socketId)) {
        DHLOGI("The session is not open, target networkId: %{public}s", GetAnonyString(remoteNetworkId).c_str());
        return ERR_DH_FWK_COMPONENT_TRANSPORT_OPT_FAILED;
    }
    std::string compressedPayLoad = Compress(payload);
    uint32_t compressedPayLoadSize = compressedPayLoad.size();
    DHLOGI("Send payload size: %{puablic}" PRIu32 ", after compressed size: %{public}" PRIu32
        ", target networkId: %{public}s, socketId: %{public}d", payload.size(), compressedPayLoadSize,
        GetAnonyString(remoteNetworkId).c_str(), socketId);

    if (compressedPayLoadSize > MAX_SEND_MSG_LENGTH) {
        DHLOGE("Send error: msg size: %{public}" PRIu32 " too long", compressedPayLoadSize);
        return ERR_DH_FWK_COMPONENT_TRANSPORT_OPT_FAILED;
    }
    uint8_t *buf = reinterpret_cast<uint8_t *>(calloc((compressedPayLoadSize), sizeof(uint8_t)));
    if (buf == nullptr) {
        DHLOGE("Send: malloc memory failed");
        return ERR_DH_FWK_COMPONENT_TRANSPORT_OPT_FAILED;
    }

    if (memcpy_s(buf, compressedPayLoadSize, reinterpret_cast<const uint8_t *>(compressedPayLoad.c_str()),
                 compressedPayLoadSize) != EOK) {
        DHLOGE("Send: memcpy memory failed");
        free(buf);
        return ERR_DH_FWK_COMPONENT_TRANSPORT_OPT_FAILED;
    }

    int32_t ret = SendBytes(socketId, buf, compressedPayLoadSize);
    free(buf);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("dsoftbus send error, ret: %{public}d", ret);
        return ERR_DH_FWK_COMPONENT_TRANSPORT_OPT_FAILED;
    }
    DHLOGI("Send payload success");
    return DH_FWK_SUCCESS;
}
} // DistributedHardware
} // OHOS