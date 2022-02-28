/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_SERVICE_H
#define OHOS_DISTRIBUTED_HARDWARE_SERVICE_H

#include "system_ability.h"
#include "ipc_object_stub.h"

#include "distributed_hardware_stub.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
enum class ServiceRunningState {
    STATE_NOT_START,
    STATE_RUNNING
};

class DistributedHardwareService : public SystemAbility, public DistributedHardwareStub {
DECLARE_SYSTEM_ABILITY(DistributedHardwareService);
public:
    DistributedHardwareService(int32_t saId, bool runOnCreate);
    ~DistributedHardwareService() = default;
    int32_t QuerySinkVersion(std::unordered_map<DHType, std::string> &versionMap) override;

protected:
    void OnStart() override;
    void OnStop() override;

private:
    bool Init();

private:
    bool registerToService_ = false;
    ServiceRunningState state_ = ServiceRunningState::STATE_NOT_START;
};
}
}
#endif