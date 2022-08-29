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
    const std::string DHARDWARE_PROC_NAME = "dhardware";
}
void DistributedHardwareFwkKitTest::SetUp()
{
    dhfwkPtr_ = std::make_shared<DistributedHardwareFwkKit>();
}

void DistributedHardwareFwkKitTest::TearDown()
{
    if (dhfwkPtr_ != nullptr) {
        dhfwkPtr_.reset();
        dhfwkPtr_ = nullptr;
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
} // namespace DistributedHardware
} // namespace OHOS