/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "distributed_hardware_proxy.h"

#include <cinttypes>
#include <unordered_set>

#include "anonymous_string.h"
#include "av_trans_errno.h"
#include "constants.h"
#include "dhardware_ipc_interface_code.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "nlohmann/json.hpp"
#include "parcel.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "DistributedHardwareProxy"
const std::unordered_set<DHType> DH_TYPE_SET {
    DHType::UNKNOWN, DHType::CAMERA, DHType::AUDIO, DHType::SCREEN, DHType::GPS, DHType::INPUT,
    DHType::HFP, DHType::A2D, DHType::VIRMODEM_AUDIO, DHType::MAX_DH,
   };

int32_t DistributedHardwareProxy::RegisterPublisherListener(const DHTopic topic,
    const sptr<IPublisherListener> &listener)
{
    if (listener == nullptr) {
        DHLOGE("publisher listener is null");
        return ERR_DH_FWK_PUBLISHER_LISTENER_IS_NULL;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return ERR_DH_FWK_SERVICE_REMOTE_IS_NULL;
    }
    if (topic < DHTopic::TOPIC_MIN || topic > DHTopic::TOPIC_MAX) {
        DHLOGE("Topic is invalid!");
        return ERR_DH_FWK_PARA_INVALID;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_FWK_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteUint32((uint32_t)topic)) {
        DHLOGE("DistributedHardwareProxy write topic failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        DHLOGE("DistributedHardwareProxy write listener failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::REG_PUBLISHER_LISTNER),
        data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_FWK_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    ret = reply.ReadInt32();
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Register Publisher Listener failed, ret: %d", ret);
    }

    return ret;
}

int32_t DistributedHardwareProxy::UnregisterPublisherListener(const DHTopic topic,
    const sptr<IPublisherListener> &listener)
{
    if (listener == nullptr) {
        DHLOGE("publisher listener is null");
        return ERR_DH_FWK_PUBLISHER_LISTENER_IS_NULL;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return ERR_DH_FWK_SERVICE_REMOTE_IS_NULL;
    }
    if (topic < DHTopic::TOPIC_MIN || topic > DHTopic::TOPIC_MAX) {
        DHLOGE("Topic is invalid!");
        return ERR_DH_FWK_PARA_INVALID;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_FWK_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteUint32((uint32_t)topic)) {
        DHLOGE("DistributedHardwareProxy write topic failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        DHLOGE("DistributedHardwareProxy write listener failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::UNREG_PUBLISHER_LISTENER),
        data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_FWK_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    ret = reply.ReadInt32();
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Unregister Publisher Listener failed, ret: %d", ret);
    }

    return ret;
}

int32_t DistributedHardwareProxy::PublishMessage(const DHTopic topic, const std::string &msg)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return ERR_DH_FWK_SERVICE_REMOTE_IS_NULL;
    }
    if (topic < DHTopic::TOPIC_MIN || topic > DHTopic::TOPIC_MAX) {
        DHLOGE("Topic is invalid!");
        return ERR_DH_FWK_PARA_INVALID;
    }
    if (msg.empty() || msg.size() > MAX_MESSAGE_LEN) {
        DHLOGE("Msg is invalid");
        return ERR_DH_FWK_SERVICE_MSG_INVALID;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_FWK_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteUint32((uint32_t)topic)) {
        DHLOGE("DistributedHardwareProxy write topic failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteString(msg)) {
        DHLOGE("DistributedHardwareProxy write listener failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::PUBLISH_MESSAGE),
        data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_FWK_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    ret = reply.ReadInt32();
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("PublishMessage failed, ret: %d", ret);
    }

    return ret;
}

std::string DistributedHardwareProxy::QueryLocalSysSpec(QueryLocalSysSpecType spec)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return "";
    }
    if (spec < QueryLocalSysSpecType::MIN || spec > QueryLocalSysSpecType::MAX) {
        DHLOGE("Sys spec type is invalid!");
        return "";
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return "";
    }
    if (!data.WriteUint32((uint32_t)spec)) {
        DHLOGE("DistributedHardwareProxy write local sys spec failed");
        return "";
    }

    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::QUERY_LOCAL_SYS_SPEC),
        data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %d", ret);
        return "";
    }

    std::string specStr = reply.ReadString();
    DHLOGI("Query local sys spec %" PRIu32 ", get: %s", (uint32_t)spec, specStr.c_str());
    return specStr;
}

int32_t DistributedHardwareProxy::InitializeAVCenter(const TransRole &transRole, int32_t &engineId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteUint32((uint32_t)transRole)) {
        DHLOGE("Write trans role failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::INIT_CTL_CEN), data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }
    engineId = reply.ReadInt32();

    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::ReleaseAVCenter(int32_t engineId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteInt32(engineId)) {
        DHLOGE("Write engine id failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::RELEASE_CTL_CEN), data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::CreateControlChannel(int32_t engineId, const std::string &peerDevId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteInt32(engineId)) {
        DHLOGE("Write engine id failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteString(peerDevId)) {
        DHLOGE("Write peerDevId failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::CREATE_CTL_CEN_CHANNEL),
        data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::NotifyAVCenter(int32_t engineId, const AVTransEvent &event)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteInt32(engineId)) {
        DHLOGE("Write engine id failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteUint32((uint32_t)event.type)) {
        DHLOGE("Write event type failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteString(event.content)) {
        DHLOGE("Write event content failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteString(event.peerDevId)) {
        DHLOGE("Write event peerDevId failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::NOTIFY_AV_EVENT), data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::RegisterCtlCenterCallback(int32_t engineId,
    const sptr<IAVTransControlCenterCallback> &callback)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteInt32(engineId)) {
        DHLOGE("Write engine id failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        DHLOGE("Write callback failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::REGISTER_CTL_CEN_CALLBACK),
        data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::NotifySourceRemoteSinkStarted(std::string &deviceId)
{
    DHLOGI("DistributedHardwareProxy NotifySourceRemoteSinkStarted Start");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("NotifySourceRemoteSinkStarted error, remote info is null");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_FWK_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteString(deviceId)) {
        DHLOGE("Write deviceId error.");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DHMsgInterfaceCode::NOTIFY_SOURCE_DEVICE_REMOTE_DMSDP_STARTED), data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }
    DHLOGI("DistributedHardwareProxy NotifySourceRemoteSinkStarted End");
    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::PauseDistributedHardware(DHType dhType, const std::string &networkId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    uint32_t type = static_cast<uint32_t>(dhType);
    if (!data.WriteInt32(type)) {
        DHLOGE("Write type failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteString(networkId)) {
        DHLOGE("Write networkId failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::PAUSE_DISTRIBUTED_HARDWARE),
        data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::ResumeDistributedHardware(DHType dhType, const std::string &networkId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    uint32_t type = static_cast<uint32_t>(dhType);
    if (!data.WriteInt32(type)) {
        DHLOGE("Write type failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteString(networkId)) {
        DHLOGE("Write networkId failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::RESUME_DISTRIBUTED_HARDWARE),
        data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::StopDistributedHardware(DHType dhType, const std::string &networkId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    int32_t type = static_cast<int32_t>(dhType);
    if (!data.WriteInt32(type)) {
        DHLOGE("Write type failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteString(networkId)) {
        DHLOGE("Write networkId failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::STOP_DISTRIBUTED_HARDWARE),
        data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    return reply.ReadInt32();
}
} // namespace DistributedHardware
} // namespace OHOS