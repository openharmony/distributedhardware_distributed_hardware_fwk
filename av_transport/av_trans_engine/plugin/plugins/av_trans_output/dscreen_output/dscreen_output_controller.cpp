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

#include "dscreen_output_controller.h"

namespace OHOS {
namespace DistributedHardware {
void DScreenOutputController::PrepareSmooth()
{
    OutputController::PrepareSmooth();
    SetBufferTime(SMOOTH_BUFFER_TIME);
    SetDynamicBalanceThre(DYNAMIC_BALANCE_THRE);
    SetAverIntervalDiffThre(AVER_INTERVAL_DIFF_THRE);
    SetPushOnceDiffThre(PUSH_ONCE_DIFF_THRE);
    SetTimeStampOnceDiffThre(TIMESTAMP_ONCE_DIFF_THRE);
    SetAdjustSleepFactor(ADJUST_SLEEP_FACTOR);
    SetWaitClockFactor(WAIT_CLOCK_FACTOR);
    SetTrackClockFactor(TRACK_CLOCK_FACTOR);
    SetSleepThre(SLEEP_THRE);
}

void DScreenOutputController::PrepareSync()
{
    OutputController::PrepareSync();
    SetAdjustSleepFactor(ADJUST_SLEEP_FACTOR);
    SetWaitClockThre(WAIT_CLOCK_THRE);
    SetTrackClockThre(TRACK_CLOCK_THRE);
    SetSleepThre(SLEEP_THRE);
}
} // namespace DistributedHardware
} // namespace OHOS