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
        case (uint32_t)IDistributedHardware::Message::QUERY_SINK_VERSION: {
            return QuerySinkVersionInner(reply);
        }
        case (uint32_t)IDistributedHardware::Message::REG_PUBLISHER_LISTNER: {
            return RegisterPublisherListenerInner(data);
        }
        case (uint32_t)IDistributedHardware::Message::UNREG_PUBLISHER_LISTENER: {
            return UnregisterPublisherListenerInner(data);
        }
        case (uint32_t)IDistributedHardware::Message::PUBLISH_MESSAGE: {
            return PublishMessageInner(data);
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::QuerySinkVersionInner(MessageParcel &reply)
{
    std::unordered_map<DHType, std::string> versionMap;
    QuerySinkVersion(versionMap);
    auto version = ToJson(versionMap);
    if (!reply.WriteString(version)) {
        DHLOGE("write version failed");
        return ERR_DH_FWK_SERVICE_IPC_WRITE_PARA_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::RegisterPublisherListenerInner(MessageParcel &data)
{
    uint32_t topicInt = data.ReadUint32();
    if (!ValidTopic(topicInt)) {
        DHLOGE("Topic invalid: %" PRIu32, topicInt);
        return ERR_DH_FWK_PARA_INVALID;
    }

    DHTopic topic = (DHTopic)topicInt;
    sptr<IPublisherListener> listener = iface_cast<IPublisherListener>(data.ReadRemoteObject());
    DHLOGI("Register listener, topic: %" PRIu32 , (uint32_t)topic);
    RegisterPublisherListener(topic, listener);
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::UnregisterPublisherListenerInner(MessageParcel &data)
{
    uint32_t topicInt = data.ReadUint32();
    if (!ValidTopic(topicInt)) {
        DHLOGE("Topic invalid: %" PRIu32, topicInt);
        return ERR_DH_FWK_PARA_INVALID;
    }

    DHTopic topic = (DHTopic)topicInt;
    sptr<IPublisherListener> listener = iface_cast<IPublisherListener>(data.ReadRemoteObject());
    DHLOGI("Unregister listener, topic: %" PRIu32 , (uint32_t)topic);
    UnregisterPublisherListener(topic, listener);
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareStub::PublishMessageInner(MessageParcel &data)
{
    uint32_t topicInt = data.ReadUint32();
    if (!ValidTopic(topicInt)) {
        DHLOGE("Topic invalid: %" PRIu32, topicInt);
        return ERR_DH_FWK_PARA_INVALID;
    }

    DHTopic topic = (DHTopic)topicInt;
    std::string message = data.ReadString();
    DHLOGI("Publish Message, topic: %" PRIu32 ", message: %s", (uint32_t)topic, message.c_str());
    PublishMessage(topic, message);
    return DH_FWK_SUCCESS;
}

bool DistributedHardwareStub::ValidTopic(uint32_t topic)
{
    if (topic <= (uint32_t)DHTopic::TOPIC_MIN || topic >= (uint32_t)DHTopic::TOPIC_MAX) {
        return false;
    }

    return true;
}

std::string DistributedHardwareStub::ToJson(const std::unordered_map<DHType, std::string> &versionMap) const
{
    nlohmann::json jsonObj;
    for (const auto &item : versionMap) {
        nlohmann::json json;
        json[DH_COMPONENT_TYPE] = item.first;
        json[DH_COMPONENT_SINK_VER] = item.second;
        jsonObj[DH_COMPONENT_VERSIONS].emplace_back(json);
    }
    return jsonObj.dump();
}
} // namespace DistributedHardware
} // namespace OHOS
