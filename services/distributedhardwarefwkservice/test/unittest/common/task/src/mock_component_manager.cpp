/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "mock_component_manager.h"

#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<IComponentManager> IComponentManager::componentManager_;

std::shared_ptr<IComponentManager> IComponentManager::GetOrCreateInstance()
{
    if (!componentManager_) {
        componentManager_ = std::make_shared<MockComponentManager>();
    }

    return componentManager_;
}

void IComponentManager::ReleaseInstance()
{
    componentManager_.reset();
    componentManager_ = nullptr;
}

int32_t ComponentManager::CheckDemandStart(const std::string &uuid, const DHType dhType, bool &enableSource)
{
    enableSource = true;
    return IComponentManager::GetOrCreateInstance()->CheckDemandStart(uuid, dhType, enableSource);
}

int32_t ComponentManager::ForceDisableSink(const DHDescriptor &dhDescriptor)
{
    return IComponentManager::GetOrCreateInstance()->ForceDisableSink(dhDescriptor);
}

int32_t ComponentManager::ForceDisableSource(const std::string &networkId, const DHDescriptor &dhDescriptor)
{
    return IComponentManager::GetOrCreateInstance()->ForceDisableSource(networkId, dhDescriptor);
}

int32_t ComponentManager::EnableSink(const DHDescriptor &dhDescriptor, int32_t callingUid, int32_t callingPid)
{
    return IComponentManager::GetOrCreateInstance()->EnableSink(dhDescriptor, callingUid, callingPid);
}

int32_t ComponentManager::EnableSource(const std::string &networkId,
    const DHDescriptor &dhDescriptor, int32_t callingUid, int32_t callingPid)
{
    return IComponentManager::GetOrCreateInstance()->EnableSource(networkId, dhDescriptor, callingUid, callingPid);
}

int32_t ComponentManager::DisableSink(const DHDescriptor &dhDescriptor, int32_t callingUid, int32_t callingPid)
{
    return IComponentManager::GetOrCreateInstance()->DisableSink(dhDescriptor, callingUid, callingPid);
}

int32_t ComponentManager::DisableSource(const std::string &networkId,
    const DHDescriptor &dhDescriptor, int32_t callingUid, int32_t callingPid)
{
    return IComponentManager::GetOrCreateInstance()->DisableSource(networkId, dhDescriptor, callingUid, callingPid);
}

int32_t ComponentManager::CheckSinkConfigStart(const DHType dhType, bool &enableSink)
{
    return IComponentManager::GetOrCreateInstance()->CheckSinkConfigStart(dhType, enableSink);
}
} // namespace DistributedHardware
} // namespace OHOS
