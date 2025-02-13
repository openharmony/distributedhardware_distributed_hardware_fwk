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
#include "dh_utils_tool.h"
#include "distributed_hardware_log.h"
#include "hardware_status_listener_proxy.h"

namespace OHOS {
namespace DistributedHardware {
HDSinkStatusListenerProxy::HDSinkStatusListenerProxy(const sptr<IRemoteObject> object)
    : IRemoteProxy<IHDSinkStatusListener>(object)
{
}

HDSinkStatusListenerProxy::~HDSinkStatusListenerProxy()
{
}

void HDSinkStatusListenerProxy::OnEnable(const DHDescriptor &dhDescriptor)
{
    DHLOGI("HDSinkStatusListenerProxy OnEnable.");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("Get Remote IRemoteObject failed!");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("HDSinkStatusListenerProxy write token failed!");
        return;
    }
    if (!data.WriteUint32((uint32_t)dhDescriptor.dhType)) {
        DHLOGE("HDSinkStatusListenerProxy write dhDescriptor.dhType failed!");
        return;
    }
    if (!data.WriteString(dhDescriptor.id)) {
        DHLOGE("HDSinkStatusListenerProxy write dhDescriptor.id failed!");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IHDSinkStatusListener::Message::ON_ENABLE), data, reply, option);
    if (ret != 0) {
        DHLOGE("HDSinkStatusListenerProxy send requeset failed, ret: %{public}d!", ret);
        return;
    }
}

void HDSinkStatusListenerProxy::OnDisable(const DHDescriptor &dhDescriptor)
{
    DHLOGI("HDSinkStatusListenerProxy OnDisable.");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("Get Remote IRemoteObject failed!");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("HDSinkStatusListenerProxy write token failed!");
        return;
    }
    if (!data.WriteUint32((uint32_t)dhDescriptor.dhType)) {
        DHLOGE("HDSinkStatusListenerProxy write dhDescriptor.dhType failed!");
        return;
    }
    if (!data.WriteString(dhDescriptor.id)) {
        DHLOGE("HDSinkStatusListenerProxy write dhDescriptor.id failed!");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IHDSinkStatusListener::Message::ON_DISABLE), data, reply, option);
    if (ret != 0) {
        DHLOGE("HDSinkStatusListenerProxy send requeset failed, ret: %{public}d!", ret);
        return;
    }
}

HDSourceStatusListenerProxy::HDSourceStatusListenerProxy(const sptr<IRemoteObject> object)
    : IRemoteProxy<IHDSourceStatusListener>(object)
{
}

HDSourceStatusListenerProxy::~HDSourceStatusListenerProxy()
{
}

void HDSourceStatusListenerProxy::OnEnable(const std::string &networkId, const DHDescriptor &dhDescriptor)
{
    DHLOGI("HDSourceStatusListenerProxy OnEnable.");
    if (!IsIdLengthValid(networkId)) {
        DHLOGE("IsIdLengthValid check failed!");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("Get Remote IRemoteObject failed!");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("HDSourceStatusListenerProxy write token failed!");
        return;
    }
    if (!data.WriteString(networkId)) {
        DHLOGE("HDSourceStatusListenerProxy write networkId failed!");
        return;
    }
    if (!data.WriteUint32((uint32_t)dhDescriptor.dhType)) {
        DHLOGE("HDSourceStatusListenerProxy write dhDescriptor.dhType failed!");
        return;
    }
    if (!data.WriteString(dhDescriptor.id)) {
        DHLOGE("HDSourceStatusListenerProxy write dhDescriptor.id failed!");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(HDSourceStatusListenerProxy::Message::ON_ENABLE), data, reply, option);
    if (ret != 0) {
        DHLOGE("HDSourceStatusListenerProxy send requeset failed, ret: %{public}d!", ret);
        return;
    }
}

void HDSourceStatusListenerProxy::OnDisable(const std::string &networkId, const DHDescriptor &dhDescriptor)
{
    DHLOGI("HDSourceStatusListenerProxy OnDisable.");
    if (!IsIdLengthValid(networkId)) {
        DHLOGE("IsIdLengthValid check failed!");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("Get Remote IRemoteObject failed!");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("HDSourceStatusListenerProxy write token failed!");
        return;
    }
    if (!data.WriteString(networkId)) {
        DHLOGE("HDSourceStatusListenerProxy write networkId failed!");
        return;
    }
    if (!data.WriteUint32((uint32_t)dhDescriptor.dhType)) {
        DHLOGE("HDSourceStatusListenerProxy write dhDescriptor.dhType failed!");
        return;
    }
    if (!data.WriteString(dhDescriptor.id)) {
        DHLOGE("HDSourceStatusListenerProxy write dhDescriptor.id failed!");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(HDSourceStatusListenerProxy::Message::ON_DISABLE), data, reply, option);
    if (ret != 0) {
        DHLOGE("HDSourceStatusListenerProxy send requeset failed, ret: %{public}d!", ret);
        return;
    }
}
} // namespace DistributedHardware
} // namespace OHOS