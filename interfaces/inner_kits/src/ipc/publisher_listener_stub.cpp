/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "publisher_listener_stub.h"

#include "constants.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
PublisherListenerStub::PublisherListenerStub() : IRemoteStub(true)
{
}

PublisherListenerStub::~PublisherListenerStub()
{
}

int32_t PublisherListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        DHLOGE("PublisherListenerStub read valid token failed");
        return ERR_INVALID_DATA;
    }
    IPublisherListener::Message msgCode = static_cast<IPublisherListener::Message>(code);
    switch (msgCode) {
        case IPublisherListener::Message::ON_MESSAGE: {
            DHTopic topic = static_cast<DHTopic>(data.ReadUint32());
            if (topic < DHTopic::TOPIC_MIN || topic > DHTopic::TOPIC_MAX) {
                DHLOGE("Topic is invalid!");
                return ERR_INVALID_DATA;
            }
            std::string message = data.ReadString();
            if (message.empty() || message.size() > MAX_MESSAGE_LEN) {
                DHLOGE("Message is invalid!");
                return ERR_INVALID_DATA;
            }
            OnMessage(topic, message);
            break;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    return DH_FWK_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS