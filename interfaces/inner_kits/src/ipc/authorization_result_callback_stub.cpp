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

#include "authorization_result_callback_stub.h"

#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
AuthorizationResultCallbackStub::AuthorizationResultCallbackStub()
{
}

AuthorizationResultCallbackStub::~AuthorizationResultCallbackStub()
{
}

int32_t AuthorizationResultCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        DHLOGE("AuthorizationResultCallbackStub read valid token failed!");
        return ERR_INVALID_DATA;
    }
    IAuthorizationResultCallback::Message msgCode = static_cast<IAuthorizationResultCallback::Message>(code);
    switch (msgCode) {
        case IAuthorizationResultCallback::Message::ON_AUTHORIZATION_RESULT: {
            std::string networkId = data.ReadString();
            std::string requestId = data.ReadString();
            OnAuthorizationResult(networkId, requestId);
            break;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return DH_FWK_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS