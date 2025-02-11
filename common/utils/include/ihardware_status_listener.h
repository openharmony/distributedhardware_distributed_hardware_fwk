/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_IHARDWARE_STATUS_LISTENER_H
#define OHOS_IHARDWARE_STATUS_LISTENER_H

#include <cstdint>
#include <string>

#include <iremote_broker.h>

#include "dhardware_descriptor.h"

namespace OHOS {
namespace DistributedHardware {
class IHDSinkStatusListener : public IRemoteBroker {
public:
    virtual void OnEnable(const DHDescriptor &dhDescriptor) = 0;
    virtual void OnDisable(const DHDescriptor &dhDescriptor) = 0;

    enum class Message : uint32_t {
        ON_ENABLE,
        ON_DISABLE
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.DistributedHardware.DistributedHardwareFwk.IHDSinkStatusListener");
};

class IHDSourceStatusListener : public IRemoteBroker {
public:
    virtual void OnEnable(const std::string &networkId, const DHDescriptor &dhDescriptor) = 0;
    virtual void OnDisable(const std::string &networkId, const DHDescriptor &dhDescriptor) = 0;

    enum class Message : uint32_t {
        ON_ENABLE,
        ON_DISABLE
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.DistributedHardware.DistributedHardwareFwk.IHDSourceStatusListener");
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_IHARDWARE_STATUS_LISTENER_H