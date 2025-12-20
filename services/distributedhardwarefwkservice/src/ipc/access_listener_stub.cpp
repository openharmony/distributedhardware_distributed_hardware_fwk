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

#include "access_listener_stub.h"

#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
AccessListenerStub::AccessListenerStub()
{
}

AccessListenerStub::~AccessListenerStub()
{
}

int32_t AccessListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        DHLOGE("AccessListenerStub read valid token failed!");
        return ERR_INVALID_DATA;
    }
    IAccessListener::Message msgCode = static_cast<IAccessListener::Message>(code);
    switch (msgCode) {
        case IAccessListener::Message::ON_REQUEST_HARDWARE_ACCESS: {
            std::string requestId = data.ReadString();
            AuthDeviceInfo info { "", "", 0 };
            ReadDeviceInfo(data, info);
            DHType dhType = static_cast<DHType>(data.ReadUint32());
            std::string pkgName = data.ReadString();
            OnRequestHardwareAccess(requestId, info, dhType, pkgName);
            break;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return DH_FWK_SUCCESS;
}

int32_t AccessListenerStub::ReadDeviceInfo(MessageParcel &data, AuthDeviceInfo info)
{
    info.networkId = data.ReadString();
    info.deviceName = data.ReadString();
    info.deviceType = data.ReadUint16();
    return NO_ERROR;
}
} // namespace DistributedHardware
} // namespace OHOS