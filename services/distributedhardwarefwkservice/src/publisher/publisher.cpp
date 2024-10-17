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

#include "publisher.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(Publisher);
Publisher::Publisher() : publisherItems_({
        { DHTopic::TOPIC_START_DSCREEN, std::make_shared<PublisherItem>(DHTopic::TOPIC_START_DSCREEN) },
        { DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO,
            std::make_shared<PublisherItem>(DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO) },
        { DHTopic::TOPIC_STOP_DSCREEN, std::make_shared<PublisherItem>(DHTopic::TOPIC_STOP_DSCREEN) },
        { DHTopic::TOPIC_DEV_OFFLINE, std::make_shared<PublisherItem>(DHTopic::TOPIC_DEV_OFFLINE) },
        { DHTopic::TOPIC_LOW_LATENCY, std::make_shared<PublisherItem>(DHTopic::TOPIC_LOW_LATENCY) },
        { DHTopic::TOPIC_INIT_DHMS_READY, std::make_shared<PublisherItem>(DHTopic::TOPIC_INIT_DHMS_READY) },
        { DHTopic::TOPIC_PHY_DEV_PLUGIN, std::make_shared<PublisherItem>(DHTopic::TOPIC_PHY_DEV_PLUGIN) },
        { DHTopic::TOPIC_ISOMERISM, std::make_shared<PublisherItem>(DHTopic::TOPIC_ISOMERISM) }
    })
{
}

Publisher::~Publisher()
{
}

void Publisher::RegisterListener(const DHTopic topic, const sptr<IPublisherListener> listener)
{
    publisherItems_[topic]->AddListener(listener);
}

void Publisher::UnregisterListener(const DHTopic topic, const sptr<IPublisherListener> listener)
{
    publisherItems_[topic]->RemoveListener(listener);
}

void Publisher::PublishMessage(const DHTopic topic, const std::string &message)
{
    publisherItems_[topic]->PublishMessage(message);
}
} // namespace DistributedHardware
} // namespace OHOS