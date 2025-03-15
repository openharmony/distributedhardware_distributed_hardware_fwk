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

#include "publisher_listener_proxy_test.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void PublisherListenerProxyTest::SetUpTestCase()
{
}

void PublisherListenerProxyTest::TearDownTestCase()
{
}

void PublisherListenerProxyTest::SetUp()
{
}

void PublisherListenerProxyTest::TearDown()
{
}

void PublisherListenerProxyTest::TestPublisherListenerStub::OnMessage(const DHTopic topic, const std::string& message)
{
    (void)topic;
    (void)message;
}

/**
 * @tc.name: OnMessage_001
 * @tc.desc: Verify the OnMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(PublisherListenerProxyTest, OnMessage_001, TestSize.Level1)
{
    DHTopic topic = DHTopic::TOPIC_MIN;
    std::string message = "";
    sptr<IRemoteObject> listenerStub(new TestPublisherListenerStub());
    ASSERT_TRUE(listenerStub != nullptr);
    PublisherListenerProxy listenerProxy(listenerStub);
    EXPECT_NO_FATAL_FAILURE(listenerProxy.OnMessage(topic, message));
}

/**
 * @tc.name: OnMessage_002
 * @tc.desc: Verify the OnMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(PublisherListenerProxyTest, OnMessage_002, TestSize.Level1)
{
    DHTopic topic = DHTopic::TOPIC_MAX;
    std::string message = "";
    sptr<IRemoteObject> listenerStub(new TestPublisherListenerStub());
    ASSERT_TRUE(listenerStub != nullptr);
    PublisherListenerProxy listenerProxy(listenerStub);
    EXPECT_NO_FATAL_FAILURE(listenerProxy.OnMessage(topic, message));
}

/**
 * @tc.name: OnMessage_003
 * @tc.desc: Verify the OnMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(PublisherListenerProxyTest, OnMessage_003, TestSize.Level1)
{
    DHTopic topic = DHTopic::TOPIC_LOW_LATENCY;
    std::string message = "";
    sptr<IRemoteObject> listenerStub(new TestPublisherListenerStub());
    ASSERT_TRUE(listenerStub != nullptr);
    PublisherListenerProxy listenerProxy(listenerStub);
    EXPECT_NO_FATAL_FAILURE(listenerProxy.OnMessage(topic, message));
}

HWTEST_F(PublisherListenerProxyTest, OnMessage_004, TestSize.Level1)
{
    DHTopic topic = DHTopic::TOPIC_LOW_LATENCY;
    std::string message = "message_test";
    sptr<IRemoteObject> listenerStub(new TestPublisherListenerStub());
    ASSERT_TRUE(listenerStub != nullptr);
    PublisherListenerProxy listenerProxy(listenerStub);
    EXPECT_NO_FATAL_FAILURE(listenerProxy.OnMessage(topic, message));
}
} // namespace DistributedHardware
} // namespace OHOS
