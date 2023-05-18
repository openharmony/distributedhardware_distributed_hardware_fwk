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

#include "constants.h"
#include "distributed_hardware_log.h"
#include "publisher_listener_proxy.h"

namespace OHOS {
namespace DistributedHardware {
PublisherListenerProxy::PublisherListenerProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IPublisherListener>(object)
{
}

PublisherListenerProxy::~PublisherListenerProxy()
{
}

void PublisherListenerProxy::OnMessage(const DHTopic topic, const std::string& message)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("Get Remote IRemoteObject failed");
        return;
    }
    if (topic < DHTopic::TOPIC_MIN || topic > DHTopic::TOPIC_MAX) {
        DHLOGE("Topic is invalid!");
        return;
    }
    if (message.size() == 0 || message.size() > MAX_MESSAGE_LEN) {
        DHLOGE("Message is invalid");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("PublisherListenerProxy write token failed");
        return;
    }

    if (!data.WriteUint32((uint32_t)topic)) {
        DHLOGE("Parcel write topic failed");
        return;
    }

    if (!data.WriteString(message)) {
        DHLOGE("Parcel write message failed");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IPublisherListener::Message::ON_MESSAGE), data, reply, option);
    if (ret != 0) {
        DHLOGE("PublisherListenerProxy send requeset failed, ret: %d", ret);
        return;
    }
}
} // namespace DistributedHardware
} // namespace OHOS