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

#include <unordered_map>

namespace OHOS {
namespace DistributedHardware {
void DHCompMgrTraceStart(const std::string &anonyNetworkId, const std::string &anonyDHId, const std::string &msg)
{
    StartTrace(DHFWK_HITRACE_LABEL, anonyNetworkId + "_" + anonyDHId + "_" + msg);
}

void DHQueryTraceStart(const DHType dhType)
{
    std::string dhTypeStr = "UNKNOWN";
    auto it = DHTypeStrMap.find(dhType);
    if (it != DHTypeStrMap.end()) {
        dhTypeStr = it->second;
    }
    StartTrace(DHFWK_HITRACE_LABEL, dhTypeStr + "_" + DH_QUERY_START);
}

void DHTraceStart(const std::string &msg)
{
    StartTrace(DHFWK_HITRACE_LABEL, msg);
}

void DHTraceEnd()
{
    FinishTrace(DHFWK_HITRACE_LABEL);
}
} // namespace DistributedHardware
} // namespace OHOS