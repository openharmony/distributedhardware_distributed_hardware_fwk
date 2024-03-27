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
#include "dh_utils_hisysevent.h"

#include <unordered_map>

#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
void HiSysEventWriteMsg(const std::string &status, const OHOS::HiviewDFX::HiSysEvent::EventType eventType,
    const std::string &msg)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_HARDWARE_FWK,
        status.c_str(),
        eventType,
        "MSG", msg.c_str());
    if (res != DH_FWK_SUCCESS) {
        DHLOGE("Write HiSysEvent error, res:%{public}d", res);
    }
}

void HiSysEventWriteErrCodeMsg(const std::string &status, const OHOS::HiviewDFX::HiSysEvent::EventType eventType,
    int32_t errCode, const std::string &msg)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_HARDWARE_FWK,
        status.c_str(),
        eventType,
        "ERR_CODE", errCode,
        "ERR_MSG", msg.c_str());
    if (res != DH_FWK_SUCCESS) {
        DHLOGE("Write HiSysEvent error, res:%{public}d", res);
    }
}

void HiSysEventWriteReleaseMsg(const std::string &status, const OHOS::HiviewDFX::HiSysEvent::EventType eventType,
    const DHType dhType, int32_t errCode, const std::string &msg)
{
    std::string dhTypeStr = "UNKNOWN";
    auto it = DHTypeStrMap.find(dhType);
    if (it != DHTypeStrMap.end()) {
        dhTypeStr = it->second;
    }

    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_HARDWARE_FWK,
        status.c_str(),
        eventType,
        "DHTYPE", dhTypeStr.c_str(),
        "ERR_CODE", errCode,
        "ERR_MSG", msg.c_str());
    if (res != DH_FWK_SUCCESS) {
        DHLOGE("Write HiSysEvent error, res:%{public}d", res);
    }
}

void HiSysEventWriteCompOfflineMsg(const std::string &status, const OHOS::HiviewDFX::HiSysEvent::EventType eventType,
    const std::string &anonyNetworkId, const std::string &msg)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_HARDWARE_FWK,
        status.c_str(),
        eventType,
        "NETWORKID", anonyNetworkId.c_str(),
        "MSG", msg.c_str());
    if (res != DH_FWK_SUCCESS) {
        DHLOGE("Write HiSysEvent error, res:%{public}d", res);
    }
}

void HiSysEventWriteCompMgrFailedMsg(const std::string &status, const OHOS::HiviewDFX::HiSysEvent::EventType eventType,
    const std::string &anonyDHId, int32_t errCode, const std::string &msg)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_HARDWARE_FWK,
        status.c_str(),
        eventType,
        "DHID", anonyDHId.c_str(),
        "ERR_CODE", errCode,
        "ERR_MSG", msg.c_str());
    if (res != DH_FWK_SUCCESS) {
        DHLOGE("Write HiSysEvent error, res:%{public}d", res);
    }
}
} // namespace DistributedHardware
} // namespace OHOS