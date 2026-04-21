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

#include "distributedhardwarefwkkitsix_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>

#include "distributed_hardware_fwk_kit.h"

namespace OHOS {
namespace DistributedHardware {
void TestAuthorizationResultCallback::OnAuthorizationResult(const std::string &networkId, const std::string &requestId)
{
    (void)networkId;
    (void)requestId;
}

void RegisterHardwareAccessListenerFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    DistributedHardwareFwkKit dhfwkKit;
    DHType dhType = static_cast<DHType>(*(reinterpret_cast<const uint32_t*>(data)));
    sptr<IAuthorizationResultCallback> callback(new TestAuthorizationResultCallback());
    int32_t timeOut = *(reinterpret_cast<const int32_t*>(data));
    std::string pkgName(reinterpret_cast<const char*>(data), size);
    dhfwkKit.RegisterHardwareAccessListener(dhType, callback, timeOut, pkgName);
}

void UnregisterHardwareAccessListenerFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    DistributedHardwareFwkKit dhfwkKit;
    DHType dhType = static_cast<DHType>(*(reinterpret_cast<const uint32_t*>(data)));
    sptr<IAuthorizationResultCallback> callback(new TestAuthorizationResultCallback());
    std::string pkgName(reinterpret_cast<const char*>(data), size);
    dhfwkKit.UnregisterHardwareAccessListener(dhType, callback, pkgName);
}

void SetAuthorizationResultFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    DistributedHardwareFwkKit dhfwkKit;
    DHType dhType = static_cast<DHType>(*(reinterpret_cast<const uint32_t*>(data)));
    std::string requestId(reinterpret_cast<const char*>(data), size);
    bool granted = false;
    dhfwkKit.SetAuthorizationResult(dhType, requestId, granted);
}
} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::RegisterHardwareAccessListenerFuzzTest(data, size);
    OHOS::DistributedHardware::UnregisterHardwareAccessListenerFuzzTest(data, size);
    OHOS::DistributedHardware::SetAuthorizationResultFuzzTest(data, size);
    return 0;
}
