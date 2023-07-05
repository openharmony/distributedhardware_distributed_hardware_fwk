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

#ifndef OHOS_DSCREEN_OUTPUT_CONTROLLER_H
#define OHOS_DSCREEN_OUTPUT_CONTROLLER_H

#include "output_controller.h"
#include "av_trans_utils.h"
#include "av_sync_utils.h"

namespace OHOS {
namespace DistributedHardware {
class DScreenOutputController : public OutputController {
public:
    ~DScreenOutputController() override = default;
    void PrepareSmooth() override;
    void PrepareSync() override;

private:
    constexpr static float ADJUST_SLEEP_FACTOR = 0.1;
    constexpr static float WAIT_CLOCK_FACTOR = 0.1;
    constexpr static float TRACK_CLOCK_FACTOR = 0.2;
    constexpr static uint8_t DYNAMIC_BALANCE_THRE = 3;
    constexpr static int32_t SMOOTH_BUFFER_TIME = 0 * NS_ONE_MS;
    constexpr static uint32_t AVER_INTERVAL_DIFF_THRE = 2 * NS_ONE_MS;
    constexpr static uint32_t PUSH_ONCE_DIFF_THRE = 10 * NS_ONE_MS;
    constexpr static uint32_t TIMESTAMP_ONCE_DIFF_THRE = 3 * NS_ONE_MS;
    constexpr static uint32_t WAIT_CLOCK_THRE = 125 * NS_ONE_MS;
    constexpr static uint32_t TRACK_CLOCK_THRE = 45 * NS_ONE_MS;
    constexpr static int64_t SLEEP_THRE = 1000 * NS_ONE_MS;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DSCREEN_OUTPUT_CONTROLLER_H