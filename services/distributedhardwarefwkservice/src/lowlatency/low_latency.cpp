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

#include "low_latency.h"

#include <cinttypes>

#include "res_sched_client.h"
#include "res_type.h"

#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(LowLatency);

namespace {
    constexpr int32_t MODE_ENABLE = 0;
    constexpr int32_t MODE_DISABLE = 1;
    constexpr uint32_t MAX_SWITCH_SIZE = 256;
    const std::string KEY = "identity";
    const std::string PKG_NAME = "ohos.DistributedHardware";
}

void LowLatency::EnableLowLatency(DHType dhType)
{
    DHLOGI("Start EnableLowLatency dhType: %#X", dhType);
    if (dhType <= DHType::UNKNOWN || dhType >= DHType::MAX_DH) {
        DHLOGE("DHType is invalid, dhType: %" PRIu32, (uint32_t)dhType);
        return;
    }
    std::lock_guard<std::mutex> lock(lowLatencyMutex_);
    DHLOGI("lowLatencySwitchSet size: %d", lowLatencySwitchSet_.size());
    if (lowLatencySwitchSet_.empty()) {
        DHLOGD("Open LowLatency dhType: %#X", dhType);
        auto &rssClient = OHOS::ResourceSchedule::ResSchedClient::GetInstance();
        // to enable low latency mode: value = 0
        rssClient.ReportData(OHOS::ResourceSchedule::ResType::RES_TYPE_NETWORK_LATENCY_REQUEST, MODE_ENABLE,
            {{KEY, PKG_NAME}});
    }
    if (lowLatencySwitchSet_.size() >= MAX_SWITCH_SIZE) {
        DHLOGE("lowLatencySwitchSet_ is oversize");
        return;
    }
    lowLatencySwitchSet_.insert(dhType);
    DHLOGI("End EnableLowLatency dhType: %#X", dhType);
}

void LowLatency::DisableLowLatency(DHType dhType)
{
    DHLOGI("Start DisableLowLatency dhType: %#X", dhType);
    if (dhType <= DHType::UNKNOWN || dhType >= DHType::MAX_DH) {
        DHLOGE("DHType is invalid, dhType: %" PRIu32, (uint32_t)dhType);
        return;
    }
    std::lock_guard<std::mutex> lock(lowLatencyMutex_);
    lowLatencySwitchSet_.erase(dhType);
    if (lowLatencySwitchSet_.empty()) {
        DHLOGD("Close LowLatency dhType: %#X", dhType);

        auto &rssClient = OHOS::ResourceSchedule::ResSchedClient::GetInstance();
        // to restore normal latency mode: value = 1
        rssClient.ReportData(OHOS::ResourceSchedule::ResType::RES_TYPE_NETWORK_LATENCY_REQUEST, MODE_DISABLE,
            {{KEY, PKG_NAME}});
    }
    DHLOGI("End DisableLowLatency dhType: %#X", dhType);
}

void LowLatency::CloseLowLatency()
{
    DHLOGI("Shutdown LowLatency");
    std::lock_guard<std::mutex> lock(lowLatencyMutex_);
    lowLatencySwitchSet_.clear();
    auto &rssClient = OHOS::ResourceSchedule::ResSchedClient::GetInstance();
    // to restore normal latency mode: value = 1
    rssClient.ReportData(OHOS::ResourceSchedule::ResType::RES_TYPE_NETWORK_LATENCY_REQUEST, MODE_DISABLE,
        {{KEY, PKG_NAME}});
}
} // namespace DistributedHardware
} // namespace OHOS