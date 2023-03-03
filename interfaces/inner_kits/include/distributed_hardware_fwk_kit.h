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

#ifndef OHOS_DISTRIBUTED_HARDWARE_FWK_KIT_H
#define OHOS_DISTRIBUTED_HARDWARE_FWK_KIT_H

#include <atomic>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <set>
#include "refbase.h"

#include "ipublisher_listener.h"
#include "idistributed_hardware.h"

#ifndef API_EXPORT
#define API_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace DistributedHardware {
class DistributedHardwareFwkKit final {
public:
    /**
     * @brief Constructor.
     * @return No return value.
     */
    API_EXPORT DistributedHardwareFwkKit();

    /**
     * @brief Destructor.
     * @return No return value.
     */
    API_EXPORT ~DistributedHardwareFwkKit();

    /**
     * @brief Register publisher listener.
     * @param topic    Distributed hardware topic.
     * @param listener Publisher listener.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t RegisterPublisherListener(const DHTopic topic, sptr<IPublisherListener> listener);

    /**
     * @brief Unregister publisher listener.
     * @param topic    Distributed hardware topic.
     * @param listener Publisher listener.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t UnregisterPublisherListener(const DHTopic topic, sptr<IPublisherListener> listener);

    /**
     * @brief Publish message.
     * @param topic   Distributed hardware topic.
     * @param message Message content.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t PublishMessage(const DHTopic topic, const std::string &message);

    /**
     * @brief Distributed hardware framework online.
     * @param isOnLine Online or not.
     * @return No return value.
     */
    void OnDHFWKOnLine(bool isOnLine);
private:
    /**
     * @brief Determine whether the topic is valid.
     * @param topic Distributed hardware topic.
     * @return Returns true if success.
     */
    bool IsDHTopicValid(DHTopic topic);

private:
    std::mutex listenerMutex_;
    std::unordered_map<DHTopic, std::set<sptr<IPublisherListener>>> listenerMap_;
    std::atomic<bool> isDHFWKOnLine_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_HARDWARE_FWK_KIT_H