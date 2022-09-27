/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "publisher_item.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
PublisherItem::PublisherItem() : topic_(DHTopic::TOPIC_MIN)
{
}

PublisherItem::PublisherItem(DHTopic topic) : topic_(topic)
{
    DHLOGE("Ctor PublisherItem, topic: %d", topic);
}

PublisherItem::~PublisherItem()
{
    DHLOGE("Dtor PublisherItem, topic: %d", topic_);
    std::lock_guard<std::mutex> lock(mutex_);
    listeners_.clear();
}

void PublisherItem::AddListener(const sptr<IPublisherListener> &listener)
{
    if (listener == nullptr) {
        DHLOGE("Add null publisher listener");
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    listeners_.insert(listener);
}

void PublisherItem::RemoveListener(const sptr<IPublisherListener> &listener)
{
    if (listener == nullptr) {
        DHLOGE("Remove null publisher listener");
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto &lis : listeners_) {
        if (lis->AsObject().GetRefPtr() == listener->AsObject().GetRefPtr()) {
            listeners_.erase(lis);
            break;
        }
    }
}

void PublisherItem::PublishMessage(const std::string &message)
{
    if (message.size() == 0 || message.size() > MAX_MESSAGE_LEN) {
        DHLOGE("Message is invalid");
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto &listener : listeners_) {
        listener->OnMessage(topic_, message);
    }
}
} // DistributedHardware
} // OHOS