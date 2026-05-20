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

#include "publisher.h"
#include "ipublisher_listener.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing::ext;
using namespace OHOS::DistributedHardware;

class MockIPublisherListener : public IPublisherListener {
public:
    MOCK_METHOD2(OnMessage, void(const DHTopic, const std::string&));
};

class PublisherTest : public testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(PublisherTest, test_singleton_instance, TestSize.Level1)
{
    auto& publisher1 = Publisher::GetInstance();
    auto& publisher2 = Publisher::GetInstance();
    EXPECT_EQ(&publisher1, &publisher2);
}

HWTEST_F(PublisherTest, test_is_topic_exist_valid, TestSize.Level1)
{
    auto& publisher = Publisher::GetInstance();
    EXPECT_TRUE(publisher.IsTopicExist(DHTopic::TOPIC_LOW_LATENCY));
}

HWTEST_F(PublisherTest, test_is_topic_exist_invalid, TestSize.Level1)
{
    auto& publisher = Publisher::GetInstance();
    EXPECT_FALSE(publisher.IsTopicExist(static_cast<DHTopic>(999)));
}

HWTEST_F(PublisherTest, test_register_listener_valid_topic, TestSize.Level1)
{
    auto& publisher = Publisher::GetInstance();
    sptr<MockIPublisherListener> listener = new MockIPublisherListener();
    EXPECT_NO_FATAL_FAILURE(publisher.RegisterListener(DHTopic::TOPIC_LOW_LATENCY, listener));
}

HWTEST_F(PublisherTest, test_register_listener_invalid_topic, TestSize.Level1)
{
    auto& publisher = Publisher::GetInstance();
    sptr<MockIPublisherListener> listener = new MockIPublisherListener();
    EXPECT_NO_FATAL_FAILURE(publisher.RegisterListener(static_cast<DHTopic>(999), listener));
}

HWTEST_F(PublisherTest, test_publish_message_valid_topic, TestSize.Level1)
{
    auto& publisher = Publisher::GetInstance();
    EXPECT_NO_FATAL_FAILURE(publisher.PublishMessage(DHTopic::TOPIC_LOW_LATENCY, "test_message"));
}

HWTEST_F(PublisherTest, test_publish_message_invalid_topic, TestSize.Level1)
{
    auto& publisher = Publisher::GetInstance();
    EXPECT_NO_FATAL_FAILURE(publisher.PublishMessage(static_cast<DHTopic>(999), "test_message"));
}