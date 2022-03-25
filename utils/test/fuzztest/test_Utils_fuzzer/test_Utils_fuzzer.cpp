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

#include "test_Utils_fuzzer.h"

#include <stddef.h>
#include <stdint.h>

#include "anonymous_string.h"
#include "dh_utils_tool.h"


namespace OHOS {
namespace DistributedHardware {
constexpr uint16_t STR_LEN = 32;

bool UtilsFuzzerTet(const uint8_t* data, size_t size)
{
    if (size > STR_LEN) {
        std::string str1(reinterpret_cast<const char*>(data), STR_LEN);
        std::string anonyStr1 = GetAnonyString(str1);

        std::string str2(reinterpret_cast<const char*>(data), (*data) % STR_LEN);
        std::string anonyStr2 = GetAnonyString(str2);

        int32_t i = *(reinterpret_cast<const int32_t*>(data));
        std::string anonyStri = GetAnonyInt32(i);
        return true;
    }
    return false;
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::UtilsFuzzerTet(data, size);
    return 0;
}

