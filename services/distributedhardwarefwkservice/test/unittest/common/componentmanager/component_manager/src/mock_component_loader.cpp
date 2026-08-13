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

#include "mock_component_loader.h"

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<IComponentLoader> IComponentLoader::componentLoaderInstance_;

std::shared_ptr<IComponentLoader> IComponentLoader::GetOrCreateInstance()
{
    if (!componentLoaderInstance_) {
        componentLoaderInstance_ = std::make_shared<MockComponentLoader>();
    }
    return componentLoaderInstance_;
}

void IComponentLoader::ReleaseInstance()
{
    componentLoaderInstance_.reset();
    componentLoaderInstance_ = nullptr;
}

bool ComponentLoader::IsDHTypeSupport(DHType dhType)
{
    return IComponentLoader::GetOrCreateInstance()->IsDHTypeSupport(dhType);
}

int32_t ComponentLoader::GetSink(const DHType dhType, IDistributedHardwareSink *&sinkPtr)
{
    return IComponentLoader::GetOrCreateInstance()->GetSink(dhType, sinkPtr);
}

int32_t ComponentLoader::GetHardwareHandler(const DHType dhType, IHardwareHandler *&hardwareHandlerPtr)
{
    return IComponentLoader::GetOrCreateInstance()->GetHardwareHandler(dhType, hardwareHandlerPtr);
}

int32_t ComponentLoader::ReleaseSink(const DHType dhType)
{
    return IComponentLoader::GetOrCreateInstance()->ReleaseSink(dhType);
}

void GetAllCompTypes(std::vector<DHType> &dhTypeVec)
{
    IComponentLoader::GetOrCreateInstance()->GetAllCompTypes(dhTypeVec);
}

int32_t ComponentLoader::GetSource(const DHType dhType, IDistributedHardwareSource *&sourcePtr)
{
    return IComponentLoader::GetOrCreateInstance()->GetSource(dhType, sourcePtr);
}

int32_t ComponentLoader::GetSourceSaId(const DHType dhType)
{
    return IComponentLoader::GetOrCreateInstance()->GetSourceSaId(dhType);
}

int32_t ComponentLoader::ReleaseSource(const DHType dhType)
{
    return IComponentLoader::GetOrCreateInstance()->ReleaseSource(dhType);
}

int32_t ComponentLoader::GetLocalDHVersion(DHVersion &dhVersion)
{
    return IComponentLoader::GetOrCreateInstance()->GetLocalDHVersion(dhVersion);
}

std::map<std::string, bool> ComponentLoader::GetCompResourceDesc()
{
    return IComponentLoader::GetOrCreateInstance()->GetCompResourceDesc();
}
} // namespace DistributedHardware
} // namespace OHOS
