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

#include "distributed_hardware_service.h"

#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"

#include "access_manager.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
REGISTER_SYSTEM_ABILITY_BY_ID(DistributedHardwareService, DISTRIBUTED_HARDWARE_SA_ID, true);

DistributedHardwareService::DistributedHardwareService(int32_t saId, bool runOnCreate)
    : SystemAbility(saId, runOnCreate)
{
}

void DistributedHardwareService::OnStart()
{
    DHLOGI("DistributedHardwareService::OnStart start");
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        DHLOGI("DistributedHardwareService has already started.");
        return;
    }
    if (!Init()) {
        DHLOGE("failed to init DistributedHardwareService");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    DHLOGI("DistributedHardwareService::OnStart start service success.");
}

bool DistributedHardwareService::Init()
{
    DHLOGI("DistributedHardwareService::Init ready to init.");
    if (!registerToService_) {
        bool ret = Publish(this);
        if (!ret) {
            DHLOGE("DistributedHardwareService::Init Publish failed!");
            return false;
        }
        registerToService_ = true;
    }
    auto ret = AccessManager::GetInstance()->Init();
    if (ret != DH_FWK_SUCCESS) {
        DHLOGI("DistributedHardwareService::Init failed.");
        return false;
    }
    DHLOGI("DistributedHardwareService::Init init success.");
    return true;
}

void DistributedHardwareService::OnStop()
{
    DHLOGI("DistributedHardwareService::OnStop ready to stop service.");
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToService_ = false;
}

int32_t DistributedHardwareService::QuerySinkVersion(std::unordered_map<DHType, std::string> &versionMap)
{
    (void)versionMap;
    return 0;
}
}
}
