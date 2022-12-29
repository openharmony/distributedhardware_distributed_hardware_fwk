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

#include "low_latency_listener_test.h"
#include "nlohmann/json.hpp"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void LowLatencyListenerTest::SetUpTestCase()
{
}

void LowLatencyListenerTest::TearDownTestCase()
{
}

void LowLatencyListenerTest::SetUp()
{
    Listener_ = new(std::nothrow) LowLatencyListener;
}

void LowLatencyListenerTest::TearDown()
{
    Listener_ = nullptr;
}

/**
 * @tc.name: OnMessage_001
 * @tc.desc: Verify the OnMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyListenerTest, OnMessage_001, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_MIN;
    std::string message;
    Listener_->OnMessage(topic, message);
    EXPECT_EQ(nullptr, Listener_->AsObject());
}

/**
 * @tc.name: OnMessage_002
 * @tc.desc: Verify the OnMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyListenerTest, OnMessage_002, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_START_DSCREEN;
    std::string message;
    Listener_->OnMessage(topic, message);
    EXPECT_EQ(nullptr, Listener_->AsObject());
}

/**
 * @tc.name: OnMessage_003
 * @tc.desc: Verify the OnMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyListenerTest, OnMessage_003, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_START_DSCREEN;
    std::string message = "message";
    Listener_->OnMessage(topic, message);
    EXPECT_EQ(nullptr, Listener_->AsObject());
}

/**
 * @tc.name: OnMessage_004
 * @tc.desc: Verify the OnMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyListenerTest, OnMessage_004, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_START_DSCREEN;

    nlohmann::json json;
    const std::string DH_TYPE = "dh_type";
    const std::string LOW_LATENCY_ENABLE = "low_latency_enable";

    json[DH_TYPE] = "dh_type";
    json[LOW_LATENCY_ENABLE] = "low_latency_enable";
    std::string message = json.dump();
    Listener_->OnMessage(topic, message);
    EXPECT_EQ(nullptr, Listener_->AsObject());
}
} // namespace DistributedHardware
} // namespace OHOS
