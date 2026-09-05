/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "componentmanagersink_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include "component_manager.h"
#include "constants.h"
#include "dhardware_descriptor.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "idistributed_hardware_sink.h"

namespace OHOS {
namespace DistributedHardware {
void EnableSinkWithCustomParamsFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t) + sizeof(int32_t) + sizeof(uint32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t callingUid = fdp.ConsumeIntegral<int32_t>();
    int32_t callingPid = fdp.ConsumeIntegral<int32_t>();
    DHType dhType = static_cast<DHType>(fdp.ConsumeIntegral<uint32_t>());
    std::string dhId = fdp.ConsumeRandomLengthString();
    std::string customParams = fdp.ConsumeRandomLengthString();
    DHDescriptor dhDescriptor {
        .id = dhId,
        .dhType = dhType,
        .customParams = customParams
    };
    ComponentManager::GetInstance().Init();
    ComponentManager::GetInstance().EnableSink(dhDescriptor, callingUid, callingPid);
    ComponentManager::GetInstance().DisableSink(dhDescriptor, callingUid, callingPid);
    ComponentManager::GetInstance().UnInit();
}

void EnableSourceWithCustomParamsFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t) + sizeof(int32_t) + sizeof(uint32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t callingUid = fdp.ConsumeIntegral<int32_t>();
    int32_t callingPid = fdp.ConsumeIntegral<int32_t>();
    DHType dhType = static_cast<DHType>(fdp.ConsumeIntegral<uint32_t>());
    std::string networkId = fdp.ConsumeRandomLengthString();
    std::string dhId = fdp.ConsumeRandomLengthString();
    std::string customParams = fdp.ConsumeRandomLengthString();
    DHDescriptor dhDescriptor {
        .id = dhId,
        .dhType = dhType,
        .customParams = customParams
    };
    ComponentManager::GetInstance().Init();
    ComponentManager::GetInstance().EnableSource(networkId, dhDescriptor, callingUid, callingPid);
    ComponentManager::GetInstance().DisableSource(networkId, dhDescriptor, callingUid, callingPid);
    ComponentManager::GetInstance().UnInit();
}

void ForceDisableWithCustomParamsFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    DHType dhType = static_cast<DHType>(fdp.ConsumeIntegral<uint32_t>());
    std::string networkId = fdp.ConsumeRandomLengthString();
    std::string dhId = fdp.ConsumeRandomLengthString();
    std::string customParams = fdp.ConsumeRandomLengthString();
    DHDescriptor dhDescriptor {
        .id = dhId,
        .dhType = dhType,
        .customParams = customParams
    };
    ComponentManager::GetInstance().Init();
    ComponentManager::GetInstance().ForceDisableSink(dhDescriptor);
    ComponentManager::GetInstance().ForceDisableSource(networkId, dhDescriptor);
    ComponentManager::GetInstance().UnInit();
}

void EnableSinkWithTokenIdFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t) + sizeof(int32_t) + sizeof(uint32_t) + sizeof(uint32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t callingUid = fdp.ConsumeIntegral<int32_t>();
    int32_t callingPid = fdp.ConsumeIntegral<int32_t>();
    DHType dhType = static_cast<DHType>(fdp.ConsumeIntegral<uint32_t>());
    uint32_t firstCallingTokenId = fdp.ConsumeIntegral<uint32_t>();
    std::string dhId = fdp.ConsumeRandomLengthString();
    DHDescriptor dhDescriptor {
        .id = dhId,
        .dhType = dhType,
        .firstCallingTokenId = firstCallingTokenId
    };
    ComponentManager::GetInstance().Init();
    ComponentManager::GetInstance().EnableSink(dhDescriptor, callingUid, callingPid);
    ComponentManager::GetInstance().DisableSink(dhDescriptor, callingUid, callingPid);
    ComponentManager::GetInstance().UnInit();
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::EnableSinkWithCustomParamsFuzzTest(data, size);
    OHOS::DistributedHardware::EnableSourceWithCustomParamsFuzzTest(data, size);
    OHOS::DistributedHardware::ForceDisableWithCustomParamsFuzzTest(data, size);
    OHOS::DistributedHardware::EnableSinkWithTokenIdFuzzTest(data, size);
    return 0;
}
