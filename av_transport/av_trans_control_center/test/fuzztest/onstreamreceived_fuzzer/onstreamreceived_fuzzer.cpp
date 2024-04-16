/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "onstreamreceived_fuzzer.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <thread>
#include <unistd.h>

#include "softbus_channel_adapter.h"

namespace OHOS {
namespace DistributedHardware {

void OnStreamReceivedFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    int32_t sessionId = *(reinterpret_cast<const int32_t*>(data));
    const StreamData *streamData = reinterpret_cast<const StreamData*>(data);
    const StreamData *extData = reinterpret_cast<const StreamData*>(data);
    const StreamFrameInfo *frameInfo = reinterpret_cast<const StreamFrameInfo*>(data);

    SoftbusChannelAdapter::GetInstance().OnSoftbusStreamReceived(sessionId, streamData, extData, frameInfo);
}
} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::OnStreamReceivedFuzzTest(data, size);
    return 0;
}