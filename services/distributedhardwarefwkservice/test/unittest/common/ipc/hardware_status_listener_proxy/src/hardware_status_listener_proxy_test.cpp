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

#include "hardware_status_listener_proxy_test.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void HardwareStatusListenerProxyTest::SetUpTestCase()
{
}

void HardwareStatusListenerProxyTest::TearDownTestCase()
{
}

void HardwareStatusListenerProxyTest::SetUp()
{
}

void HardwareStatusListenerProxyTest::TearDown()
{
}

void HardwareStatusListenerProxyTest::TestHDSinkStatusListenerStub::OnEnable(const DHDescriptor &dhDescriptor)
{
    (void)dhDescriptor.dhType;
}

void HardwareStatusListenerProxyTest::TestHDSinkStatusListenerStub::OnDisable(const DHDescriptor &dhDescriptor)
{
    (void)dhDescriptor.dhType;
}

void HardwareStatusListenerProxyTest::TestHDSourceStatusListenerStub::OnEnable(const std::string &networkId,
    const DHDescriptor &dhDescriptor)
{
    (void)networkId;
    (void)dhDescriptor.dhType;
}

void HardwareStatusListenerProxyTest::TestHDSourceStatusListenerStub::OnDisable(const std::string &networkId,
    const DHDescriptor &dhDescriptor)
{
    (void)networkId;
    (void)dhDescriptor.dhType;
}

/**
 * @tc.name: OnMessage_001
 * @tc.desc: Verify the OnMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(HardwareStatusListenerProxyTest, OnMessage_001, TestSize.Level1)
{
    DHDescriptor dhDescriptor {
        .id = "id_test",
        .dhType = DHType::CAMERA
    };
    sptr<IRemoteObject> listenerStub(new TestHDSinkStatusListenerStub());
    ASSERT_TRUE(listenerStub != nullptr);
    HDSinkStatusListenerProxy listenerProxy(listenerStub);
    EXPECT_NO_FATAL_FAILURE(listenerProxy.OnEnable(dhDescriptor));
}

/**
 * @tc.name: OnMessage_002
 * @tc.desc: Verify the OnMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(HardwareStatusListenerProxyTest, OnMessage_002, TestSize.Level1)
{
    DHDescriptor dhDescriptor {
        .id = "id_test",
        .dhType = DHType::CAMERA
    };
    sptr<IRemoteObject> listenerStub(new TestHDSinkStatusListenerStub());
    ASSERT_TRUE(listenerStub != nullptr);
    HDSinkStatusListenerProxy listenerProxy(listenerStub);
    EXPECT_NO_FATAL_FAILURE(listenerProxy.OnDisable(dhDescriptor));
}

/**
 * @tc.name: OnMessage_101
 * @tc.desc: Verify the OnMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(HardwareStatusListenerProxyTest, OnMessage_101, TestSize.Level1)
{
    DHDescriptor dhDescriptor {
        .id = "id_test",
        .dhType = DHType::CAMERA
    };
    sptr<IRemoteObject> listenerStub(new TestHDSourceStatusListenerStub());
    ASSERT_TRUE(listenerStub != nullptr);
    HDSourceStatusListenerProxy listenerProxy(listenerStub);
    EXPECT_NO_FATAL_FAILURE(listenerProxy.OnEnable("netWorkId_test", dhDescriptor));
}

/**
 * @tc.name: OnMessage_102
 * @tc.desc: Verify the OnMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(HardwareStatusListenerProxyTest, OnMessage_102, TestSize.Level1)
{
    DHDescriptor dhDescriptor {
        .id = "id_test",
        .dhType = DHType::CAMERA
    };
    sptr<IRemoteObject> listenerStub(new TestHDSourceStatusListenerStub());
    ASSERT_TRUE(listenerStub != nullptr);
    HDSourceStatusListenerProxy listenerProxy(listenerStub);
    EXPECT_NO_FATAL_FAILURE(listenerProxy.OnDisable("netWorkId_test", dhDescriptor));
}
} // namespace DistributedHardware
} // namespace OHOS
