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

#ifndef OHOS_DISTRIBUTED_HARDWARE_CAPABILITY_UTILS_H
#define OHOS_DISTRIBUTED_HARDWARE_CAPABILITY_UTILS_H

#include <string>
#include <vector>

#include "capability_info.h"

namespace OHOS {
namespace DistributedHardware {
enum CapabilityInfoFilter : uint32_t {
    FILTER_DH_ID = 0,
    FILTER_DEVICE_ID = 1,
    FILTER_DEVICE_NAME = 2,
    FILTER_DEVICE_TYPE = 3,
    FILTER_DH_TYPE = 4,
    FILTER_DH_ATTRS = 5
};
class CapabilityUtils {
public:
    static int32_t GetCapabilityByValue(const std::string &value, std::shared_ptr<CapabilityInfo> &capPtr);
    static std::string GetCapabilityKey(const std::string &deviceId, const std::string &dhId);
    static bool IsCapKeyMatchDeviceId(const std::string &key, const std::string &deviceId);
};
}
}
#endif
