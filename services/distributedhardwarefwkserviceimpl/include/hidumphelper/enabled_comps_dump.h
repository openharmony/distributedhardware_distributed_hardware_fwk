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

#ifndef OHOS_DISTRIBUTED_ENABLED_COMPS_DUMP_H
#define OHOS_DISTRIBUTED_ENABLED_COMPS_DUMP_H
#include <cstdint>
#include <mutex>
#include <set>
#include <string>

#include "device_type.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
struct HidumpCompInfo {
    std::string networkId_;
    std::string dhId_;
    DHType dhType_;

    HidumpCompInfo(std::string networkId, DHType dhType, std::string dhId)
        : networkId_(networkId), dhId_(dhId), dhType_(dhType) {}

    bool operator < (const HidumpCompInfo &other) const
    {
        return (((this->networkId_ == other.networkId_) && (this->dhId_ < other.dhId_)) ||
            (this->networkId_ < other.networkId_));
    }
};

class EnabledCompsDump {
DECLARE_SINGLE_INSTANCE_BASE(EnabledCompsDump);
public:
    void DumpEnabledComp(const std::string &networkId, const DHType dhType, const std::string &dhId);
    void DumpDisabledComp(const std::string &networkId, const DHType dhType, const std::string &dhId);

    void Dump(std::set<HidumpCompInfo> &compInfoSet);

private:
    explicit EnabledCompsDump() = default;
    ~EnabledCompsDump() = default;

private:
    std::mutex compInfosMutex_;
    std::set<HidumpCompInfo> compInfoSet_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif