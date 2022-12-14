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

#include "low_latency_timer.h"

#include "res_sched_client.h"
#include "res_type.h"

#include "constants.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "LowLatencyTimer"

LowLatencyTimer::LowLatencyTimer(std::string timerId, int32_t delayTimeMs) : DHTimer(timerId, delayTimeMs)
{
    DHLOGI("LowLatencyTimer ctor!");
}

LowLatencyTimer::~LowLatencyTimer()
{
    DHLOGI("LowLatencyTimer dtor!");
}

void LowLatencyTimer::ExecuteInner()
{
    DHLOGD("ExecuteInner");
    auto &rssClient = OHOS::ResourceSchedule::ResSchedClient::GetInstance();
    // to enable low latency mode: value = 0
    rssClient.ReportData(OHOS::ResourceSchedule::ResType::RES_TYPE_NETWORK_LATENCY_REQUEST, MODE_ENABLE,
        {{LOW_LATENCY_KEY, DH_FWK_PKG_NAME}});
}

void LowLatencyTimer::HandleStopTimer()
{
    DHLOGI("HandleStopTimer!");
    auto &rssClient = OHOS::ResourceSchedule::ResSchedClient::GetInstance();
    // to restore normal latency mode: value = 1
    rssClient.ReportData(OHOS::ResourceSchedule::ResType::RES_TYPE_NETWORK_LATENCY_REQUEST, MODE_DISABLE,
        {{LOW_LATENCY_KEY, DH_FWK_PKG_NAME}});
}
}
}
