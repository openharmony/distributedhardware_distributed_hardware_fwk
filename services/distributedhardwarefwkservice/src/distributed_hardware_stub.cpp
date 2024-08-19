/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "distributed_hardware_stub.h"

#include <cinttypes>

#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "tokenid_kit.h"

#include "anonymous_string.h"
#include "constants.h"
#include "dhardware_ipc_interface_code.h"
#include "dh_context.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "publisher_listener_proxy.h"

namespace OHOS {
namespace DistributedHardware {
int32_t DistributedHardwareStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        DHLOGE("IPC Token valid fail!");
        return ERR_INVALID_DATA;
    }
    switch (code) {
        case static_cast<uint32_t>(DHMsgInterfaceCode::REG_PUBLISHER_LISTNER): {
            return RegisterPublisherListenerInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::UNREG_PUBLISHER_LISTENER): {
            return UnregisterPublisherListenerInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::PUBLISH_MESSAGE): {
            return PublishMessageInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::INIT_CTL_CEN): {
            return InitializeAVCenterInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::RELEASE_CTL_CEN): {
            return ReleaseAVCenterInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::CREATE_CTL_CEN_CHANNEL): {
            return CreateControlChannelInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::NOTIFY_AV_EVENT): {
            return NotifyAVCenterInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::REGISTER_CTL_CEN_CALLBACK): {
            return RegisterControlCenterCallbackInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::QUERY_LOCAL_SYS_SPEC): {
            return QueryLocalSysSpecInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::NOTIFY_SOURCE_DEVICE_REMOTE_DMSDP_STARTED): {
            return HandleNotifySourceRemoteSinkStarted(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::PAUSE_DISTRIBUTED_HARDWARE): {
            return PauseDistributedHardwareInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::RESUME_DISTRIBUTED_HARDWARE): {
            return ResumeDistributedHardwareInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::STOP_DISTRIBUTED_HARDWARE): {
            return StopDistributedHardwareInner(data, reply);
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::RegisterPublisherListenerInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }

    uint32_t topicInt = data.ReadUint32();
    if (!ValidTopic(topicInt)) {
        DHLOGE("Topic invalid: %{public}" PRIu32, topicInt);
        reply.WriteInt32(ERR_DH_FWK_PARA_INVALID);
        return ERR_DH_FWK_PARA_INVALID;
    }

    DHTopic topic = (DHTopic)topicInt;
    sptr<IPublisherListener> listener = iface_cast<IPublisherListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        DHLOGE("Register publisher listener is null");
        reply.WriteInt32(ERR_DH_FWK_PARA_INVALID);
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("Register listener, topic: %{public}" PRIu32, (uint32_t)topic);
    RegisterPublisherListener(topic, listener);
    reply.WriteInt32(DH_FWK_SUCCESS);
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::UnregisterPublisherListenerInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }

    uint32_t topicInt = data.ReadUint32();
    if (!ValidTopic(topicInt)) {
        DHLOGE("Topic invalid: %{public}" PRIu32, topicInt);
        reply.WriteInt32(ERR_DH_FWK_PARA_INVALID);
        return ERR_DH_FWK_PARA_INVALID;
    }

    DHTopic topic = (DHTopic)topicInt;
    sptr<IPublisherListener> listener = iface_cast<IPublisherListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        DHLOGE("Unregister publisher listener is null");
        reply.WriteInt32(ERR_DH_FWK_PARA_INVALID);
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("Unregister listener, topic: %{public}" PRIu32, (uint32_t)topic);
    UnregisterPublisherListener(topic, listener);
    reply.WriteInt32(DH_FWK_SUCCESS);
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::PublishMessageInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }

    uint32_t topicInt = data.ReadUint32();
    if (!ValidTopic(topicInt)) {
        DHLOGE("Topic invalid: %{public}" PRIu32, topicInt);
        reply.WriteInt32(ERR_DH_FWK_PARA_INVALID);
        return ERR_DH_FWK_PARA_INVALID;
    }

