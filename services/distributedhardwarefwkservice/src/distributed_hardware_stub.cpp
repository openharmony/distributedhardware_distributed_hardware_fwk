/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "device_manager.h"

#include "anonymous_string.h"
#include "constants.h"
#include "dhardware_ipc_interface_code.h"
#include "dh_context.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "hdf_operate.h"
#include "publisher_listener_proxy.h"
#include "av_trans_errno.h"

namespace OHOS {
namespace DistributedHardware {
int32_t DistributedHardwareStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    DHLOGI("OnRemoteRequest, code = %{public}u.", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        DHLOGE("IPC Token valid fail!");
        return ERR_INVALID_DATA;
    }
    if (code != static_cast<uint32_t>(DHMsgInterfaceCode::NOTIFY_SOURCE_DEVICE_REMOTE_DMSDP_STARTED) &&
        code != static_cast<uint32_t>(DHMsgInterfaceCode::INIT_SINK_DMSDP) &&
        code != static_cast<uint32_t>(DHMsgInterfaceCode::NOTIFY_SINK_DEVICE_REMOTE_DMSDP_STARTED)) {
        if (!IPCSkeleton::IsLocalCalling()) {
            DHLOGE("Invalid request, only support local, code = %{public}u.", code);
            return ERR_DH_FWK_IS_LOCAL_PROCESS_FAIL;
        }
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
        default:
            return OnRemoteRequestEx(code, data, reply, option);
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
    sptr<IAvTransControlCenterCallback> callback = iface_cast<IAvTransControlCenterCallback>(data.ReadRemoteObject());
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
    DHLOGI("Notify source device remote sink DMSDP start.");
    std::string udid = data.ReadString();
    if (!IsIdLengthValid(udid)) {
        DHLOGE("The udid is invalid.");
        return ERR_DH_FWK_PARA_INVALID;
    }
    int32_t ret = NotifySourceRemoteSinkStarted(udid);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("write ret failed.");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    DHLOGI("Notify source device End.");
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
    if (!IsSystemHap() && !IsNativeSA()) {
        DHLOGE("GetCallerProcessName not system hap or not native sa.");
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

int32_t DistributedHardwareStub::GetDistributedHardwareInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    std::string networkId = data.ReadString();
    EnableStep enableStep = static_cast<EnableStep>(data.ReadUint32());
    sptr<IGetDhDescriptorsCallback> callback =
        iface_cast<IGetDhDescriptorsCallback>(data.ReadRemoteObject());
    if (callback == nullptr) {
        DHLOGE("Input get distributed hardware callback is null!");
        return ERR_DH_FWK_PARA_INVALID;
    }
    int32_t ret = GetDistributedHardware(networkId, enableStep, callback);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::RegisterDHStatusSinkListenerInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    sptr<IHDSinkStatusListener> listener = iface_cast<IHDSinkStatusListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        DHLOGE("Input distributed hardware status sink listener is null");
        return ERR_DH_FWK_PARA_INVALID;
    }
    int32_t ret = RegisterDHStatusListener(listener);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::UnregisterDHStatusSinkListenerInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    sptr<IHDSinkStatusListener> listener = iface_cast<IHDSinkStatusListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        DHLOGE("Input distributed hardware status sink listener is null");
        return ERR_DH_FWK_PARA_INVALID;
    }
    int32_t ret = UnregisterDHStatusListener(listener);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::RegisterDHStatusSourceListenerInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    std::string networkId = data.ReadString();
    sptr<IHDSourceStatusListener> listener = iface_cast<IHDSourceStatusListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        DHLOGE("Input distributed hardware status source listener is null");
        return ERR_DH_FWK_PARA_INVALID;
    }
    int32_t ret = RegisterDHStatusListener(networkId, listener);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::UnregisterDHStatusSourceListenerInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    std::string networkId = data.ReadString();
    sptr<IHDSourceStatusListener> listener = iface_cast<IHDSourceStatusListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        DHLOGE("Input distributed hardware status source listener is null");
        return ERR_DH_FWK_PARA_INVALID;
    }
    int32_t ret = UnregisterDHStatusListener(networkId, listener);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::EnableSinkInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    std::vector<DHDescriptor> descriptors;
    ReadDescriptors(data, descriptors);
    int32_t ret = EnableSink(descriptors);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::DisableSinkInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    std::vector<DHDescriptor> descriptors;
    ReadDescriptors(data, descriptors);
    int32_t ret = DisableSink(descriptors);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::EnableSourceInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    std::string networkId = data.ReadString();
    std::vector<DHDescriptor> descriptors;
    ReadDescriptors(data, descriptors);
    int32_t ret = EnableSource(networkId, descriptors);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::DisableSourceInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    std::string networkId = data.ReadString();
    std::vector<DHDescriptor> descriptors;
    ReadDescriptors(data, descriptors);
    int32_t ret = DisableSource(networkId, descriptors);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::LoadDistributedHDFInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    DHType dhType = static_cast<DHType>(data.ReadUint32());
    int32_t ret = LoadDistributedHDF(dhType);
    if (ret == DH_FWK_SUCCESS) {
        sptr<IRemoteObject> remoteObj = data.ReadRemoteObject();
        if (remoteObj != nullptr) {
            ret = HdfOperateManager::GetInstance().AddDeathRecipient(dhType, remoteObj);
            if (ret != DH_FWK_SUCCESS) {
                DHLOGE("AddDeathRecipient failed!");
                UnLoadDistributedHDF(dhType);
            }
        }
    }
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::UnLoadDistributedHDFInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    DHType dhType = static_cast<DHType>(data.ReadUint32());
    int32_t ret = UnLoadDistributedHDF(dhType);
    if (ret == DH_FWK_SUCCESS) {
        sptr<IRemoteObject> remoteObj = data.ReadRemoteObject();
        if (remoteObj != nullptr) {
            ret = HdfOperateManager::GetInstance().RemoveDeathRecipient(dhType, remoteObj);
            if (ret != DH_FWK_SUCCESS) {
                DHLOGE("RemoveDeathRecipient failed!");
            }
        }
    }
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::RegisterHardwareAccessListenerInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    DHType dhType = static_cast<DHType>(data.ReadUint32());
    sptr<IAuthorizationResultCallback> callback = iface_cast<IAuthorizationResultCallback>(data.ReadRemoteObject());
    if (callback == nullptr) {
        DHLOGE("Register hardware access listener callback is null");
        return ERR_DH_FWK_PARA_INVALID;
    }
    int32_t timeOut = data.ReadInt32();
    std::string pkgName = data.ReadString();
    int32_t ret = RegisterHardwareAccessListener(dhType, callback, timeOut, pkgName);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::UnregisterHardwareAccessListenerInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    DHType dhType = static_cast<DHType>(data.ReadUint32());
    sptr<IAuthorizationResultCallback> callback = iface_cast<IAuthorizationResultCallback>(data.ReadRemoteObject());
    if (callback == nullptr) {
        DHLOGE("Unregister hardware access listener callback is null");
        return ERR_DH_FWK_PARA_INVALID;
    }
    std::string pkgName = data.ReadString();
    int32_t ret = UnregisterHardwareAccessListener(dhType, callback, pkgName);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed!");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::SetAuthorizationResultInner(MessageParcel &data, MessageParcel &reply)
{
    if (!HasAccessDHPermission()) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    DHType dhType = static_cast<DHType>(data.ReadUint32());
    std::string requestId = data.ReadString();
    bool granted = data.ReadBool();
    SetAuthorizationResult(dhType, requestId, granted);
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::ReadDescriptors(MessageParcel &data, std::vector<DHDescriptor> &descriptors)
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

int32_t DistributedHardwareStub::WriteDescriptors(MessageParcel &data, const std::vector<DHDescriptor> &descriptors)
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

int32_t DistributedHardwareStub::HandleLoadSinkDMSDPService(MessageParcel &data, MessageParcel &reply)
{
    DHLOGI("Load sink DMSDP service start");
    std::string udid = data.ReadString();
    if (!IsIdLengthValid(udid)) {
        DHLOGE("The udid is invalid.");
        return ERR_DH_FWK_PARA_INVALID;
    }
    int32_t ret = LoadSinkDMSDPService(udid);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("write ret failed.");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    DHLOGI("Load sink DMSDP service end.");
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::HandleNotifySinkRemoteSourceStarted(MessageParcel &data, MessageParcel &reply)
{
    DHLOGI("Notify sink device remote source DMSDP started.");
    std::string udid = data.ReadString();
    if (!IsIdLengthValid(udid)) {
        DHLOGE("The udid is invalid.");
        return ERR_DH_FWK_PARA_INVALID;
    }

    int32_t ret = NotifySinkRemoteSourceStarted(udid);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("write ret failed.");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    DHLOGI("Notify sink device End.");
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::OnRemoteRequestEx(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(DHMsgInterfaceCode::PAUSE_DISTRIBUTED_HARDWARE): {
            return PauseDistributedHardwareInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::RESUME_DISTRIBUTED_HARDWARE): {
            return ResumeDistributedHardwareInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::STOP_DISTRIBUTED_HARDWARE): {
            return StopDistributedHardwareInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::GET_DISTRIBUTED_HARDWARE): {
            return GetDistributedHardwareInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::REG_DH_SINK_STATUS_LISTNER): {
            return RegisterDHStatusSinkListenerInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::UNREG_DH_SINK_STATUS_LISTNER): {
            return UnregisterDHStatusSinkListenerInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::REG_DH_SOURCE_STATUS_LISTNER): {
            return RegisterDHStatusSourceListenerInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::UNREG_DH_SOURCE_STATUS_LISTNER): {
            return UnregisterDHStatusSourceListenerInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::ENABLE_SINK): {
            return EnableSinkInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::DISABLE_SINK): {
            return DisableSinkInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::ENABLE_SOURCE): {
            return EnableSourceInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::DISABLE_SOURCE): {
            return DisableSourceInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::LOAD_HDF): {
            return LoadDistributedHDFInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::UNLOAD_HDF): {
            return UnLoadDistributedHDFInner(data, reply);
        }
        default:
            return OnRemoteRequestRPC(code, data, reply, option);
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::OnRemoteRequestRPC(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(DHMsgInterfaceCode::NOTIFY_SOURCE_DEVICE_REMOTE_DMSDP_STARTED): {
            return HandleNotifySourceRemoteSinkStarted(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::INIT_SINK_DMSDP): {
            return HandleLoadSinkDMSDPService(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::NOTIFY_SINK_DEVICE_REMOTE_DMSDP_STARTED): {
            return HandleNotifySinkRemoteSourceStarted(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::REGISTER_HARDWARE_ACCESS_LISTENER): {
            return RegisterHardwareAccessListenerInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::UNREGISTER_HARDWARE_ACCESS_LISTENER): {
            return UnregisterHardwareAccessListenerInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::SET_AUTHORIZATION_RESULT): {
            return SetAuthorizationResultInner(data, reply);
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
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

bool DistributedHardwareStub::IsNativeSA()
{
    Security::AccessToken::AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::ATokenTypeEnum tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    DHLOGI("the tokenType: %{public}d", tokenType);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        return true;
    }
    DHLOGE("is not native sa");
    return false;
}
} // namespace DistributedHardware
} // namespace OHOS
