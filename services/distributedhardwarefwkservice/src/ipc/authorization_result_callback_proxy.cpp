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

#include "authorization_result_callback_proxy.h"

#include "av_trans_errno.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
AuthorizationResultCallbackProxy::AuthorizationResultCallbackProxy(const sptr<IRemoteObject> object)
    : IRemoteProxy<IAuthorizationResultCallback>(object)
{
}

AuthorizationResultCallbackProxy::~AuthorizationResultCallbackProxy()
{
}

void AuthorizationResultCallbackProxy::OnAuthorizationResult(const std::string &networkId,
    const std::string &requestId)
{
    DHLOGI("AuthorizationResultCallbackProxy OnAuthorizationResult.");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("Get Remote IRemoteObject failed!");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("AuthorizationResultCallbackProxy write token failed!");
        return;
    }
    if (!data.WriteString(networkId)) {
        DHLOGE("AuthorizationResultCallbackProxy write networkId failed!");
        return;
    }
    if (!data.WriteString(requestId)) {
        DHLOGE("AuthorizationResultCallbackProxy write requestId failed!");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAuthorizationResultCallback::Message::ON_AUTHORIZATION_RESULT), data, reply, option);
    if (ret != 0) {
        DHLOGE("AuthorizationResultCallbackProxy send requeset failed, ret: %{public}d!", ret);
        return;
    }
}
} // namespace DistributedHardware
} // namespace OHOS