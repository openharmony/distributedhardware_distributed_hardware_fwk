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

#include "constants.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "hardware_status_listener_stub.h"

namespace OHOS {
namespace DistributedHardware {
HDSinkStatusListenerStub::HDSinkStatusListenerStub()
{
}

HDSinkStatusListenerStub::~HDSinkStatusListenerStub()
{
}

int32_t HDSinkStatusListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        DHLOGE("HDSinkStatusListenerStub read valid token failed!");
        return ERR_INVALID_DATA;
    }
    DHDescriptor dhDescriptor;
    dhDescriptor.dhType = static_cast<DHType>(data.ReadUint32());
    dhDescriptor.id = data.ReadString();
    IHDSinkStatusListener::Message msgCode = static_cast<IHDSinkStatusListener::Message>(code);
    switch (msgCode) {
        case IHDSinkStatusListener::Message::ON_ENABLE: {
            OnEnable(dhDescriptor);
            break;
        }
        case IHDSinkStatusListener::Message::ON_DISABLE: {
            OnDisable(dhDescriptor);
            break;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return DH_FWK_SUCCESS;
}

HDSourceStatusListenerStub::HDSourceStatusListenerStub()
{
}

HDSourceStatusListenerStub::~HDSourceStatusListenerStub()
{
}

int32_t HDSourceStatusListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        DHLOGE("PublisherListenerStub read valid token failed!");
        return ERR_INVALID_DATA;
    }
    std::string networkId = data.ReadString();
    DHDescriptor dhDescriptor;
    dhDescriptor.dhType = static_cast<DHType>(data.ReadUint32());
    dhDescriptor.id = data.ReadString();
    IHDSinkStatusListener::Message msgCode = static_cast<IHDSinkStatusListener::Message>(code);
    switch (msgCode) {
        case IHDSinkStatusListener::Message::ON_ENABLE: {
            OnEnable(networkId, dhDescriptor);
            break;
        }
        case IHDSinkStatusListener::Message::ON_DISABLE: {
            OnDisable(networkId, dhDescriptor);
            break;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return DH_FWK_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS