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

#include "distributed_hardware_fwk_kit.h"

#include <cinttypes>

#include "constants.h"
#include "dhfwk_sa_manager.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "idistributed_hardware.h"

namespace OHOS {
namespace DistributedHardware {
DistributedHardwareFwkKit::DistributedHardwareFwkKit() : listenerMap_({}), isDHFWKOnLine_(false)
{
    DHLOGI("Ctor DistributedHardwareFwkKit");
    DHFWKSAManager::GetInstance().RegisterSAStateCallback(
        std::bind(&DistributedHardwareFwkKit::OnDHFWKOnLine, this, std::placeholders::_1));
    DHFWKSAManager::GetInstance().RegisterAbilityListener();
}

DistributedHardwareFwkKit::~DistributedHardwareFwkKit()
{
    DHLOGI("Dtor DistributedHardwareFwkKit");
}

int32_t DistributedHardwareFwkKit::RegisterPublisherListener(const DHTopic topic, sptr<IPublisherListener> listener)
{
    DHLOGI("Register publisher listener, topic: %" PRIu32 ", is DHFWK online: %s",
        (uint32_t)topic, isDHFWKOnLine_ ? "true" : "false");
    if (!IsDHTopicValid(topic)) {
        DHLOGE("Topic invalid, topic: %" PRIu32, (uint32_t)topic);
        return ERR_DH_FWK_PARA_INVALID;
    }

    int32_t ret = DH_FWK_SUCCESS;
    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() != nullptr) {
        ret = DHFWKSAManager::GetInstance().GetDHFWKProxy()->RegisterPublisherListener(topic, listener);
        DHLOGI("Register publisher listener to DHFWK, ret: %" PRId32, ret);
    } else {
        DHLOGI("DHFWK not online, or get proxy failed, save listener temporary");
        std::lock_guard<std::mutex> lock(listenerMutex_);
        if (listenerMap_.size() >= MAX_TOPIC_SIZE || listenerMap_[topic].size() >= MAX_LISTENER_SIZE) {
            DHLOGE("listeners are over size!");
            return ERR_DH_FWK_PUBLISH_LISTENER_OVER_SIZE;
        }
        listenerMap_[topic].insert(listener);
    }

    return ret;
}

int32_t DistributedHardwareFwkKit::UnregisterPublisherListener(const DHTopic topic, sptr<IPublisherListener> listener)
{
    DHLOGI("Unregister publisher listener, topic: %" PRIu32 ", is DHFWK online: %s",
        (uint32_t)topic, isDHFWKOnLine_ ? "true" : "false");
    if (!IsDHTopicValid(topic)) {
        DHLOGE("Topic invalid, topic: %" PRIu32, (uint32_t)topic);
        return ERR_DH_FWK_PARA_INVALID;
    }

    int32_t ret = DH_FWK_SUCCESS;
    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() != nullptr) {
        ret = DHFWKSAManager::GetInstance().GetDHFWKProxy()->UnregisterPublisherListener(topic, listener);
        DHLOGI("Unregister publisher listener to DHFWK, ret: %" PRId32, ret);
    }
    std::lock_guard<std::mutex> lock(listenerMutex_);
    listenerMap_[topic].erase(listener);

    return ret;
}

int32_t DistributedHardwareFwkKit::PublishMessage(const DHTopic topic, const std::string &message)
{
    DHLOGI("Publish message, topic: %" PRIu32, (uint32_t)topic);
    if (!IsDHTopicValid(topic)) {
        DHLOGE("Topic invalid, topic: %" PRIu32, (uint32_t)topic);
        return ERR_DH_FWK_PARA_INVALID;
    }
    if (message.empty() || message.size() > MAX_MESSAGE_LEN) {
        DHLOGE("Message size is invalid!");
        return ERR_DH_FWK_PARA_INVALID;
    }

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online, can not publish message");
        return ERR_DH_FWK_PUBLISH_MSG_FAILED;
    }

    int32_t ret = DHFWKSAManager::GetInstance().GetDHFWKProxy()->PublishMessage(topic, message);
    DHLOGI("Publish message to DHFWK, ret: %" PRId32, ret);

    return ret;
}

bool DistributedHardwareFwkKit::IsDHTopicValid(DHTopic topic)
{
    return topic > DHTopic::TOPIC_MIN && topic < DHTopic::TOPIC_MAX;
}

void DistributedHardwareFwkKit::OnDHFWKOnLine(bool isOnLine)
{
    DHLOGI("Receive DHFWK online callback, %s", (isOnLine ? "true" : "false"));
    isDHFWKOnLine_ = isOnLine;

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        return;
    }

    DHLOGI("DHFWK online, register saved listener to it");
    std::unordered_map<DHTopic, std::set<sptr<IPublisherListener>>> regSuccListeners;
    std::lock_guard<std::mutex> lock(listenerMutex_);
    for (const auto &entry : listenerMap_) {
        for (const auto &listener : entry.second) {
            int32_t ret =
                DHFWKSAManager::GetInstance().GetDHFWKProxy()->RegisterPublisherListener(entry.first, listener);
            if (ret == DH_FWK_SUCCESS) {
                regSuccListeners[entry.first].insert(listener);
            }
        }
    }

    for (const auto &succ : regSuccListeners) {
        for (const auto &listener : succ.second) {
            listenerMap_[succ.first].erase(listener);
        }
    }
}
} // DistributedHardware
} // OHOS