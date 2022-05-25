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

#include <string>

#include "hitrace_meter.h"

namespace OHOS {
namespace DistributedHardware {
constexpr uint64_t DHFWK_HITRACE_LABEL = HITRACE_TAG_DISTRIBUTED_HARDWARE_FWK;

const std::string DH_FWK_COMPONENT_ENABLE_START = "DH_FWK_COMPONENT_ENABLE_START";
const std::string DH_FWK_COMPONENT_DISABLE_START = "DH_FWK_COMPONENT_DISABLE_START";
} // namespace DistributedHardware
} // namespace OHOS