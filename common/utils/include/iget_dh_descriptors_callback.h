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

#ifndef OHOS_IGET_DH_DESCRIPTORS_CALLBACK_H
#define OHOS_IGET_DH_DESCRIPTORS_CALLBACK_H

#include <cstdint>
#include <string>

#include <iremote_broker.h>

#include "dhardware_descriptor.h"

namespace OHOS {
namespace DistributedHardware {
class IGetDhDescriptorsCallback : public IRemoteBroker {
public:
    virtual void OnSuccess(const std::string &networkId, const std::vector<DHDescriptor> &descriptors,
        EnableStep enableStep) = 0;
    virtual void OnError(const std::string &networkId, int32_t error) = 0;

    enum class Message : uint32_t {
        ON_SUCCESS,
        ON_ERROR
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.DistributedHardware.DistributedHardwareFwk.IGetDhDescriptorsCallback");
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_IGET_DH_DESCRIPTORS_CALLBACK_H