    DHTopic topic = (DHTopic)topicInt;
    std::string message = data.ReadString();
    DHLOGI("Publish message, topic: %{public}" PRIu32, (uint32_t)topic);
    PublishMessage(topic, message);
    reply.WriteInt32(DH_FWK_SUCCESS);
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::QueryLocalSysSpecInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }

    uint32_t specInt = data.ReadUint32();
    if (!ValidQueryLocalSpec(specInt)) {
        DHLOGE("Spec invalid: %{public}" PRIu32, specInt);
        reply.WriteInt32(ERR_DH_FWK_PARA_INVALID);
        return ERR_DH_FWK_PARA_INVALID;
    }

    QueryLocalSysSpecType spec = (QueryLocalSysSpecType)specInt;
    DHLOGI("Query Local Sys Spec: %{public}" PRIu32, (uint32_t)spec);
    std::string res = QueryLocalSysSpec(spec);
    DHLOGI("Get Local spec: %{public}s", res.c_str());
    reply.WriteString(res);
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::InitializeAVCenterInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }

    TransRole transRole = (TransRole)(data.ReadUint32());
    int32_t engineId = 0;
    int32_t ret = InitializeAVCenter(transRole, engineId);
    if (!reply.WriteInt32(engineId)) {
        DHLOGE("Write engine id failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::ReleaseAVCenterInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }

    int32_t engineId = data.ReadInt32();
    int32_t ret = ReleaseAVCenter(engineId);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::CreateControlChannelInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }

    int32_t engineId = data.ReadInt32();
    std::string peerDevId = data.ReadString();
    int32_t ret = CreateControlChannel(engineId, peerDevId);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::NotifyAVCenterInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }

    int32_t engineId = data.ReadInt32();
    uint32_t type = data.ReadUint32();
    std::string content = data.ReadString();
    std::string peerDevId = data.ReadString();
    int32_t ret = NotifyAVCenter(engineId, AVTransEvent{ (EventType)type, content, peerDevId });
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::RegisterControlCenterCallbackInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }

    int32_t engineId = data.ReadInt32();
    sptr<IAVTransControlCenterCallback> callback = iface_cast<IAVTransControlCenterCallback>(data.ReadRemoteObject());
    if (callback == nullptr) {
        DHLOGE("Input av control center callback is null");
        return ERR_DH_FWK_PARA_INVALID;
    }

    int32_t ret = RegisterCtlCenterCallback(engineId, callback);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t OHOS::DistributedHardware::DistributedHardwareStub::HandleNotifySourceRemoteSinkStarted(MessageParcel &data,
    MessageParcel &reply)
{
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    std::string udid = data.ReadString();
    std::string networkId = DHContext::GetInstance().GetNetworkIdByUDID(udid);
    uint32_t dAccessToken = Security::AccessToken::AccessTokenKit::AllocLocalTokenID(networkId,
        callerToken);
    const std::string permissionName = "ohos.permission.ACCESS_DISTRIBUTED_HARDWARE";
    int32_t result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(dAccessToken,
        permissionName);
    if (result != Security::AccessToken::PERMISSION_GRANTED) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }

    DHLOGI("DistributedHardwareStub HandleNotifySourceRemoteSinkStarted Start.");
    int32_t ret = NotifySourceRemoteSinkStarted(udid);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("write ret failed.");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    DHLOGI("DistributedHardwareStub HandleNotifySourceRemoteSinkStarted End.");
    return DH_FWK_SUCCESS;
}

bool DistributedHardwareStub::ValidTopic(uint32_t topic)
{
    if (topic <= (uint32_t)DHTopic::TOPIC_MIN || topic >= (uint32_t)DHTopic::TOPIC_MAX) {
        return false;
    }
    return true;
}

bool DistributedHardwareStub::ValidQueryLocalSpec(uint32_t spec)
{
    if (spec <= (uint32_t)QueryLocalSysSpecType::MIN || spec >= (uint32_t)QueryLocalSysSpecType::MAX) {
        return false;
    }
    return true;
}

int32_t DistributedHardwareStub::PauseDistributedHardwareInner(MessageParcel &data, MessageParcel &reply)
{
    if (!IsSystemHap()) {
        DHLOGE("GetCallerProcessName not system hap.");
        return ERR_DH_FWK_IS_SYSTEM_HAP_CHECK_FAIL;
    }
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    DHType dhType = static_cast<DHType>(data.ReadInt32());
    std::string networkId = data.ReadString();
    int32_t ret = PauseDistributedHardware(dhType, networkId);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::ResumeDistributedHardwareInner(MessageParcel &data, MessageParcel &reply)
{
    if (!IsSystemHap()) {
        DHLOGE("GetCallerProcessName not system hap.");
        return ERR_DH_FWK_IS_SYSTEM_HAP_CHECK_FAIL;
    }
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    DHType dhType = static_cast<DHType>(data.ReadInt32());
    std::string networkId = data.ReadString();
    int32_t ret = ResumeDistributedHardware(dhType, networkId);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::StopDistributedHardwareInner(MessageParcel &data, MessageParcel &reply)
{
    if (!IsSystemHap()) {
        DHLOGE("GetCallerProcessName not system hap.");
        return ERR_DH_FWK_IS_SYSTEM_HAP_CHECK_FAIL;
    }
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    DHType dhType = static_cast<DHType>(data.ReadInt32());
    std::string networkId = data.ReadString();
    int32_t ret = StopDistributedHardware(dhType, networkId);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

bool DistributedHardwareStub::HasAccessDHPermission()
{
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    const std::string permissionName = "ohos.permission.ACCESS_DISTRIBUTED_HARDWARE";
    int32_t result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken,
        permissionName);
    return (result == Security::AccessToken::PERMISSION_GRANTED);
}

bool DistributedHardwareStub::IsSystemHap()
{
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!OHOS::Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        return false;
    }
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS
