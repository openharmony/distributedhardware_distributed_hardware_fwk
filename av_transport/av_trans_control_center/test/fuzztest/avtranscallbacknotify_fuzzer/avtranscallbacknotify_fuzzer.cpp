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

#include "avtranscallbacknotify_fuzzer.h"

#include "av_trans_control_center_callback.h"
#include "av_sync_utils.h"

namespace OHOS {
namespace DistributedHardware {
const uint32_t DC_EVENTTYPE_SIZE = 13;
void AVTransCallbackNotifyFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    EventType type = static_cast<EventType>(*(reinterpret_cast<const uint32_t*>(data)) % DC_EVENTTYPE_SIZE);
    std::string content(reinterpret_cast<const char*>(data), size);
    std::string peerDevId(reinterpret_cast<const char*>(data), size);
    AVTransEvent event = AVTransEvent{ type, content, peerDevId };
    sptr<AVTransControlCenterCallback> controlCenterCallback(new (std::nothrow) AVTransControlCenterCallback());
    controlCenterCallback->Notify(event);
}
} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::AVTransCallbackNotifyFuzzTest(data, size);
    return 0;
}