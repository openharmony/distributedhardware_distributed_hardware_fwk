/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "componentdisable_fuzzer.h"

#include "component_disable.h"
#include "distributed_hardware_errno.h"

namespace OHOS {
namespace DistributedHardware {

void ComponentDisableFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    auto compDisable = std::make_shared<ComponentDisable>();
    std::string uuid(reinterpret_cast<const char*>(data), size);
    std::string dhId(reinterpret_cast<const char*>(data), size);
    int32_t status = DH_FWK_SUCCESS;
    std::string disableData(reinterpret_cast<const char*>(data), size);
    compDisable->OnUnregisterResult(uuid, dhId, status, disableData);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::ComponentDisableFuzzTest(data, size);
    return 0;
}

