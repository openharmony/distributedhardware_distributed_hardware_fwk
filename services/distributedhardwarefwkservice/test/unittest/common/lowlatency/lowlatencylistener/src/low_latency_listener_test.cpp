/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "cJSON.h"

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
    listener_ = new(std::nothrow) LowLatencyListener;
}

void LowLatencyListenerTest::TearDown()
{
    listener_ = nullptr;
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
    listener_->OnMessage(topic, message);
    EXPECT_EQ(nullptr, listener_->AsObject());
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
    listener_->OnMessage(topic, message);
    EXPECT_EQ(nullptr, listener_->AsObject());
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
    listener_->OnMessage(topic, message);
    EXPECT_EQ(nullptr, listener_->AsObject());
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

    cJSON* json = cJSON_CreateObject();
    const char* DH_TYPE = "dh_type";
    const char* LOW_LATENCY_ENABLE = "low_latency_enable";

    cJSON_AddStringToObject(json, DH_TYPE, "dh_type");
    cJSON_AddStringToObject(json, LOW_LATENCY_ENABLE, "low_latency_enable");
    char* cjson = cJSON_Print(json);
    std::string message(cjson);
    listener_->OnMessage(topic, message);
    cJSON_free(cjson);
    cJSON_Delete(json);
    EXPECT_EQ(nullptr, listener_->AsObject());
}

/**
 * @tc.name: OnMessage_005
 * @tc.desc: Verify the OnMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyListenerTest, OnMessage_005, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_START_DSCREEN;
    std::string message;
    uint32_t MAX_MESSAGE_LEN = 40 * 1024 * 1024;
    message.resize(MAX_MESSAGE_LEN);
    listener_->OnMessage(topic, message);
    EXPECT_EQ(nullptr, listener_->AsObject());
}

/**
 * @tc.name: OnMessage_006
 * @tc.desc: Verify the OnMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyListenerTest, OnMessage_006, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_START_DSCREEN;
    cJSON* json = cJSON_CreateObject();
    const char* DH_TYPE = "dh_type";
    const char* LOW_LATENCY_ENABLE = "low_latency_enable";
    cJSON_AddNumberToObject(json, DH_TYPE, 0x01);
    cJSON_AddBoolToObject(json, LOW_LATENCY_ENABLE, true);
    char* cjson = cJSON_Print(json);
    std::string message(cjson);
    listener_->OnMessage(topic, message);
    cJSON_free(cjson);
    cJSON_Delete(json);
    EXPECT_EQ(nullptr, listener_->AsObject());
}

/**
 * @tc.name: OnMessage_007
 * @tc.desc: Verify the OnMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyListenerTest, OnMessage_007, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_START_DSCREEN;
    cJSON* json = cJSON_CreateObject();
    const char* DH_TYPE = "dh_type";
    const char* LOW_LATENCY_ENABLE = "low_latency_enable";
    cJSON_AddNumberToObject(json, DH_TYPE, 0x01);
    cJSON_AddBoolToObject(json, LOW_LATENCY_ENABLE, false);
    char* cjson = cJSON_Print(json);
    std::string message(cjson);
    listener_->OnMessage(topic, message);
    cJSON_free(cjson);
    cJSON_Delete(json);
    EXPECT_EQ(nullptr, listener_->AsObject());
}

/**
 * @tc.name: ExecuteInner_008
 * @tc.desc: Verify the OnMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyListenerTest, ExecuteInner_008, TestSize.Level0)
{
    std::string timerId;
    int32_t delayTimeMs = 1;
    LowLatencyTimer timer(timerId, delayTimeMs);
    timer.ExecuteInner();
    EXPECT_EQ(nullptr, listener_->AsObject());
}

/**
 * @tc.name: HandleStopTimer_008
 * @tc.desc: Verify the OnMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyListenerTest, HandleStopTimer_008, TestSize.Level0)
{
    std::string timerId;
    int32_t delayTimeMs = 1;
    LowLatencyTimer timer(timerId, delayTimeMs);
    timer.HandleStopTimer();
    EXPECT_EQ(nullptr, listener_->AsObject());
}
} // namespace DistributedHardware
} // namespace OHOS
