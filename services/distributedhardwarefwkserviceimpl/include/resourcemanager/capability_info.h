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

#ifndef OHOS_DISTRIBUTED_HARDWARE_CAPABILITY_INFO_H
#define OHOS_DISTRIBUTED_HARDWARE_CAPABILITY_INFO_H

#include <list>
#include <memory>
#include <map>

#include "nlohmann/json.hpp"

#include "device_type.h"

namespace OHOS {
namespace DistributedHardware {
class CapabilityInfo {
public:
    CapabilityInfo()
        : dhId_(""),
          deviceId_(""),
          deviceName_(""),
          deviceType_(0),
          dhType_(DHType::UNKNOWN),
          dhAttrs_("")
    {}

    CapabilityInfo(std::string dhId, std::string devId, std::string devName, uint16_t devType, DHType dhType,
                   std::string dhAttrs)
        : dhId_(dhId), deviceId_(devId), deviceName_(devName), deviceType_(devType), dhType_(dhType), dhAttrs_(dhAttrs)
    {}

    virtual ~CapabilityInfo() {}

    std::string GetDHId() const;

    void SetDHId(const std::string &dhId);

    std::string GetDeviceId() const;

    void SetDeviceId(const std::string &deviceId);

    std::string GetDeviceName() const;

    void SetDeviceName(const std::string &deviceName);

    uint16_t GetDeviceType() const;

    void SetDeviceType(uint16_t deviceType);

    DHType GetDHType() const;

    void SetDHType(const DHType dhType);

    std::string GetDHAttrs() const;

    void SetDHAttrs(const std::string &dhAttrs);

    virtual std::string GetKey() const;
    virtual std::string GetAnonymousKey() const;
    virtual int32_t FromJsonString(const std::string &jsonStr);
    virtual std::string ToJsonString();

private:
    std::string dhId_;
    std::string deviceId_;
    std::string deviceName_;
    uint16_t deviceType_;
    DHType dhType_;
    std::string dhAttrs_;
};

void ToJson(nlohmann::json &jsonObject, const CapabilityInfo &capability);
void FromJson(const nlohmann::json &jsonObject, CapabilityInfo &capability);

using CapabilityInfoMap = std::map<std::string, std::shared_ptr<CapabilityInfo>>;
}
}
#endif
