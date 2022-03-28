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

#include "test_utils_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "anonymous_string.h"
#include "dh_utils_tool.h"

namespace OHOS {
namespace DistributedHardware {
void GetAnonyStringTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    std::string str(reinterpret_cast<const char*>(data), size);
    std::string anonyStr = GetAnonyString(str);
}

void GetAnonyInt32Test(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    int32_t i = *(reinterpret_cast<const int32_t*>(data));
    std::string anonyStr = GetAnonyInt32(i);
}

void UtilsToolTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    std::string networkId(reinterpret_cast<const char*>(data), size);
    std::string uuid(reinterpret_cast<const char*>(data), size);
    std::string uuidStr = GetUUIDBySoftBus(networkId);
    std::string deviceIdStr = GetDeviceIdByUUID(uuid);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::GetAnonyStringTest(data, size);
    OHOS::DistributedHardware::GetAnonyInt32Test(data, size);
    OHOS::DistributedHardware::UtilsToolTest(data, size);
    return 0;
}

