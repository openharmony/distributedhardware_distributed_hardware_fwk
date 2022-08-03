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

#include "distributed_hardware_fwk_kit_test.h"

#include <cstdint>
#include <chrono>
#include <cinttypes>
#include <thread>

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "distributed_hardware_fwk_kit.h"
#include "distributed_hardware_log.h"
#include "distributed_hardware_errno.h"
#include "start_dhfwk_tool.h"

using OHOS::DistributedHardware::DHTopic;

namespace OHOS {
namespace DistributedHardware {
namespace {
    std::map<DHTopic, std::string> TOPIC_MSGS = {
        { DHTopic::TOPIC_START_DSCREEN, "Start DScreen" },
        { DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO, "Sink Project Window" },
        { DHTopic::TOPIC_STOP_DSCREEN, "Stop DScreen" },
        { DHTopic::TOPIC_DEV_OFFLINE, "Dev Offline" }
    };
    const int32_t TOPIC_MSG_CNT = 100;
    const std::string DHARDWARE_PROC_NAME = "dhardware";
    const int32_t SA_EXIT_SLEEP_TIME = 1000;
}
void DistributedHardwareFwkKitTest::SetUp()
{
    dhfwkPtr_ = std::make_shared<DistributedHardwareFwkKit>();
    startDHFWKToolPtr_ = std::make_shared<StartDHFWKTool>();
}

void DistributedHardwareFwkKitTest::TearDown()
{
    if (dhfwkPtr_ != nullptr) {
        dhfwkPtr_.reset();
        dhfwkPtr_ = nullptr;
    }

    if (startDHFWKToolPtr_ != nullptr) {
        startDHFWKToolPtr_.reset();
        startDHFWKToolPtr_ = nullptr;
    }
}

void DistributedHardwareFwkKitTest::SetUpTestCase()
{
}

void DistributedHardwareFwkKitTest::TearDownTestCase()
{
}

void DistributedHardwareFwkKitTest::TestPublisherListener::OnMessage(const DHTopic topic, const std::string& message)
{
    std::lock_guard<std::mutex> lock(mutex_);
    msgCnts_[topic]++;
}

uint32_t DistributedHardwareFwkKitTest::TestPublisherListener::GetTopicMsgCnt(const DHTopic topic)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return msgCnts_[topic];
}

HWTEST_F(DistributedHardwareFwkKitTest, RegisterListener01, testing::ext::TestSize.Level0)
{
    sptr<TestPublisherListener> listener1 = new TestPublisherListener();
    int32_t ret = dhfwkPtr_->RegisterPublisherListener(DHTopic::TOPIC_START_DSCREEN, listener1);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    sptr<TestPublisherListener> listener2 = new TestPublisherListener();
    ret = dhfwkPtr_->RegisterPublisherListener(DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO, listener2);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    sptr<TestPublisherListener> listener3 = new TestPublisherListener();
    ret = dhfwkPtr_->RegisterPublisherListener(DHTopic::TOPIC_STOP_DSCREEN, listener3);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    sptr<TestPublisherListener> listener4 = new TestPublisherListener();
    ret = dhfwkPtr_->RegisterPublisherListener(DHTopic::TOPIC_DEV_OFFLINE, listener4);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    ret = dhfwkPtr_->UnregisterPublisherListener(DHTopic::TOPIC_START_DSCREEN, listener1);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    ret = dhfwkPtr_->UnregisterPublisherListener(DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO, listener2);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    ret = dhfwkPtr_->UnregisterPublisherListener(DHTopic::TOPIC_STOP_DSCREEN, listener3);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    ret = dhfwkPtr_->UnregisterPublisherListener(DHTopic::TOPIC_DEV_OFFLINE, listener4);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(DistributedHardwareFwkKitTest, PublishMessage01, testing::ext::TestSize.Level0)
{
    EXPECT_EQ(true, startDHFWKToolPtr_->StartDHFWK());

    sptr<TestPublisherListener> listener1 = new TestPublisherListener();
    int32_t ret = dhfwkPtr_->RegisterPublisherListener(DHTopic::TOPIC_START_DSCREEN, listener1);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    sptr<TestPublisherListener> listener2 = new TestPublisherListener();
    ret = dhfwkPtr_->RegisterPublisherListener(DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO, listener2);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    sptr<TestPublisherListener> listener3 = new TestPublisherListener();
    ret = dhfwkPtr_->RegisterPublisherListener(DHTopic::TOPIC_STOP_DSCREEN, listener3);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    sptr<TestPublisherListener> listener4 = new TestPublisherListener();
    ret = dhfwkPtr_->RegisterPublisherListener(DHTopic::TOPIC_DEV_OFFLINE, listener4);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    int32_t cnt = 0;
    while (cnt < TOPIC_MSG_CNT) {
        for (const auto &tpMsg : TOPIC_MSGS) {
            ret = dhfwkPtr_->PublishMessage(tpMsg.first, tpMsg.second);
            EXPECT_EQ(DH_FWK_SUCCESS, ret);
        }
        cnt++;
    }

    EXPECT_EQ(TOPIC_MSG_CNT, listener1->GetTopicMsgCnt(DHTopic::TOPIC_START_DSCREEN));
    EXPECT_EQ(TOPIC_MSG_CNT, listener2->GetTopicMsgCnt(DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO));
    EXPECT_EQ(TOPIC_MSG_CNT, listener3->GetTopicMsgCnt(DHTopic::TOPIC_STOP_DSCREEN));
    EXPECT_EQ(TOPIC_MSG_CNT, listener4->GetTopicMsgCnt(DHTopic::TOPIC_DEV_OFFLINE));

    ret = dhfwkPtr_->UnregisterPublisherListener(DHTopic::TOPIC_START_DSCREEN, listener1);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    ret = dhfwkPtr_->UnregisterPublisherListener(DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO, listener2);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    ret = dhfwkPtr_->UnregisterPublisherListener(DHTopic::TOPIC_STOP_DSCREEN, listener3);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    ret = dhfwkPtr_->UnregisterPublisherListener(DHTopic::TOPIC_DEV_OFFLINE, listener4);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    startDHFWKToolPtr_->KillProcess(DHARDWARE_PROC_NAME.c_str());
    std::this_thread::sleep_for(std::chrono::milliseconds(SA_EXIT_SLEEP_TIME));
}

HWTEST_F(DistributedHardwareFwkKitTest, PublishMessage02, testing::ext::TestSize.Level0)
{
    EXPECT_EQ(true, startDHFWKToolPtr_->StartDHFWK());

    sptr<TestPublisherListener> listener = new TestPublisherListener();
    int32_t ret = dhfwkPtr_->RegisterPublisherListener(DHTopic::TOPIC_START_DSCREEN, listener);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    ret = dhfwkPtr_->RegisterPublisherListener(DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO, listener);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    ret = dhfwkPtr_->RegisterPublisherListener(DHTopic::TOPIC_STOP_DSCREEN, listener);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    ret = dhfwkPtr_->RegisterPublisherListener(DHTopic::TOPIC_DEV_OFFLINE, listener);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    int32_t cnt = 0;
    while (cnt < TOPIC_MSG_CNT) {
        for (const auto &tpMsg : TOPIC_MSGS) {
            ret = dhfwkPtr_->PublishMessage(tpMsg.first, tpMsg.second);
            EXPECT_EQ(DH_FWK_SUCCESS, ret);
        }
        cnt++;
    }

    EXPECT_EQ(TOPIC_MSG_CNT, listener->GetTopicMsgCnt(DHTopic::TOPIC_START_DSCREEN));
    EXPECT_EQ(TOPIC_MSG_CNT, listener->GetTopicMsgCnt(DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO));
    EXPECT_EQ(TOPIC_MSG_CNT, listener->GetTopicMsgCnt(DHTopic::TOPIC_STOP_DSCREEN));
    EXPECT_EQ(TOPIC_MSG_CNT, listener->GetTopicMsgCnt(DHTopic::TOPIC_DEV_OFFLINE));

    ret = dhfwkPtr_->UnregisterPublisherListener(DHTopic::TOPIC_START_DSCREEN, listener);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    ret = dhfwkPtr_->UnregisterPublisherListener(DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO, listener);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    ret = dhfwkPtr_->UnregisterPublisherListener(DHTopic::TOPIC_STOP_DSCREEN, listener);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    ret = dhfwkPtr_->UnregisterPublisherListener(DHTopic::TOPIC_DEV_OFFLINE, listener);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    startDHFWKToolPtr_->KillProcess(DHARDWARE_PROC_NAME.c_str());
    std::this_thread::sleep_for(std::chrono::milliseconds(SA_EXIT_SLEEP_TIME));
}
} // namespace DistributedHardware
} // namespace OHOS