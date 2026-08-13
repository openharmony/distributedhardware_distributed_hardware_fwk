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

#ifndef OHOS_DH_HITRACE_H
#define OHOS_DH_HITRACE_H

#include <cstdint>
#include <string>

#include "device_type.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace DistributedHardware {
constexpr uint64_t DHFWK_HITRACE_LABEL = HITRACE_TAG_DISTRIBUTED_HARDWARE_FWK;

const std::string COMPONENT_LOAD_START = "COMPONENT_LOAD_START";
const std::string COMPONENT_UNLOAD_START = "COMPONENT_UNLOAD_START";

const std::string COMPONENT_INIT_START = "COMPONENT_INIT_START";
const std::string COMPONENT_RELEASE_START = "COMPONENT_RELEASE_START";

const std::string DH_QUERY_START = "DH_QUERY_START";

const std::string DH_ENABLE_START = "DH_ENABLE_START";
const std::string DH_DISABLE_START = "DH_DISABLE_START";

void DHCompMgrTraceStart(const std::string &anonyNetworkId, const std::string &anonyDHId, const std::string &msg);
void DHQueryTraceStart(const DHType dhType);
void DHTraceStart(const std::string &msg);
void DHTraceEnd();
} // namespace DistributedHardware
} // namespace OHOS
#endif