/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "nlohmann/json.hpp"

#include "anonymous_string.h"
#include "constants.h"
#include "dhardware_ipc_interface_code.h"
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
            return InitializeInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::RELEASE_CTL_CEN): {
            return ReleaseInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::CREATE_CTL_CEN_CHANNEL): {
            return CreateControlChannelInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::NOTIFY_AV_EVENT): {
            return NotifyInner(data, reply);
        }
        case static_cast<uint32_t>(DHMsgInterfaceCode::REGISTER_CTL_CEN_CALLBACK): {
            return RegisterControlCenterCallbackInner(data, reply);
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::RegisterPublisherListenerInner(MessageParcel &data, MessageParcel &reply)
{
    uint32_t topicInt = data.ReadUint32();
    if (!ValidTopic(topicInt)) {
        DHLOGE("Topic invalid: %" PRIu32, topicInt);
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
    DHLOGI("Register listener, topic: %" PRIu32, (uint32_t)topic);
    RegisterPublisherListener(topic, listener);
    reply.WriteInt32(DH_FWK_SUCCESS);
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::UnregisterPublisherListenerInner(MessageParcel &data, MessageParcel &reply)
{
    uint32_t topicInt = data.ReadUint32();
    if (!ValidTopic(topicInt)) {
        DHLOGE("Topic invalid: %" PRIu32, topicInt);
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
    DHLOGI("Unregister listener, topic: %" PRIu32, (uint32_t)topic);
    UnregisterPublisherListener(topic, listener);
    reply.WriteInt32(DH_FWK_SUCCESS);
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::PublishMessageInner(MessageParcel &data, MessageParcel &reply)
{
    uint32_t topicInt = data.ReadUint32();
    if (!ValidTopic(topicInt)) {
        DHLOGE("Topic invalid: %" PRIu32, topicInt);
        reply.WriteInt32(ERR_DH_FWK_PARA_INVALID);
        return ERR_DH_FWK_PARA_INVALID;
    }

    DHTopic topic = (DHTopic)topicInt;
    std::string message = data.ReadString();
    DHLOGI("Publish message, topic: %" PRIu32, (uint32_t)topic);
    PublishMessage(topic, message);
    reply.WriteInt32(DH_FWK_SUCCESS);
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::InitializeInner(MessageParcel &data, MessageParcel &reply)
{
	TransRole transRole = (TransRole)(data.ReadUint32());
    int32_t engineId = 0;
    int32_t ret = Initialize(transRole, engineId);
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

int32_t DistributedHardwareStub::ReleaseInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t engineId = data.ReadInt32();
    int32_t ret = Release(engineId);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::CreateControlChannelInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t engineId = data.ReadInt32();
    std::string peerDevId = data.ReadString();
    int32_t ret = CreateControlChannel(engineId, peerDevId);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::NotifyInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t engineId = data.ReadInt32();
    uint32_t type = data.ReadUint32();
    std::string content = data.ReadString();
    std::string peerDevId = data.ReadString();
    int32_t ret = Notify(engineId, AVTransEvent{ (EventType)type, content, peerDevId });
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::RegisterControlCenterCallbackInner(MessageParcel &data, MessageParcel &reply)
{
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

bool DistributedHardwareStub::ValidTopic(uint32_t topic)
{
    if (topic <= (uint32_t)DHTopic::TOPIC_MIN || topic >= (uint32_t)DHTopic::TOPIC_MAX) {
        return false;
    }
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS
