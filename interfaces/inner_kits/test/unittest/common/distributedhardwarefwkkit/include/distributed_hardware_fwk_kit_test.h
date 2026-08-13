/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISRIBUTED_HARDWARE_FWK_KIT_H
#define OHOS_DISRIBUTED_HARDWARE_FWK_KIT_H

#include <map>
#include <memory>
#include <mutex>

#include <gtest/gtest.h>
#include <refbase.h>

#include "dm_device_info.h"
#include "device_manager.h"

#include "av_trans_errno.h"
#include "distributed_hardware_fwk_kit.h"
#include "get_dh_descriptors_callback_stub.h"
#include "hardware_status_listener_stub.h"
#include "publisher_listener_stub.h"
#include "system_ability_load_callback_stub.h"

using OHOS::DistributedHardware::DHTopic;

namespace OHOS {
namespace DistributedHardware {
class DistributedHardwareFwkKitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
    bool StartDHFWK();
public:
    class TestPublisherListener : public PublisherListenerStub {
    public:
        TestPublisherListener() = default;
        virtual ~TestPublisherListener() = default;
        void OnMessage(const DHTopic topic, const std::string& message);
        uint32_t GetTopicMsgCnt(const DHTopic topic);
    private:
        std::mutex mutex_;
        std::map<DHTopic, uint32_t> msgCnts_ = {};
    };

    class TestHDSinkStatusListener : public HDSinkStatusListenerStub {
    public:
        TestHDSinkStatusListener() = default;
        virtual ~TestHDSinkStatusListener() = default;
    protected:
        void OnEnable(const DHDescriptor &dhDescriptor) override;
        void OnDisable(const DHDescriptor &dhDescriptor) override;
    };

    class TestHDSourceStatusListener : public HDSourceStatusListenerStub {
    public:
        TestHDSourceStatusListener() = default;
        virtual ~TestHDSourceStatusListener() = default;
    protected:
        void OnEnable(const std::string &networkId, const DHDescriptor &dhDescriptor) override;
        void OnDisable(const std::string &networkId, const DHDescriptor &dhDescriptor) override;
    };

    class TestDmInitCallback : public DmInitCallback {
    public:
        TestDmInitCallback() = default;
        virtual ~TestDmInitCallback() = default;
    protected:
        void OnRemoteDied() override;
    };

    class TestGetDistributedHardwareCallback : public GetDhDescriptorsCallbackStub {
    public:
        TestGetDistributedHardwareCallback() = default;
        virtual ~TestGetDistributedHardwareCallback() = default;
    protected:
        void OnSuccess(const std::string &networkId, const std::vector<DHDescriptor> &descriptors,
            EnableStep enableStep) override;
        void OnError(const std::string &networkId, int32_t error) override;
    };

public:
    std::shared_ptr<DistributedHardwareFwkKit> dhfwkPtr_ = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISRIBUTED_HARDWARE_FWK_KIT_H
