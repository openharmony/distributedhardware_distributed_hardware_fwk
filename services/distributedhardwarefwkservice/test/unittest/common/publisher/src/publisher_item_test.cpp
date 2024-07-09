/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "publisher_item_test.h"

using namespace testing::ext;
using namespace std;
namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr uint32_t MESSAGE_LEN = 40 * 1024 * 1024 + 10;
}

void PublisherItemTest::SetUpTestCase(void) {}

void PublisherItemTest::TearDownTestCase(void) {}

void PublisherItemTest::SetUp() {}

void PublisherItemTest::TearDown() {}

/**
 * @tc.name: AddListener_001
 * @tc.desc: Verify the AddListener ToJson function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(PublisherItemTest, AddListener_001, TestSize.Level0)
{
    PublisherItem item(DHTopic::TOPIC_MIN);
    sptr<IPublisherListener> listener = nullptr;
    item.AddListener(listener);
    EXPECT_EQ(true, item.listeners_.empty());
}

/**
 * @tc.name: AddListener_002
 * @tc.desc: Verify the AddListener ToJson function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(PublisherItemTest, AddListener_002, TestSize.Level0)
{
    PublisherItem item(DHTopic::TOPIC_MIN);
    sptr<IPublisherListener> listener(new MockIPublisherListener());
    item.AddListener(listener);
    EXPECT_EQ(false, item.listeners_.empty());
}

/**
 * @tc.name: RemoveListener_001
 * @tc.desc: Verify the RemoveListener ToJson function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(PublisherItemTest, RemoveListener_001, TestSize.Level0)
{
    PublisherItem item(DHTopic::TOPIC_MIN);
    sptr<IPublisherListener> listener = nullptr;
    item.RemoveListener(listener);
    EXPECT_EQ(true, item.listeners_.empty());

    sptr<IPublisherListener> listener1(new MockIPublisherListener());
    item.AddListener(listener1);
    item.RemoveListener(listener1);
    EXPECT_EQ(true, item.listeners_.empty());
}

/**
 * @tc.name: PublishMessage_001
 * @tc.desc: Verify the PublishMessage ToJson function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(PublisherItemTest, PublishMessage_001, TestSize.Level0)
{
    PublisherItem item(DHTopic::TOPIC_MIN);
    std::string message = "";
    item.PublishMessage(message);
    EXPECT_EQ(true, item.listeners_.empty());

    std::string msg(MESSAGE_LEN, 'a');
    item.PublishMessage(msg);
    EXPECT_EQ(true, item.listeners_.empty());
}

/**
 * @tc.name: PublishMessage_002
 * @tc.desc: Verify the PublishMessage ToJson function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(PublisherItemTest, PublishMessage_002, TestSize.Level0)
{
    PublisherItem item(DHTopic::TOPIC_MIN);
    std::string message = "message";
    sptr<IPublisherListener> listener(new MockIPublisherListener());
    item.AddListener(listener);
    item.PublishMessage(message);
    EXPECT_EQ(false, item.listeners_.empty());
}
} // namespace DistributedHardware
} // namespace OHOS
