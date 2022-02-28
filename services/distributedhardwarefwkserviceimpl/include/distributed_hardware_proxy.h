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

#ifndef OHOS_DISTRIBUTED_HARDWARE_PROXY_H
#define OHOS_DISTRIBUTED_HARDWARE_PROXY_H

#include <cstdint>

#include "iremote_proxy.h"
#include "refbase.h"
#include "idistributed_hardware.h"

namespace OHOS {
namespace DistributedHardware {
class DistributedHardwareProxy : public IRemoteProxy<IDistributedHardware> {
public:
    explicit DistributedHardwareProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IDistributedHardware>(impl)
    {
    }

    virtual ~DistributedHardwareProxy() {}
    int32_t QuerySinkVersion(std::unordered_map<DHType, std::string> &versionMap) override;

private:
    std::unordered_map<DHType, std::string> FromJson(const std::string &json) const;

    static inline BrokerDelegator<DistributedHardwareProxy> delegator_;
};
}
}
#endif
