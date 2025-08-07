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

#ifndef OHOS_DISTRIBUTED_HARDWARE_MOCK_COMPONENT_LOADER_H
#define OHOS_DISTRIBUTED_HARDWARE_MOCK_COMPONENT_LOADER_H

#include <gmock/gmock.h>

#include "component_loader.h"

namespace OHOS {
namespace DistributedHardware {
class IComponentLoader {
public:
    virtual ~IComponentLoader() = default;

    virtual bool IsDHTypeSupport(DHType dhType) = 0;
    virtual int32_t GetSink(const DHType dhType, IDistributedHardwareSink *&sinkPtr) = 0;
    virtual int32_t GetHardwareHandler(const DHType dhType, IHardwareHandler *&hardwareHandlerPtr) = 0;
    virtual int32_t ReleaseSink(const DHType dhType) = 0;
    virtual void GetAllCompTypes(std::vector<DHType> &dhTypeVec) = 0;
    virtual int32_t GetSource(const DHType dhType, IDistributedHardwareSource *&sourcePtr) = 0;
    virtual int32_t GetSourceSaId(const DHType dhType) = 0;
    virtual int32_t ReleaseSource(const DHType dhType) = 0;
    virtual int32_t GetLocalDHVersion(DHVersion &dhVersion) = 0;
    virtual std::map<std::string, bool> GetCompResourceDesc() = 0;

    static std::shared_ptr<IComponentLoader> GetOrCreateInstance();
    static void ReleaseInstance();
private:
    static std::shared_ptr<IComponentLoader> componentLoaderInstance_;
};

class MockComponentLoader : public IComponentLoader {
public:
    MOCK_METHOD(bool, IsDHTypeSupport, (DHType));
    MOCK_METHOD(int32_t, GetSink, (const DHType, IDistributedHardwareSink *&));
    MOCK_METHOD(int32_t, GetHardwareHandler, (const DHType, IHardwareHandler *&));
    MOCK_METHOD(int32_t, ReleaseSink, (const DHType));
    MOCK_METHOD(void, GetAllCompTypes, (std::vector<DHType> &));
    MOCK_METHOD(int32_t, GetSource, (const DHType, IDistributedHardwareSource *&));
    MOCK_METHOD(int32_t, GetSourceSaId, (const DHType));
    MOCK_METHOD(int32_t, ReleaseSource, (const DHType));
    MOCK_METHOD(int32_t, GetLocalDHVersion, (DHVersion &));
    MOCK_METHOD((std::map<std::string, bool>), GetCompResourceDesc, ());
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_HARDWARE_MOCK_COMPONENT_LOADER_H
