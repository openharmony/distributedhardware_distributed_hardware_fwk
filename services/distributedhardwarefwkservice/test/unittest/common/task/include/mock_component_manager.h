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

#ifndef OHOS_DISTRIBUTED_HARDWARE_MOCK_COMPONENT_MANAGER_H
#define OHOS_DISTRIBUTED_HARDWARE_MOCK_COMPONENT_MANAGER_H

#include <string>
#include <gmock/gmock.h>

#include "component_manager.h"
#include "component_loader.h"
namespace OHOS {
namespace DistributedHardware {
class IComponentManager {
public:
    virtual int32_t CheckDemandStart(const std::string &uuid, const DHType dhType, bool &enableSource) = 0;
    virtual int32_t ForceDisableSink(const DHDescriptor &dhDescriptor) = 0;
    virtual int32_t ForceDisableSource(const std::string &networkId, const DHDescriptor &dhDescriptor) = 0;
    virtual int32_t EnableSink(const DHDescriptor &dhDescriptor, int32_t callingUid, int32_t callingPid) = 0;
    virtual int32_t EnableSource(const std::string &networkId,
        const DHDescriptor &dhDescriptor, int32_t callingUid, int32_t callingPid) = 0;
    virtual int32_t DisableSink(const DHDescriptor &dhDescriptor, int32_t callingUid, int32_t callingPid) = 0;
    virtual int32_t DisableSource(const std::string &networkId,
        const DHDescriptor &dhDescriptor, int32_t callingUid, int32_t callingPid) = 0;
    virtual int32_t CheckSinkConfigStart(const DHType dhType, bool &enableSink) = 0;

    static std::shared_ptr<IComponentManager> GetOrCreateInstance();
    static void ReleaseInstance();
public:
    static std::shared_ptr<IComponentManager> componentManager_;
};

class MockComponentManager : public IComponentManager {
public:
    virtual ~MockComponentManager() = default;
    MOCK_METHOD(int32_t, CheckDemandStart, (const std::string&, const DHType, (bool&)));
    MOCK_METHOD(int32_t, ForceDisableSink, (const DHDescriptor&));
    MOCK_METHOD(int32_t, ForceDisableSource, (const std::string&, (const DHDescriptor&)));
    MOCK_METHOD(int32_t, EnableSink, (const DHDescriptor&, int32_t, int32_t));
    MOCK_METHOD(int32_t, EnableSource, (const std::string&, const DHDescriptor&, int32_t, int32_t));
    MOCK_METHOD(int32_t, DisableSink, (const DHDescriptor&, int32_t, int32_t));
    MOCK_METHOD(int32_t, DisableSource, (const std::string&, const DHDescriptor&, int32_t, int32_t));
    MOCK_METHOD(int32_t, CheckSinkConfigStart, (const DHType, bool&));
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_HARDWARE_MOCK_COMPONENT_MANAGER_H
 