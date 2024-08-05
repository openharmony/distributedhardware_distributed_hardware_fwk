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

#include "distributed_hardware_fwk_kit.h"

#include <cinttypes>

#include "anonymous_string.h"
#include "constants.h"
#include "dh_utils_tool.h"
#include "dhfwk_sa_manager.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "dh_utils_hisysevent.h"
#include "idistributed_hardware.h"

namespace OHOS {
namespace DistributedHardware {
DistributedHardwareFwkKit::DistributedHardwareFwkKit() : isDHFWKOnLine_(false)
{
    DHLOGI("Ctor DistributedHardwareFwkKit");
    DHFWKSAManager::GetInstance().RegisterSAStateCallback([this](bool isOnLine) { this->OnDHFWKOnLine(isOnLine); });
    DHFWKSAManager::GetInstance().RegisterAbilityListener();
}

DistributedHardwareFwkKit::~DistributedHardwareFwkKit()
{
    DHLOGI("Dtor DistributedHardwareFwkKit");
}

int32_t DistributedHardwareFwkKit::RegisterPublisherListener(const DHTopic topic, sptr<IPublisherListener> listener)
{
    DHLOGI("Register publisher listener, topic: %{public}" PRIu32 ", is DHFWK online: %{public}s",
        (uint32_t)topic, isDHFWKOnLine_ ? "true" : "false");
    if (!IsDHTopicValid(topic)) {
        DHLOGE("Topic invalid, topic: %{public}" PRIu32, (uint32_t)topic);
        return ERR_DH_FWK_PARA_INVALID;
    }

    int32_t ret = DH_FWK_SUCCESS;
    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() != nullptr) {
        ret = DHFWKSAManager::GetInstance().GetDHFWKProxy()->RegisterPublisherListener(topic, listener);
        DHLOGI("Register publisher listener to DHFWK, ret: %{public}" PRId32, ret);
        if (ret == DH_FWK_SUCCESS) {
            return DHFWKSAManager::GetInstance().AddPublisherListenerToCache(topic, listener);
        }
    } else {
        DHLOGI("DHFWK not online, or get proxy failed, save listener temporary");
        return DHFWKSAManager::GetInstance().AddPublisherListenerToCache(topic, listener);
    }

    return ret;
}

int32_t DistributedHardwareFwkKit::UnregisterPublisherListener(const DHTopic topic, sptr<IPublisherListener> listener)
{
    DHLOGI("Unregister publisher listener, topic: %{public}" PRIu32 ", is DHFWK online: %{public}s",
        (uint32_t)topic, isDHFWKOnLine_ ? "true" : "false");
    if (!IsDHTopicValid(topic)) {
        DHLOGE("Topic invalid, topic: %{public}" PRIu32, (uint32_t)topic);
        return ERR_DH_FWK_PARA_INVALID;
    }

    int32_t ret = DH_FWK_SUCCESS;
    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() != nullptr) {
        ret = DHFWKSAManager::GetInstance().GetDHFWKProxy()->UnregisterPublisherListener(topic, listener);
        DHLOGI("Unregister publisher listener to DHFWK, ret: %{public}" PRId32, ret);
    }

    DHFWKSAManager::GetInstance().RemovePublisherListenerFromCache(topic, listener);
    return ret;
}

int32_t DistributedHardwareFwkKit::PublishMessage(const DHTopic topic, const std::string &message)
{
    DHLOGI("Publish message, topic: %{public}" PRIu32, (uint32_t)topic);
    if (!IsDHTopicValid(topic)) {
        DHLOGE("Topic invalid, topic: %{public}" PRIu32, (uint32_t)topic);
        return ERR_DH_FWK_PARA_INVALID;
    }
    if (!IsMessageLengthValid(message)) {
        return ERR_DH_FWK_PARA_INVALID;
    }

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online, can not publish message");
        return ERR_DH_FWK_PUBLISH_MSG_FAILED;
    }

    int32_t ret = DHFWKSAManager::GetInstance().GetDHFWKProxy()->PublishMessage(topic, message);
    DHLOGI("Publish message to DHFWK, ret: %{public}" PRId32, ret);

    return ret;
}

bool DistributedHardwareFwkKit::IsDHTopicValid(DHTopic topic)
{
    return topic > DHTopic::TOPIC_MIN && topic < DHTopic::TOPIC_MAX;
}

