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

#ifndef OHOS_DISTRIBUTED_HARDWARE_COMPONENT_MONITOR_H
#define OHOS_DISTRIBUTED_HARDWARE_COMPONENT_MONITOR_H

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <refbase.h>

#include "system_ability_status_change_stub.h"

#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class ComponentMonitor {
REMOVE_NO_USE_CONSTRUCTOR(ComponentMonitor);
public:
    explicit ComponentMonitor();
    ~ComponentMonitor();
    void AddSAMonitor(int32_t saId);
    void RemoveSAMonitor(int32_t saId);

public:
    class CompSystemAbilityListener : public SystemAbilityStatusChangeStub {
    public:
        void OnAddSystemAbility(int32_t saId, const std::string &deviceId) override;
        void OnRemoveSystemAbility(int32_t saId, const std::string &deviceId) override;
    };

private:
    std::mutex saListenersMtx_;
    std::map<int32_t, sptr<SystemAbilityStatusChangeStub>> saListeners_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif