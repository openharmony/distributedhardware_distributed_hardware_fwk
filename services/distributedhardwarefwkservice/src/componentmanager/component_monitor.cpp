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

#include "component_monitor.h"

#include <cinttypes>

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "anonymous_string.h"
#include "component_loader.h"
#include "component_manager.h"
#include "device_type.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
ComponentMonitor::ComponentMonitor() : saListeners_({})
{
    DHLOGI("Ctor ComponentMonitor");
}

ComponentMonitor::~ComponentMonitor()
{
    DHLOGI("Dtor ComponentMonitor");
    std::lock_guard<std::mutex> lock(saListenersMtx_);
    saListeners_.clear();
}

void ComponentMonitor::CompSystemAbilityListener::OnAddSystemAbility(int32_t saId, const std::string &deviceId)
{
    DHLOGI("OnAddSystemAbility, saId: %d, deviceId: %s", saId, GetAnonyString(deviceId).c_str());
}

void ComponentMonitor::CompSystemAbilityListener::OnRemoveSystemAbility(int32_t saId, const std::string &deviceId)
{
    DHLOGI("OnRemoveSystemAbility, saId: %d, deviceId: %s", saId, GetAnonyString(deviceId).c_str());
    DHType dhType = ComponentLoader::GetInstance().GetDHTypeBySrcSaId(saId);
    if (dhType == DHType::UNKNOWN) {
        DHLOGE("Can not find DHType by sa Id: %d", saId);
        return;
    }

    DHLOGI("Try Recover Component, dhType: %" PRIu32, (uint32_t)dhType);
    ComponentManager::GetInstance().Recover(dhType);
}

void ComponentMonitor::AddSAMonitor(int32_t saId)
{
    DHLOGI("Try add sa monitor, saId: %" PRIu32, saId);
    std::lock_guard<std::mutex> lock(saListenersMtx_);
    if (saListeners_.find(saId) != saListeners_.end()) {
        DHLOGW("SaId is in monitor, id: %" PRIu32, saId);
        return;
    }

    sptr<CompSystemAbilityListener> listener = new CompSystemAbilityListener();
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        DHLOGE("get system ability manager failed.");
        return;
    }

    int32_t ret = systemAbilityManager->SubscribeSystemAbility(saId, listener);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("subscribe sa change listener failed: %d", ret);
        return;
    }

    saListeners_[saId] = listener;
    DHLOGI("subscribe sa change listener success.");
    return;
}

void ComponentMonitor::RemoveSAMonitor(int32_t saId)
{
    DHLOGI("Try remove sa monitor, saId: %" PRIu32, saId);
    std::lock_guard<std::mutex> lock(saListenersMtx_);
    if (saListeners_.find(saId) == saListeners_.end()) {
        DHLOGW("can not find sa listener info, id: %" PRIu32, saId);
        return;
    }

    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        DHLOGE("get system ability manager failed.");
        return;
    }

    int32_t ret = systemAbilityManager->UnSubscribeSystemAbility(saId, saListeners_[saId]);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("unsubscribe sa change listener failed: %d", ret);
        return;
    }

    saListeners_.erase(saId);
    DHLOGI("unsubscribe sa change listener success");
    return;
}
} // namespace DistributedHardware
} // namespace OHOS