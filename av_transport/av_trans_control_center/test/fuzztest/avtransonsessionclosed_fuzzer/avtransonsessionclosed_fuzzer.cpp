/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "avtransonsessionclosed_fuzzer.h"

#include <algorithm>
#include <fuzzer/FuzzedDataProvider.h>
#include <securec.h>
#include <thread>
#include <unistd.h>

#include "softbus_channel_adapter.h"

namespace OHOS {
namespace DistributedHardware {
void AVTransOnSessionClosedFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    int32_t reasonValue = fdp.ConsumeIntegral<int32_t>();
    ShutdownReason reason = static_cast<ShutdownReason>(reasonValue);

    SoftbusChannelAdapter::GetInstance().OnSoftbusChannelClosed(sessionId, reason);
}
} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::AVTransOnSessionClosedFuzzTest(data, size);
    return 0;
}