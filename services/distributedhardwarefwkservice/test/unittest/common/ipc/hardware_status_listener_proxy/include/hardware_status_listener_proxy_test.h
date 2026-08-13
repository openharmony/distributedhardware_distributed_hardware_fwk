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

#ifndef OHOS_HARDWARE_STATUS_LISTENER_PROXY_TEST_H
#define OHOS_HARDWARE_STATUS_LISTENER_PROXY_TEST_H

#include <string>
#include <memory>
#include <gtest/gtest.h>

#include "distributed_hardware_errno.h"
#include "get_dh_descriptors_callback_proxy.h"
#include "get_dh_descriptors_callback_stub.h"
#include "hardware_status_listener_proxy.h"
#include "hardware_status_listener_stub.h"

namespace OHOS {
namespace DistributedHardware {
class HardwareStatusListenerProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    class TestHDSinkStatusListenerStub : public
        OHOS::DistributedHardware::HDSinkStatusListenerStub {
    public:
        TestHDSinkStatusListenerStub() = default;
        virtual ~TestHDSinkStatusListenerStub() = default;
        void OnEnable(const DHDescriptor &dhDescriptor) override;
        void OnDisable(const DHDescriptor &dhDescriptor) override;
    };

    class TestHDSourceStatusListenerStub : public
        OHOS::DistributedHardware::HDSourceStatusListenerStub {
    public:
        TestHDSourceStatusListenerStub() = default;
        virtual ~TestHDSourceStatusListenerStub() = default;
        void OnEnable(const std::string &networkId, const DHDescriptor &dhDescriptor) override;
        void OnDisable(const std::string &networkId, const DHDescriptor &dhDescriptor) override;
    };

    class TestGetDhDescriptorsCallbackStub : public
        OHOS::DistributedHardware::GetDhDescriptorsCallbackStub {
    public:
        TestGetDhDescriptorsCallbackStub() = default;
        virtual ~TestGetDhDescriptorsCallbackStub() = default;
        void OnSuccess(const std::string &networkId, const std::vector<DHDescriptor> &descriptors,
            EnableStep enableStep) override;
        void OnError(const std::string &networkId, int32_t error) override;
    };
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_HARDWARE_STATUS_LISTENER_PROXY_TEST_H