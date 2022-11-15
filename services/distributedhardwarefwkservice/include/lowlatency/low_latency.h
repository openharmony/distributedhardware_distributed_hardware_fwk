/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_LOW_LATENCY_H
#define OHOS_DISTRIBUTED_HARDWARE_LOW_LATENCY_H

#include <mutex>
#include <unordered_set>

#include "device_type.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class LowLatency {
DECLARE_SINGLE_INSTANCE_BASE(LowLatency);
public:
    void EnableLowLatency(DHType dhType);
    void DisableLowLatency(DHType dhType);
    void CloseLowLatency();

private:
    LowLatency() = default;
    ~LowLatency() = default;

private:
    std::unordered_set<DHType> lowLatencySwitchSet_;
    std::mutex lowLatencyMutex_;
};
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_DISTRIBUTED_HARDWARE_LOW_LATENCY_H
