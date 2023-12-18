/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
    sptr<IRemoteObject> object = nullptr;
    proxy_ = std::make_shared<PublisherListenerProxy>(object);
}

void PublisherListenerProxyTest::TearDown()
{
    proxy_ = nullptr;
}

/**
 * @tc.name: OnMessage_001
 * @tc.desc: Verify the OnMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(PublisherListenerProxyTest, OnMessage_001, TestSize.Level0)
{
    uint32_t invalid = 8;
    DHTopic topic = static_cast<DHTopic>(invalid);
    std::string message;
    proxy_->OnMessage(topic, message);
    EXPECT_EQ(true, message.empty());
}

/**
 * @tc.name: OnMessage_002
 * @tc.desc: Verify the OnMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(PublisherListenerProxyTest, OnMessage_002, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_START_DSCREEN;
    std::string message;
    proxy_->OnMessage(topic, message);
    EXPECT_EQ(true, message.empty());
}

/**
 * @tc.name: OnMessage_003
 * @tc.desc: Verify the OnMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(PublisherListenerProxyTest, OnMessage_003, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_START_DSCREEN;
    std::string message = "message";
    proxy_->OnMessage(topic, message);
    EXPECT_EQ(false, message.empty());
}
} // namespace DistributedHardware
} // namespace OHOS
