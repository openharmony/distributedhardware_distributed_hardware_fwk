/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_META_CAPABILITY_INFO_H
#define OHOS_DISTRIBUTED_HARDWARE_META_CAPABILITY_INFO_H

#include <list>
#include <memory>
#include <map>

#include "cJSON.h"

#include "capability_info.h"
#include "device_type.h"

namespace OHOS {
namespace DistributedHardware {
class MetaCapabilityInfo : public CapabilityInfo {
public:
    MetaCapabilityInfo()
        : CapabilityInfo("", "", "", 0, DHType::UNKNOWN, "", ""), udidHash_(""), sinkVersion_("")
    {}

    MetaCapabilityInfo(std::string dhId, std::string devId, std::string devName, uint16_t devType, DHType dhType,
        std::string dhAttrs, std::string dhSubtype, std::string udidHash, std::string sinkVersion)
        : CapabilityInfo(dhId, devId, devName, devType, dhType, dhAttrs, dhSubtype),
          udidHash_(udidHash), sinkVersion_(sinkVersion) {}

    virtual ~MetaCapabilityInfo() {}

    std::string GetUdidHash() const;
    void SetUdidHash(const std::string &udidHash);
    std::string GetSinkVersion() const;
    void SetSinkVersion(const std::string &sinkVersion);

    virtual int32_t FromJsonString(const std::string &jsonStr);
    virtual std::string ToJsonString();
    bool Compare(const MetaCapabilityInfo& metaCapInfo);
    virtual std::string GetKey() const;
    virtual std::string GetAnonymousKey() const;

private:
    std::string udidHash_;
    std::string sinkVersion_;
};

void ToJson(cJSON *jsonObject, const MetaCapabilityInfo &metaCapInfo);
void FromJson(const cJSON *jsonObject, MetaCapabilityInfo &metaCapInfo);

using MetaCapInfoMap = std::map<std::string, std::shared_ptr<MetaCapabilityInfo>>;
} // namespace DistributedHardware
} // namespace OHOS
#endif
