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

#include "dhfwk_sa_manager.h"

#include <cinttypes>

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "distributed_hardware_log.h"
#include "distributed_hardware_proxy.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_fwk_kit.h"

namespace OHOS {
namespace DistributedHardware {
std::mutex DHFWKSAManager::publisherListenersMutex_;
std::unordered_map<DHTopic, std::set<sptr<IPublisherListener>>> DHFWKSAManager::publisherListenersCahce_;
std::mutex DHFWKSAManager::avTransControlCenterCbMutex_;
std::unordered_map<int32_t, sptr<IAVTransControlCenterCallback>> DHFWKSAManager::avTransControlCenterCbCache_;

IMPLEMENT_SINGLE_INSTANCE(DHFWKSAManager);
DHFWKSAManager::DHFWKSAManager()
    : dhfwkOnLine_(false), isSubscribeDHFWKSAChangeListener(false), dhfwkProxy_(nullptr),
      saListener_(new SystemAbilityListener()), saStateCallback(nullptr)
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
        DHLOGI("try subscribe sa change listener, sa id: %d", DISTRIBUTED_HARDWARE_SA_ID);
        int32_t ret = saMgr->SubscribeSystemAbility(DISTRIBUTED_HARDWARE_SA_ID, saListener_);
        if (ret != 0) {
            DHLOGE("subscribe DHFWK sa change listener failed, ret: %d", ret);
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
        DHLOGW("Receive SA Start, but sa id is not DHFWK, id: %" PRId32, systemAbilityId);
        return;
    }

    DHFWKSAManager::GetInstance().dhfwkOnLine_ = true;
    {
        std::lock_guard<std::mutex> lock(DHFWKSAManager::GetInstance().saStatCbMutex_);
        if (DHFWKSAManager::GetInstance().saStateCallback != nullptr) {
            DHFWKSAManager::GetInstance().saStateCallback(true);
        }
    }
    if (RestoreListener() != DH_FWK_SUCCESS) {
        DHLOGE("Partial listeners failed to restore");
    }
    DHLOGI("sa %" PRId32 " started", systemAbilityId);
}

void DHFWKSAManager::SystemAbilityListener::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    (void)deviceId;
    if (systemAbilityId != DISTRIBUTED_HARDWARE_SA_ID) {
        DHLOGW("Receive SA Stop, but sa id is not DHFWK, id: %" PRId32, systemAbilityId);
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
    DHLOGI("sa %" PRId32 " stopped", systemAbilityId);
}

int32_t DHFWKSAManager::RestoreListener()
{
    DHLOGI("Restore the failed listeners due to sa crash");
    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    int32_t ret = DH_FWK_SUCCESS;
    std::lock_guard<std::mutex> publisherListenersLock(publisherListenersMutex_);
    for (const auto &entry : publisherListenersCahce_) {
        for (const auto &listener : entry.second) {
            int32_t innerRet =
                DHFWKSAManager::GetInstance().GetDHFWKProxy()->RegisterPublisherListener(entry.first, listener);
                if (ret != DH_FWK_SUCCESS) {
                    ret = innerRet;
                    DHLOGE("Register publisher listener failed, topic: %" PRIu32, (uint32_t)entry.first);
            }
        }
    }
    std::lock_guard<std::mutex> avTransControlCenterCbLock(avTransControlCenterCbMutex_);
    for (auto &entry : avTransControlCenterCbCache_) {
        int32_t innerRet =
        DHFWKSAManager::GetInstance().GetDHFWKProxy()->RegisterCtlCenterCallback(entry.first, entry.second);
        if (innerRet != DH_FWK_SUCCESS) {
            ret = innerRet;
            DHLOGE("Restore register av control center callback failed, engineId: %" PRId32, entry.first);
        }
    }
    return ret;
}
} // DistributedHardware
} // OHOS