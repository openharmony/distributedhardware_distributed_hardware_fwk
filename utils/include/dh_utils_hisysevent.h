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

#ifndef OHOS_DISTRIBUTED_HARDWARE_DHUTIL_HISYSEVENT_H
#define OHOS_DISTRIBUTED_HARDWARE_DHUTIL_HISYSEVENT_H

#include <cstdint>
#include <string>

#include "hisysevent.h"

#include "device_type.h"

namespace OHOS {
namespace DistributedHardware {
const std::string DHFWK_INIT_BEGIN = "DHFWK_INIT_BEGIN";
const std::string DHFWK_INIT_END = "DHFWK_INIT_END";
const std::string DHFWK_INIT_FAIL = "DHFWK_INIT_FAIL";
const std::string DHFWK_EXIT_BEGIN = "DHFWK_EXIT_BEGIN";
const std::string DHFWK_EXIT_END = "DHFWK_EXIT_END";
const std::string DHFWK_DEV_OFFLINE = "DHFWK_DEV_OFFLINE";
const std::string DHFWK_RELEASE_FAIL = "DHFWK_RELEASE_FAIL";
const std::string DHFWK_DH_REGISTER_FAIL = "DHFWK_DH_REGISTER_FAIL";
const std::string DHFWK_DH_UNREGISTER_FAIL = "DHFWK_DH_UNREGISTER_FAIL";

void HiSysEventWriteMsg(const std::string &status, const OHOS::HiviewDFX::HiSysEvent::EventType eventType,
    const std::string &msg);
void HiSysEventWriteErrCodeMsg(const std::string &status, const OHOS::HiviewDFX::HiSysEvent::EventType eventType,
    int32_t errCode, const std::string &msg);
void HiSysEventWriteReleaseMsg(const std::string &status, const OHOS::HiviewDFX::HiSysEvent::EventType eventType,
    const DHType dhType, int32_t errCode, const std::string &msg);
void HiSysEventWriteCompOfflineMsg(const std::string &status, const OHOS::HiviewDFX::HiSysEvent::EventType eventType,
    const std::string &anonyNetworkId, const std::string &msg);
void HiSysEventWriteCompMgrFailedMsg(const std::string &status, const OHOS::HiviewDFX::HiSysEvent::EventType eventType,
    const std::string &anonyDHId, int32_t errCode, const std::string &msg);
} // namespace DistributedHardware
} // namespace OHOS
#endif