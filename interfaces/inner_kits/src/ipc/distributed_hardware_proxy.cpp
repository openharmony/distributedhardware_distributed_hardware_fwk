/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "dh_utils_tool.h"
#include "dhardware_ipc_interface_code.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
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
    const sptr<IPublisherListener> listener)
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
    if (topic <= DHTopic::TOPIC_MIN || topic >= DHTopic::TOPIC_MAX) {
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
        DHLOGE("Send Request failed, ret: %{public}d", ret);
        return ERR_DH_FWK_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    ret = reply.ReadInt32();
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Register Publisher Listener failed, ret: %{public}d", ret);
    }

    return ret;
}

int32_t DistributedHardwareProxy::UnregisterPublisherListener(const DHTopic topic,
    const sptr<IPublisherListener> listener)
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
    if (topic <= DHTopic::TOPIC_MIN || topic >= DHTopic::TOPIC_MAX) {
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
        DHLOGE("Send Request failed, ret: %{public}d", ret);
        return ERR_DH_FWK_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    ret = reply.ReadInt32();
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Unregister Publisher Listener failed, ret: %{public}d", ret);
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
    if (topic <= DHTopic::TOPIC_MIN || topic >= DHTopic::TOPIC_MAX) {
        DHLOGE("Topic is invalid!");
        return ERR_DH_FWK_PARA_INVALID;
    }
    if (!IsMessageLengthValid(msg)) {
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
        DHLOGE("Send Request failed, ret: %{public}d", ret);
        return ERR_DH_FWK_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    ret = reply.ReadInt32();
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("PublishMessage failed, ret: %{public}d", ret);
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
    if (spec <= QueryLocalSysSpecType::MIN || spec >= QueryLocalSysSpecType::MAX) {
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
        DHLOGE("Send Request failed, ret: %{public}d", ret);
        return "";
    }

    std::string specStr = reply.ReadString();
    DHLOGI("Query local sys spec %{public}" PRIu32 ", get: %{public}s", (uint32_t)spec, specStr.c_str());
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
        DHLOGE("Send Request failed, ret: %{public}d", ret);
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
        DHLOGE("Send Request failed, ret: %{public}d", ret);
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
        DHLOGE("Send Request failed, ret: %{public}d", ret);
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
        DHLOGE("Send Request failed, ret: %{public}d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::RegisterCtlCenterCallback(int32_t engineId,
    const sptr<IAvTransControlCenterCallback> callback)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }
    if (callback == nullptr) {
        DHLOGE("callback is null");
        return ERR_DH_FWK_AVTRANS_CALLBACK_IS_NULL;
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
        DHLOGE("Send Request failed, ret: %{public}d", ret);
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
        DHLOGE("Send Request failed, ret: %{public}d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }
    DHLOGI("DistributedHardwareProxy NotifySourceRemoteSinkStarted End");
    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::PauseDistributedHardware(DHType dhType, const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
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
        DHLOGE("Send Request failed, ret: %{public}d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::ResumeDistributedHardware(DHType dhType, const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
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
        DHLOGE("Send Request failed, ret: %{public}d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::StopDistributedHardware(DHType dhType, const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
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
        DHLOGE("Send Request failed, ret: %{public}d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::GetDistributedHardware(const std::string &networkId, EnableStep enableStep,
    const sptr<IGetDhDescriptorsCallback> callback)
{
    DHLOGI("DistributedHardwareProxy GetDistributedHardware.");
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    if (callback == nullptr) {
        DHLOGE("get distributed hardware callback is null");
        return ERR_DH_FWK_PARA_INVALID;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null!");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteString(networkId)) {
        DHLOGE("Write networkId failed!");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(enableStep))) {
        DHLOGE("Write enableStep failed!");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        DHLOGE("Write callback failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::GET_DISTRIBUTED_HARDWARE),
        data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %{public}d!", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }
    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::RegisterDHStatusListener(sptr<IHDSinkStatusListener> listener)
{
    DHLOGI("DistributedHardwareProxy RegisterDHStatusListener.");
    if (listener == nullptr) {
        DHLOGE("status listener is null");
        return ERR_DH_FWK_STATUS_LISTENER_IS_NULL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null!");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        DHLOGE("Write listener failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::REG_DH_SINK_STATUS_LISTNER),
        data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %{public}d!", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }
    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::UnregisterDHStatusListener(sptr<IHDSinkStatusListener> listener)
{
    DHLOGI("DistributedHardwareProxy UnregisterDHStatusListener.");
    if (listener == nullptr) {
        DHLOGE("status listener is null");
        return ERR_DH_FWK_STATUS_LISTENER_IS_NULL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null!");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        DHLOGE("Write listener failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::UNREG_DH_SINK_STATUS_LISTNER),
        data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %{public}d!", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }
    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::RegisterDHStatusListener(
    const std::string &networkId, sptr<IHDSourceStatusListener> listener)
{
    DHLOGI("DistributedHardwareProxy RegisterDHStatusListener.");
    if (listener == nullptr) {
        DHLOGE("status listener is null");
        return ERR_DH_FWK_STATUS_LISTENER_IS_NULL;
    }
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null!");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteString(networkId)) {
        DHLOGE("Write networkId failed!");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        DHLOGE("Write listener failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::REG_DH_SOURCE_STATUS_LISTNER),
        data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %{public}d!", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }
    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::UnregisterDHStatusListener(
    const std::string &networkId, sptr<IHDSourceStatusListener> listener)
{
    DHLOGI("DistributedHardwareProxy UnregisterDHStatusListener.");
    if (listener == nullptr) {
        DHLOGE("status listener is null");
        return ERR_DH_FWK_STATUS_LISTENER_IS_NULL;
    }
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null!");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteString(networkId)) {
        DHLOGE("Write networkId failed!");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        DHLOGE("Write listener failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::UNREG_DH_SOURCE_STATUS_LISTNER),
        data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %{public}d!", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }
    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::EnableSink(const std::vector<DHDescriptor> &descriptors)
{
    DHLOGI("DistributedHardwareProxy EnableSink.");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null!");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (WriteDescriptors(data, descriptors)) {
        DHLOGE("WriteDescriptors failed!");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::ENABLE_SINK), data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %{public}d!", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }
    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::DisableSink(const std::vector<DHDescriptor> &descriptors)
{
    DHLOGI("DistributedHardwareProxy DisableSink.");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null!");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (WriteDescriptors(data, descriptors)) {
        DHLOGE("WriteDescriptors failed!");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::DISABLE_SINK), data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %{public}d!", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }
    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::EnableSource(
    const std::string &networkId, const std::vector<DHDescriptor> &descriptors)
{
    DHLOGI("DistributedHardwareProxy EnableSource.");
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null!");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteString(networkId)) {
        DHLOGE("Write networkId failed!");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (WriteDescriptors(data, descriptors)) {
        DHLOGE("WriteDescriptors failed!");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::ENABLE_SOURCE), data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %{public}d!", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }
    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::DisableSource(
    const std::string &networkId, const std::vector<DHDescriptor> &descriptors)
{
    DHLOGI("DistributedHardwareProxy DisableSource.");
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null!");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteString(networkId)) {
        DHLOGE("Write networkId failed!");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (WriteDescriptors(data, descriptors)) {
        DHLOGE("WriteDescriptors failed!");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::DISABLE_SOURCE), data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %{public}d!", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }
    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::LoadDistributedHDF(const DHType dhType)
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
    if (!data.WriteUint32(type)) {
        DHLOGE("Write type failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteRemoteObject(hdfDeathListenerStub_->AsObject())) {
        DHLOGE("Write publisher listener failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::LOAD_HDF),
        data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %{public}d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }
    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::UnLoadDistributedHDF(const DHType dhType)
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
    if (!data.WriteUint32(type)) {
        DHLOGE("Write type failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteRemoteObject(hdfDeathListenerStub_->AsObject())) {
        DHLOGE("Write publisher listener failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::UNLOAD_HDF),
        data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %{public}d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }
    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::ReadDescriptors(MessageParcel &data, std::vector<DHDescriptor> &descriptors)
{
    int32_t size = data.ReadInt32();
    if (size > int32_t(MAX_DH_DESCRIPTOR_ARRAY_SIZE)) {
        DHLOGE("The array descriptors are too large, size: %{public}d!", size);
        return ERR_DH_FWK_PARA_INVALID;
    }
    for (int32_t i = 0; i < size; ++i) {
        DHDescriptor descriptor;
        descriptor.dhType = static_cast<DHType>(data.ReadInt32());
        descriptor.id = data.ReadString();
        descriptors.push_back(descriptor);
    }
    return NO_ERROR;
}

int32_t DistributedHardwareProxy::WriteDescriptors(MessageParcel &data, const std::vector<DHDescriptor> &descriptors)
{
    int32_t size = (int32_t)descriptors.size();
    if (size > int32_t(MAX_DH_DESCRIPTOR_ARRAY_SIZE)) {
        DHLOGE("The array descriptors are too large, size: %{public}d!", size);
        return ERR_DH_FWK_PARA_INVALID;
    }
    if (!data.WriteInt32(size)) {
        DHLOGE("Write descriptors size failed!");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    for (int32_t i = 0; i < size; ++i) {
        const DHDescriptor &descriptor = descriptors.at(i);
        int32_t type = static_cast<int32_t>(descriptor.dhType);
        if (!data.WriteInt32(type)) {
            DHLOGE("Write descriptor.dhType failed!");
            return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
        }
        if (!data.WriteString(descriptor.id)) {
            DHLOGE("Write descriptor.id failed!");
            return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
        }
    }
    return NO_ERROR;
}

int32_t DistributedHardwareProxy::LoadSinkDMSDPService(const std::string &udid)
{
    DHLOGI("LoadSinkDMSDPService Start");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("LoadSinkDMSDPService error, remote info is null");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_FWK_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteString(udid)) {
        DHLOGE("Write deviceId error.");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DHMsgInterfaceCode::INIT_SINK_DMSDP), data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %{public}d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }
    DHLOGI("LoadSinkDMSDPService End");
    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::NotifySinkRemoteSourceStarted(const std::string &udid)
{
    DHLOGI("NotifySinkRemoteSourceStarted Start");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("NotifySinkRemoteSourceStarted error, remote info is null");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_FWK_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteString(udid)) {
        DHLOGE("Write deviceId error.");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DHMsgInterfaceCode::NOTIFY_SINK_DEVICE_REMOTE_DMSDP_STARTED), data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %{public}d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }
    DHLOGI("NotifySinkRemoteSourceStarted End");
    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::RegisterHardwareAccessListener(const DHType dhType,
    sptr<IAuthorizationResultCallback> callback, int32_t &timeOut, const std::string &pkgName)
{
    DHLOGI("RegisterHardwareAccessListener Start");
    if (callback == nullptr) {
        DHLOGE("status listener is null");
        return ERR_DH_FWK_STATUS_LISTENER_IS_NULL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("RegisterHardwareAccessListener error, remote info is null");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_FWK_SERVICE_WRITE_TOKEN_FAIL;
    }
    uint32_t type = static_cast<uint32_t>(dhType);
    if (!data.WriteUint32(type)) {
        DHLOGE("Write type failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        DHLOGE("Write callback failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteInt32(timeOut)) {
        DHLOGE("Write timeOut failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteString(pkgName)) {
        DHLOGE("Write pkgName failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DHMsgInterfaceCode::REGISTER_HARDWARE_ACCESS_LISTENER), data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %{public}d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }
    DHLOGI("RegisterHardwareAccessListener End");
    return reply.ReadInt32();
}

int32_t DistributedHardwareProxy::UnregisterHardwareAccessListener(const DHType dhType,
    sptr<IAuthorizationResultCallback> callback, const std::string &pkgName)
{
    DHLOGI("UnregisterHardwareAccessListener Start");
    if (callback == nullptr) {
        DHLOGE("status listener is null");
        return ERR_DH_FWK_STATUS_LISTENER_IS_NULL;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("UnregisterHardwareAccessListener error, remote info is null");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_FWK_SERVICE_WRITE_TOKEN_FAIL;
    }
    uint32_t type = static_cast<uint32_t>(dhType);
    if (!data.WriteUint32(type)) {
        DHLOGE("Write type failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        DHLOGE("Write callback failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteString(pkgName)) {
        DHLOGE("Write pkgName failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DHMsgInterfaceCode::UNREGISTER_HARDWARE_ACCESS_LISTENER), data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %{public}d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }
    DHLOGI("UnregisterHardwareAccessListener End");
    return reply.ReadInt32();
}

void DistributedHardwareProxy::SetAuthorizationResult(const DHType dhType, const std::string &requestId, bool &granted)
{
    DHLOGI("SetAuthorizationResult Start");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("SetAuthorizationResult error, remote info is null");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return;
    }
    uint32_t type = static_cast<uint32_t>(dhType);
    if (!data.WriteUint32(type)) {
        DHLOGE("Write type failed");
        return;
    }
    if (!data.WriteString(requestId)) {
        DHLOGE("Write requestId failed!");
        return;
    }
    if (!data.WriteBool(granted)) {
        DHLOGE("Write granted failed");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DHMsgInterfaceCode::SET_AUTHORIZATION_RESULT), data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %{public}d", ret);
        return;
    }
    DHLOGI("OnHardwareAccessReques End");
    return;
}
} // namespace DistributedHardware
} // namespace OHOS