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

#ifndef OHOS_PUBLISHER_H
#define OHOS_PUBLISHER_H

#include <unordered_map>
#include <memory>

#include "ipublisher_listener.h"
#include "publisher_item.h"

namespace OHOS {
namespace DistributedHardware {
class Publisher {
DECLARE_SINGLE_INSTANCE_BASE(Publisher);
public:
    virtual ~Publisher();
    void RegisterListener(const DHTopic topic, const sptr<IPublisherListener> &listener);
    void UnregisterListener(const DHTopic topic, const sptr<IPublisherListener> &listener);
    void PublishMessage(const DHTopic topic, const std::string &message);
private:
    Publisher();
    std::unordered_map<DHTopic, std::shared_ptr<PublisherItem>> publisherItems_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_PUBLISHER_H