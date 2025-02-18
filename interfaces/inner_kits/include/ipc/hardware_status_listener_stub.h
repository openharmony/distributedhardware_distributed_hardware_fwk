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

#ifndef OHOS_HARDWARE_STATUS_LISTENER_STUB_H
#define OHOS_HARDWARE_STATUS_LISTENER_STUB_H

#include "ihardware_status_listener.h"

#include "iremote_stub.h"

namespace OHOS {
namespace DistributedHardware {
class HDSinkStatusListenerStub : public IRemoteStub<IHDSinkStatusListener> {
public:
    HDSinkStatusListenerStub();
    virtual ~HDSinkStatusListenerStub() override;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    DISALLOW_COPY_AND_MOVE(HDSinkStatusListenerStub);
};

class HDSourceStatusListenerStub : public IRemoteStub<IHDSourceStatusListener> {
public:
    HDSourceStatusListenerStub();
    virtual ~HDSourceStatusListenerStub() override;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    DISALLOW_COPY_AND_MOVE(HDSourceStatusListenerStub);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_HARDWARE_STATUS_LISTENER_STUB_H