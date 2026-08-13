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

#include "get_dh_descriptors_callback_proxy.h"

#include "av_trans_errno.h"
#include "constants.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
GetDhDescriptorsCallbackProxy::GetDhDescriptorsCallbackProxy(const sptr<IRemoteObject> object)
    : IRemoteProxy<IGetDhDescriptorsCallback>(object)
{
}

GetDhDescriptorsCallbackProxy::~GetDhDescriptorsCallbackProxy()
{
}

void GetDhDescriptorsCallbackProxy::OnSuccess(const std::string &networkId,
    const std::vector<DHDescriptor> &descriptors, EnableStep enableStep)
{
    DHLOGI("GetDhDescriptorsCallbackProxy OnSuccess.");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("Get Remote IRemoteObject failed!");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("GetDhDescriptorsCallbackProxy write token failed!");
        return;
    }
    if (!data.WriteString(networkId)) {
        DHLOGE("GetDhDescriptorsCallbackProxy write networkId failed!");
        return;
    }
    if (WriteDescriptors(data, descriptors)) {
        DHLOGE("WriteDescriptors failed!");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(enableStep))) {
        DHLOGE("GetDhDescriptorsCallbackProxy write enableStep failed!");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IGetDhDescriptorsCallback::Message::ON_SUCCESS), data, reply, option);
    if (ret != 0) {
        DHLOGE("GetDhDescriptorsCallbackProxy send requeset failed, ret: %{public}d!", ret);
        return;
    }
}

void GetDhDescriptorsCallbackProxy::OnError(const std::string &networkId, int32_t error)
{
    DHLOGI("GetDhDescriptorsCallbackProxy OnError.");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("Get Remote IRemoteObject failed!");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("GetDhDescriptorsCallbackProxy write token failed!");
        return;
    }
    if (!data.WriteString(networkId)) {
        DHLOGE("GetDhDescriptorsCallbackProxy write networkId failed!");
        return;
    }
    if (!data.WriteInt32(error)) {
        DHLOGE("GetDhDescriptorsCallbackProxy write error failed!");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IGetDhDescriptorsCallback::Message::ON_ERROR), data, reply, option);
    if (ret != 0) {
        DHLOGE("GetDhDescriptorsCallbackProxy send requeset failed, ret: %{public}d!", ret);
        return;
    }
}

int32_t GetDhDescriptorsCallbackProxy::WriteDescriptors(MessageParcel &data,
    const std::vector<DHDescriptor> &descriptors)
{
    uint32_t size = static_cast<uint32_t>(descriptors.size());
    if (size > MAX_DH_DESCRIPTOR_ARRAY_SIZE) {
        DHLOGE("The array descriptors are too large, size: %{public}u!", size);
        return ERR_DH_FWK_PARA_INVALID;
    }
    if (!data.WriteUint32(size)) {
        DHLOGE("Write descriptors size failed!");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    for (uint32_t i = 0; i < size; ++i) {
        const DHDescriptor &descriptor = descriptors.at(i);
        DHType type = static_cast<DHType>(descriptor.dhType);
        if (!data.WriteUint32(static_cast<uint32_t>(type))) {
            DHLOGE("Write descriptor.dhType failed!");
            return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
        }
        if (!data.WriteString(descriptor.id)) {
            DHLOGE("Write descriptor.id failed!");
            return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
        }
    }
    return NO_ERROR;
}
} // namespace DistributedHardware
} // namespace OHOS