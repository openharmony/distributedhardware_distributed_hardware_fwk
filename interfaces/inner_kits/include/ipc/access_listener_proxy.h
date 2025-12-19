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

#ifndef OHOS_ICAMERA_ACCESS_LISTENER_PROXY_H
#define OHOS_ICAMERA_ACCESS_LISTENER_PROXY_H

#include "iaccess_listener.h"

#include "iremote_proxy.h"

namespace OHOS {
namespace DistributedHardware {
class AccessListenerProxy : public IRemoteProxy<IAccessListener> {
public:
    explicit AccessListenerProxy(const sptr<IRemoteObject> object);
    virtual ~AccessListenerProxy() override;

    virtual void OnRequestHardwareAccess(const std::string &requestId, AuthDeviceInfo info, const DHType dhType,
        const std::string &pkgName) override;

private:
    int32_t WriteDeviceInfo(MessageParcel &data, const AuthDeviceInfo info);

private:
    static inline BrokerDelegator<AccessListenerProxy> delegator_;
};
} // DistributedHardware
} // OHOS
#endif // OHOS_ICAMERA_ACCESS_LISTENER_PROXY_H