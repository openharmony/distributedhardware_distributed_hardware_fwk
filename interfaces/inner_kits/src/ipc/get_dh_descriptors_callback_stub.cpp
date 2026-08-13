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

#include "get_dh_descriptors_callback_stub.h"

#include "constants.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
GetDhDescriptorsCallbackStub::GetDhDescriptorsCallbackStub()
{
}

GetDhDescriptorsCallbackStub::~GetDhDescriptorsCallbackStub()
{
}

int32_t GetDhDescriptorsCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        DHLOGE("GetDhDescriptorsCallbackStub read valid token failed!");
        return ERR_INVALID_DATA;
    }
    IGetDhDescriptorsCallback::Message msgCode = static_cast<IGetDhDescriptorsCallback::Message>(code);
    switch (msgCode) {
        case IGetDhDescriptorsCallback::Message::ON_SUCCESS: {
            std::vector<DHDescriptor> descriptors;
            std::string networkId = data.ReadString();
            ReadDescriptors(data, descriptors);
            EnableStep enableStep = static_cast<EnableStep>(data.ReadUint32());
            OnSuccess(networkId, descriptors, enableStep);
            break;
        }
        case IGetDhDescriptorsCallback::Message::ON_ERROR: {
            std::string networkId = data.ReadString();
            int32_t error = data.ReadInt32();
            OnError(networkId, error);
            break;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return DH_FWK_SUCCESS;
}

int32_t GetDhDescriptorsCallbackStub::ReadDescriptors(MessageParcel &data, std::vector<DHDescriptor> &descriptors)
{
    uint32_t size = data.ReadUint32();
    if (size > MAX_DH_DESCRIPTOR_ARRAY_SIZE) {
        DHLOGE("The array descriptors size is invalid, size: %{public}u!", size);
        return ERR_DH_FWK_PARA_INVALID;
    }
    for (uint32_t i = 0; i < size; ++i) {
        DHDescriptor descriptor;
        descriptor.dhType = static_cast<DHType>(data.ReadUint32());
        descriptor.id = data.ReadString();
        descriptors.push_back(descriptor);
    }
    return NO_ERROR;
}
} // namespace DistributedHardware
} // namespace OHOS