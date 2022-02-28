/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_DHUTILS_TOOL_H
#define OHOS_DISTRIBUTED_HARDWARE_DHUTILS_TOOL_H

#include <cstdint>
#include <string>

#include "device_type.h"

namespace OHOS {
namespace DistributedHardware {
/**
 * return current time in millisecond.
 */
int64_t GetCurrentTime();

/**
 * return a random string id.
 */
std::string GetRandomID();

std::string GetUUIDByNetworkId(const std::string &networkId);

DeviceInfo GetLocalDeviceInfo();
}
}
#endif