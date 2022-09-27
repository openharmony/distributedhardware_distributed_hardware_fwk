/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "publisher_listener_proxy.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

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
        case (uint32_t)IDistributedHardware::Message::REG_PUBLISHER_LISTNER: {
            return RegisterPublisherListenerInner(data, reply);
        }
        case (uint32_t)IDistributedHardware::Message::UNREG_PUBLISHER_LISTENER: {
            return UnregisterPublisherListenerInner(data, reply);
        }
        case (uint32_t)IDistributedHardware::Message::PUBLISH_MESSAGE: {
            return PublishMessageInner(data, reply);
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
    DHLOGI("Publish Message, topic: %" PRIu32 ", message: %s", (uint32_t)topic, message.c_str());
    PublishMessage(topic, message);
    reply.WriteInt32(DH_FWK_SUCCESS);
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
