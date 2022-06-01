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
#include "dh_utils_hitrace.h"

#include <unistd.h>
#include <unordered_map>

namespace OHOS {
namespace DistributedHardware {
void CompEnableTraceStart(const DHType dhType)
{
    std::string dhTypeStr = "UNKNOWN";
    auto it = DHTypeStrMap.find(dhType);
    if (it != DHTypeStrMap.end()) {
        dhTypeStr = it->second;
    }
    StartTrace(DHFWK_HITRACE_LABEL, dhTypeStr + "_ENABLE_START");
}

void CompDisableTraceStart(const DHType dhType)
{
    std::string dhTypeStr = "UNKNOWN";
    auto it = DHTypeStrMap.find(dhType);
    if (it != DHTypeStrMap.end()) {
        dhTypeStr = it->second;
    }
    StartTrace(DHFWK_HITRACE_LABEL, dhTypeStr + "_DISABLE_START");
}

void DHTraceStart(const std::string &msg)
{
    StartTrace(DHFWK_HITRACE_LABEL, msg);
}

void TraceEnd()
{
    FinishTrace(DHFWK_HITRACE_LABEL);
}
} // namespace DistributedHardware
} // namespace OHOS