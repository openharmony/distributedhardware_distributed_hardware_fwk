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

#include "access_listener_proxy.h"

#include "av_trans_errno.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
AccessListenerProxy::AccessListenerProxy(const sptr<IRemoteObject> object)
    : IRemoteProxy<IAccessListener>(object)
{
}

AccessListenerProxy::~AccessListenerProxy()
{
}

void AccessListenerProxy::OnRequestHardwareAccess(const std::string &requestId, AuthDeviceInfo info,
    const DHType dhType, const std::string &pkgName)
{
    DHLOGI("OnRequestHardwareAccess Start");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("OnRequestHardwareAccess error, remote info is null");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return;
    }
    if (!data.WriteString(requestId)) {
        DHLOGE("Write requestId failed!");
        return;
    }
    if (WriteDeviceInfo(data, info)) {
        DHLOGE("WriteDeviceInfo failed!");
        return;
    }
    uint32_t type = static_cast<uint32_t>(dhType);
    if (!data.WriteUint32(type)) {
        DHLOGE("Write type failed");
        return;
    }
    if (!data.WriteString(pkgName)) {
        DHLOGE("Write pkgName failed!");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAccessListener::Message::ON_REQUEST_HARDWARE_ACCESS), data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %{public}d", ret);
        return;
    }
    DHLOGI("OnRequestHardwareAccess End");
    return;
}

int32_t AccessListenerProxy::WriteDeviceInfo(MessageParcel &data, const AuthDeviceInfo info)
{
    if (!data.WriteString(info.networkId)) {
        DHLOGE("Write descriptor.dhType failed!");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteString(info.deviceName)) {
        DHLOGE("Write descriptor.id failed!");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteUint16(info.deviceType)) {
        DHLOGE("Write descriptor.dhType failed!");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    return NO_ERROR;
}
} // namespace DistributedHardware
} // namespace OHOS