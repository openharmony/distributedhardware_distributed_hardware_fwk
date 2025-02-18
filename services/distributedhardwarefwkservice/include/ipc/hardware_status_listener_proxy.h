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

#ifndef OHOS_HARDWARE_STATUS_LISTENER_PROXY_H
#define OHOS_HARDWARE_STATUS_LISTENER_PROXY_H

#include "ihardware_status_listener.h"

#include "iremote_proxy.h"

namespace OHOS {
namespace DistributedHardware {
class HDSinkStatusListenerProxy : public IRemoteProxy<IHDSinkStatusListener> {
public:
    explicit HDSinkStatusListenerProxy(const sptr<IRemoteObject> object);
    virtual ~HDSinkStatusListenerProxy() override;

    virtual void OnEnable(const DHDescriptor &dhDescriptor) override;
    virtual void OnDisable(const DHDescriptor &dhDescriptor) override;

private:
    static inline BrokerDelegator<HDSinkStatusListenerProxy> delegator_;
};

class HDSourceStatusListenerProxy : public IRemoteProxy<IHDSourceStatusListener> {
public:
    explicit HDSourceStatusListenerProxy(const sptr<IRemoteObject> object);
    virtual ~HDSourceStatusListenerProxy() override;

    virtual void OnEnable(const std::string &networkId, const DHDescriptor &dhDescriptor) override;
    virtual void OnDisable(const std::string &networkId, const DHDescriptor &dhDescriptor) override;

private:
    static inline BrokerDelegator<HDSourceStatusListenerProxy> delegator_;
};
} // DistributedHardware
} // OHOS
#endif // OHOS_HARDWARE_STATUS_LISTENER_PROXY_H