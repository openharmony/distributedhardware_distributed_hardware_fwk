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

#ifndef OHOS_AUTHORIZATION_RESULT_CALLBACK_PROXY_H
#define OHOS_AUTHORIZATION_RESULT_CALLBACK_PROXY_H

#include "iauthorization_result_callback.h"

#include "iremote_proxy.h"

namespace OHOS {
namespace DistributedHardware {
class AuthorizationResultCallbackProxy : public IRemoteProxy<IAuthorizationResultCallback> {
public:
    explicit AuthorizationResultCallbackProxy(const sptr<IRemoteObject> object);
    virtual ~AuthorizationResultCallbackProxy() override;

    virtual void OnAuthorizationResult(const std::string &networkId, const std::string &requestId) override;

private:
    static inline BrokerDelegator<AuthorizationResultCallbackProxy> delegator_;
};
} // DistributedHardware
} // OHOS
#endif // OHOS_AUTHORIZATION_RESULT_CALLBACK_PROXY_H