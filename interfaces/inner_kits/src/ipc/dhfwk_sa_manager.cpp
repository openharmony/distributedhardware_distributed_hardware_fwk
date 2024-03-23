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

#include "dhfwk_sa_manager.h"

#include <cinttypes>

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "constants.h"
#include "distributed_hardware_log.h"
#include "distributed_hardware_proxy.h"
#include "distributed_hardware_errno.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DHFWKSAManager);
DHFWKSAManager::DHFWKSAManager()
    : dhfwkOnLine_(false), isSubscribeDHFWKSAChangeListener(false), dhfwkProxy_(nullptr),
      saListener_(new SystemAbilityListener()), saStateCallback(nullptr),
      publisherListenersCache_({}), avTransControlCenterCbCache_({})
{
    DHLOGI("Ctor DHFWKSAManager");
}
DHFWKSAManager::~DHFWKSAManager()
{
    DHLOGI("Dtor DHFWKSAManager");
    dhfwkOnLine_ = false;
    isSubscribeDHFWKSAChangeListener = false;
    dhfwkProxy_ = nullptr;
    saListener_ = nullptr;
}

void DHFWKSAManager::RegisterAbilityListener()
{
    DHLOGI("Register DHFWK sa listener");
    sptr<ISystemAbilityManager> saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saMgr == nullptr) {
        DHLOGE("Get System Ability Manager failed");
        return;
    }

    if (!isSubscribeDHFWKSAChangeListener) {
        DHLOGI("try subscribe sa change listener, sa id: %{public}d", DISTRIBUTED_HARDWARE_SA_ID);
        int32_t ret = saMgr->SubscribeSystemAbility(DISTRIBUTED_HARDWARE_SA_ID, saListener_);
        if (ret != 0) {
            DHLOGE("subscribe DHFWK sa change listener failed, ret: %{public}d", ret);
            return;
        }
        isSubscribeDHFWKSAChangeListener = true;
    }
}

sptr<IDistributedHardware> DHFWKSAManager::GetDHFWKProxy()
{
    if (!isSubscribeDHFWKSAChangeListener) {
        RegisterAbilityListener();
    }

    std::lock_guard<std::mutex> lock(proxyMutex_);
    if (dhfwkProxy_ == nullptr) {
        sptr<ISystemAbilityManager> saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (saMgr == nullptr) {
            DHLOGE("Get System Ability Manager failed");
            return nullptr;
        }
        DHLOGI("Try get DHFWK sa");
        sptr<IRemoteObject> remoteObject = saMgr->CheckSystemAbility(DISTRIBUTED_HARDWARE_SA_ID);
        if (remoteObject == nullptr) {
            DHLOGE("Get DHFWK proxy return null");
            return nullptr;
        }

        dhfwkProxy_ = iface_cast<IDistributedHardware>(remoteObject);
        if (!dhfwkProxy_ || !dhfwkProxy_->AsObject()) {
            DHLOGE("Failed to Get DHFWK Proxy");
            return nullptr;
        }
    }

    return dhfwkProxy_;
}

void DHFWKSAManager::RegisterSAStateCallback(DHFWKSAStateCb callback)
{
    std::lock_guard<std::mutex> lock(saStatCbMutex_);
    saStateCallback = callback;
}

void DHFWKSAManager::SystemAbilityListener::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    (void)deviceId;
    if (systemAbilityId != DISTRIBUTED_HARDWARE_SA_ID) {
        DHLOGW("Receive SA Start, but sa id is not DHFWK, id: %{public}" PRId32, systemAbilityId);
        return;
    }

    DHFWKSAManager::GetInstance().dhfwkOnLine_ = true;
    {
        std::lock_guard<std::mutex> lock(DHFWKSAManager::GetInstance().saStatCbMutex_);
        if (DHFWKSAManager::GetInstance().saStateCallback != nullptr) {
            DHFWKSAManager::GetInstance().saStateCallback(true);
        }
    }
    if (DHFWKSAManager::GetInstance().RestoreListener() != DH_FWK_SUCCESS) {
        DHLOGE("Partial listeners failed to restore");
    }
    DHLOGI("sa %{public}" PRId32 " started", systemAbilityId);
}

void DHFWKSAManager::SystemAbilityListener::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    (void)deviceId;
    if (systemAbilityId != DISTRIBUTED_HARDWARE_SA_ID) {
        DHLOGW("Receive SA Stop, but sa id is not DHFWK, id: %{public}" PRId32, systemAbilityId);
        return;
    }

    DHFWKSAManager::GetInstance().dhfwkOnLine_ = false;
    {
        std::lock_guard<std::mutex> lock(DHFWKSAManager::GetInstance().proxyMutex_);
        DHFWKSAManager::GetInstance().dhfwkProxy_ = nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(DHFWKSAManager::GetInstance().saStatCbMutex_);
        if (DHFWKSAManager::GetInstance().saStateCallback != nullptr) {
            DHFWKSAManager::GetInstance().saStateCallback(false);
        }
    }
    DHLOGI("sa %{public}" PRId32 " stopped", systemAbilityId);
}

int32_t DHFWKSAManager::RestoreListener()
{
    DHLOGI("Restore the failed listeners due to sa crash");
    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    int32_t ret = DH_FWK_SUCCESS;
    {
        std::lock_guard<std::mutex> publisherListenersLock(publisherListenersMutex_);
        for (const auto &entry : publisherListenersCache_) {
            for (const auto &listener : entry.second) {
                int32_t innerRet =
                    DHFWKSAManager::GetInstance().GetDHFWKProxy()->RegisterPublisherListener(entry.first, listener);
                if (ret != DH_FWK_SUCCESS) {
                    ret = innerRet;
                    DHLOGE("Register publisher listener failed, topic: %{public}" PRIu32, (uint32_t)entry.first);
                }
            }
        }
    }
    {
        std::lock_guard<std::mutex> avTransControlCenterCbLock(avTransControlCenterCbMutex_);
        for (auto &entry : avTransControlCenterCbCache_) {
            int32_t innerRet =
                DHFWKSAManager::GetInstance().GetDHFWKProxy()->RegisterCtlCenterCallback(entry.first, entry.second);
            if (innerRet != DH_FWK_SUCCESS) {
                ret = innerRet;
                DHLOGE("Restore register av control center callback failed, engineId: %{public}" PRId32, entry.first);
            }
        }
    }
    return ret;
}

int32_t DHFWKSAManager::AddPublisherListenerToCache(const DHTopic topic, sptr<IPublisherListener> listener)
{
    std::lock_guard<std::mutex> publisherListenersLock(publisherListenersMutex_);
    if (publisherListenersCache_.size() >= MAX_TOPIC_SIZE ||
        publisherListenersCache_[topic].size() >= MAX_LISTENER_SIZE) {
        DHLOGE("listeners are over size!");
        return ERR_DH_FWK_PUBLISH_LISTENER_OVER_SIZE;
    }
    publisherListenersCache_[topic].insert(listener);
    return DH_FWK_SUCCESS;
}

void DHFWKSAManager::RemovePublisherListenerFromCache(const DHTopic topic, sptr<IPublisherListener> listener)
{
    std::lock_guard<std::mutex> publisherListenersLock(publisherListenersMutex_);
    publisherListenersCache_[topic].erase(listener);
}

void DHFWKSAManager::AddAVTransControlCenterCbToCache(int32_t engineId,
    const sptr<IAVTransControlCenterCallback> callback)
{
    std::lock_guard<std::mutex> avTransControlCenterCbLock(avTransControlCenterCbMutex_);
    avTransControlCenterCbCache_[engineId] = callback;
}

void DHFWKSAManager::RemoveAVTransControlCenterCbFromCache(int32_t engineId)
{
    std::lock_guard<std::mutex> avTransControlCenterCbLock(avTransControlCenterCbMutex_);
    avTransControlCenterCbCache_.erase(engineId);
}
} // DistributedHardware
} // OHOS