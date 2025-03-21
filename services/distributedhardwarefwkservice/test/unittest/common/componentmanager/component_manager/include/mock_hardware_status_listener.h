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

#ifndef OHOS_DISTRIBUTED_HARDWARE_MOCK_HARDWARE_STATUS_LISTENER_H
#define OHOS_DISTRIBUTED_HARDWARE_MOCK_HARDWARE_STATUS_LISTENER_H

#include <gmock/gmock.h>

#include "ihardware_status_listener.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

namespace OHOS {
namespace DistributedHardware {
class MockHDSinkStatusListenerProxy : public IRemoteProxy<IHDSinkStatusListener> {
public:
    virtual ~MockHDSinkStatusListenerProxy() = default;

    MOCK_METHOD(void, OnEnable, (const DHDescriptor &));
    MOCK_METHOD(void, OnDisable, (const DHDescriptor &));
};

class MockHDSourceStatusListenerProxy : public IRemoteProxy<IHDSourceStatusListener> {
public:
    virtual ~MockHDSourceStatusListenerProxy() = default;

    MOCK_METHOD(void, OnEnable, (const std::string &, const DHDescriptor &));
    MOCK_METHOD(void, OnDisable, (const std::string &, const DHDescriptor &));
};

class MockHDSinkStatusListenerStub : public IRemoteStub<IHDSinkStatusListener> {
public:
    virtual ~MockHDSinkStatusListenerStub() = default;

    MOCK_METHOD(void, OnEnable, (const DHDescriptor &));
    MOCK_METHOD(void, OnDisable, (const DHDescriptor &));
};

class MockHDSourceStatusListenerStub : public IRemoteStub<IHDSourceStatusListener> {
public:
    virtual ~MockHDSourceStatusListenerStub() = default;

    MOCK_METHOD(void, OnEnable, (const std::string &, const DHDescriptor &));
    MOCK_METHOD(void, OnDisable, (const std::string &, const DHDescriptor &));
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_HARDWARE_MOCK_HARDWARE_STATUS_LISTENER_H
