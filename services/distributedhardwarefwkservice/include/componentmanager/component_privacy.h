/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_COMPONENT_PRIVACY_H
#define OHOS_DISTRIBUTED_HARDWARE_COMPONENT_PRIVACY_H

#include "idistributed_hardware_sink.h"

namespace OHOS {
namespace DistributedHardware {
class ComponentPrivacy : public PrivacyResourcesListener {
public:
    ComponentPrivacy();
    virtual ~ComponentPrivacy();
    int32_t OnPrivaceResourceMessage(const ResourceEventType &type, const std::string &subtype,
        const std::string &networkId, bool &isSensitive, bool &isSameAccout) override;
    int32_t OnResourceInfoCallback(const std::string &subtype, const std::string &networkId,
        bool &isSensitive, bool &isSameAccout);
    int32_t StartPrivacePage(const std::string &subtype, const std::string &networkId);
    int32_t StopPrivacePage();
private:
    std::string DeviceTypeToString(uint16_t deviceTypeId);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
