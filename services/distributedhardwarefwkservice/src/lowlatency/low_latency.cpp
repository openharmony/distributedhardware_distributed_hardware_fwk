/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "low_latency.h"

#include <cinttypes>

#include "res_sched_client.h"
#include "res_type.h"

#include "constants.h"
#include "distributed_hardware_log.h"
#include "low_latency_timer.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "LowLatency"
IMPLEMENT_SINGLE_INSTANCE(LowLatency);
namespace {
    const std::string LOW_LATENCY_TIMER_ID = "low_latency_timer_id";
    constexpr int32_t LOW_LATENCY_DELAY_MS = 50 * 1000;
}

LowLatency::LowLatency() : lowLatencyTimer_(std::make_shared<LowLatencyTimer>(LOW_LATENCY_TIMER_ID,
    LOW_LATENCY_DELAY_MS))
{
    DHLOGI("LowLatency ctor!");
}

LowLatency::~LowLatency()
{
    DHLOGI("LowLatency dtor!");
}

void LowLatency::EnableLowLatency(DHType dhType)
{
    DHLOGI("Start EnableLowLatency dhType: %{public}#X", dhType);
    if (dhType <= DHType::UNKNOWN || dhType >= DHType::MAX_DH) {
        DHLOGE("DHType is invalid, dhType: %{public}" PRIu32, (uint32_t)dhType);
        return;
    }
    std::lock_guard<std::mutex> lock(lowLatencyMutex_);
    DHLOGI("lowLatencySwitchSet size: %{public}zu", lowLatencySwitchSet_.size());
    if (lowLatencySwitchSet_.empty() && lowLatencyTimer_ != nullptr) {
        DHLOGD("Open LowLatency dhType: %{public}#X", dhType);
        lowLatencyTimer_->StartTimer();
    }
    if (lowLatencySwitchSet_.size() >= MAX_SWITCH_SIZE) {
        DHLOGE("lowLatencySwitchSet_ is oversize");
        return;
    }
    lowLatencySwitchSet_.insert(dhType);
    DHLOGI("End EnableLowLatency dhType: %{public}#X", dhType);
}

void LowLatency::DisableLowLatency(DHType dhType)
{
    DHLOGI("Start DisableLowLatency dhType: %{public}#X", dhType);
    if (dhType <= DHType::UNKNOWN || dhType >= DHType::MAX_DH) {
        DHLOGE("DHType is invalid, dhType: %{public}" PRIu32, (uint32_t)dhType);
        return;
    }
    std::lock_guard<std::mutex> lock(lowLatencyMutex_);
    lowLatencySwitchSet_.erase(dhType);
    if (lowLatencySwitchSet_.empty() && lowLatencyTimer_ != nullptr) {
        DHLOGD("Close LowLatency dhType: %{public}#X", dhType);
        lowLatencyTimer_->StopTimer();
    }
    DHLOGI("End DisableLowLatency dhType: %{public}#X", dhType);
}

void LowLatency::CloseLowLatency()
{
    DHLOGI("Shutdown LowLatency");
    std::lock_guard<std::mutex> lock(lowLatencyMutex_);
    lowLatencySwitchSet_.clear();
    // to restore normal latency mode: value = 1
    OHOS::ResourceSchedule::ResSchedClient::GetInstance().ReportData(
        OHOS::ResourceSchedule::ResType::RES_TYPE_NETWORK_LATENCY_REQUEST, MODE_DISABLE,
        {{LOW_LATENCY_KEY, DH_FWK_PKG_NAME}});
}
} // namespace DistributedHardware
} // namespace OHOS