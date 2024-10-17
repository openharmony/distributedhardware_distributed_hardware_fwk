/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "dh_state_listener.h"

#include <cinttypes>

#include "anonymous_string.h"
#include "component_manager.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_log.h"

class ComponentManager;
namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "DHStateListener"
DHStateListener::DHStateListener()
{
    DHLOGI("Ctor DHStateListener");
}

DHStateListener::~DHStateListener()
{
    DHLOGI("Dtor DHStateListener");
}

void DHStateListener::OnStateChanged(const std::string &networkId, const std::string &dhId, const BusinessState state)
{
    if (!IsIdLengthValid(networkId)) {
        return;
    }
    DHLOGI("Receive business state change, networkId: %{public}s, dhId: %{public}s, state: %{public}" PRIu32,
        GetAnonyString(networkId).c_str(), GetAnonyString(dhId).c_str(), (uint32_t)state);
    ComponentManager::GetInstance().UpdateBusinessState(networkId, dhId, state);
}
} // namespace DistributedHardware
} // namespace OHOS