void DistributedHardwareFwkKit::OnDHFWKOnLine(bool isOnLine)
{
    DHLOGI("Receive DHFWK online callback, %{public}s", (isOnLine ? "true" : "false"));
    isDHFWKOnLine_ = isOnLine;
}

bool DistributedHardwareFwkKit::IsQueryLocalSysSpecTypeValid(QueryLocalSysSpecType spec)
{
    return spec > QueryLocalSysSpecType::MIN && spec < QueryLocalSysSpecType::MAX;
}

std::string DistributedHardwareFwkKit::QueryLocalSysSpec(enum QueryLocalSysSpecType spec)
{
    DHLOGI("Query Local Sys Spec, %{public}d", (uint32_t)spec);
    if (!IsQueryLocalSysSpecTypeValid(spec)) {
        DHLOGE("Topic invalid, topic: %{public}" PRIu32, (uint32_t)spec);
        return "";
    }

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online, can not publish message");
        return "";
    }

    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->QueryLocalSysSpec(spec);
}

int32_t DistributedHardwareFwkKit::InitializeAVCenter(const TransRole &transRole, int32_t &engineId)
{
    DHLOGI("Initialize av control center, transRole: %{public}" PRIu32, (uint32_t)transRole);

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not initializeA av control center");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }

    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->InitializeAVCenter(transRole, engineId);
}

int32_t DistributedHardwareFwkKit::ReleaseAVCenter(int32_t engineId)
{
    DHLOGI("Release av control center, engineId: %{public}" PRId32, engineId);

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not release av control center");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }

    DHFWKSAManager::GetInstance().RemoveAVTransControlCenterCbFromCache(engineId);
    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->ReleaseAVCenter(engineId);
}

int32_t DistributedHardwareFwkKit::CreateControlChannel(int32_t engineId, const std::string &peerDevId)
{
    DHLOGI("Create av control center channel, engineId: %{public}" PRId32 ", peerDevId=%{public}s.", engineId,
        GetAnonyString(peerDevId).c_str());

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not create av control center channel");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }

    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->CreateControlChannel(engineId, peerDevId);
}

int32_t DistributedHardwareFwkKit::NotifyAVCenter(int32_t engineId, const AVTransEvent &event)
{
    DHLOGI("Notify av control center, engineId: %{public}" PRId32 ", event type=%{public}" PRId32, engineId,
        event.type);

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not notity av control center event.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }

    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->NotifyAVCenter(engineId, event);
}

int32_t DistributedHardwareFwkKit::RegisterCtlCenterCallback(int32_t engineId,
    const sptr<IAVTransControlCenterCallback> &callback)
{
    DHLOGI("Register av control center callback. engineId: %{public}" PRId32, engineId);

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not register av control center callback.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    DHFWKSAManager::GetInstance().AddAVTransControlCenterCbToCache(engineId, callback);
    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->RegisterCtlCenterCallback(engineId, callback);
}

int32_t DistributedHardwareFwkKit::PauseDistributedHardware(DHType dhType, const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("Pause distributed hardware dhType %{public}u, networkId %{public}s", (uint32_t)dhType,
        GetAnonyString(networkId).c_str());

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not register av control center callback.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    HiSysEventWriteMsg(DHFWK_INIT_END, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "user pause sink ui.");
    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->PauseDistributedHardware(dhType, networkId);
}

int32_t DistributedHardwareFwkKit::ResumeDistributedHardware(DHType dhType, const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("Resume distributed hardware dhType %{public}u, networkId %{public}s", (uint32_t)dhType,
        GetAnonyString(networkId).c_str());

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not register av control center callback.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    HiSysEventWriteMsg(DHFWK_INIT_BEGIN, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "user resume sink ui.");
    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->ResumeDistributedHardware(dhType, networkId);
}

int32_t DistributedHardwareFwkKit::StopDistributedHardware(DHType dhType, const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("Stop distributed hardware dhType %{public}u, networkId %{public}s", (uint32_t)dhType,
        GetAnonyString(networkId).c_str());

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not register av control center callback.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    HiSysEventWriteMsg(DHFWK_EXIT_END, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "user stop sink ui.");
    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->StopDistributedHardware(dhType, networkId);
}
} // DistributedHardware
} // OHOS