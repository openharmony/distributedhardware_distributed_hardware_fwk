/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "distributedhardwarefwkkitfour_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>

#include "distributed_hardware_fwk_kit.h"

namespace OHOS {
namespace DistributedHardware {
void DisableSinkFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    DistributedHardwareFwkKit dhfwkKit;
    std::vector<DHDescriptor> descriptors;
    std::string dhId(reinterpret_cast<const char*>(data), size);
    DHType dhType = DHType::AUDIO;
    DHDescriptor dhDescriptor {
        .id = dhId,
        .dhType = dhType
    };
    descriptors.push_back(dhDescriptor);
    dhfwkKit.DisableSink(descriptors);
}

void LoadDistributedHDFFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    DHType dhType = static_cast<DHType>(*(reinterpret_cast<const uint32_t*>(data)));

    DistributedHardwareFwkKit dhfwkKit;
    dhfwkKit.LoadDistributedHDF(dhType);
}

void UnLoadDistributedHDFFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    DHType dhType = static_cast<DHType>(*(reinterpret_cast<const uint32_t*>(data)));

    DistributedHardwareFwkKit dhfwkKit;
    dhfwkKit.UnLoadDistributedHDF(dhType);
}

void QueryLocalSysSpecFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    DistributedHardwareFwkKit dhfwkKit;
    enum QueryLocalSysSpecType specType =
            static_cast<QueryLocalSysSpecType>(*(reinterpret_cast<const uint32_t*>(data)));
    dhfwkKit.QueryLocalSysSpec(specType);
}
} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DisableSinkFuzzTest(data, size);
    OHOS::DistributedHardware::LoadDistributedHDFFuzzTest(data, size);
    OHOS::DistributedHardware::UnLoadDistributedHDFFuzzTest(data, size);
    OHOS::DistributedHardware::QueryLocalSysSpecFuzzTest(data, size);
    return 0;
}
