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

#include "dh_state_listener.h"
#include "dh_data_sync_trigger_listener.h"
#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::DistributedHardware;

class DHListenersTest : public testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(DHListenersTest, test_dh_state_listener_ctor, TestSize.Level1)
{
    auto listener = std::make_shared<DHStateListener>();
    EXPECT_NE(listener, nullptr);
}

HWTEST_F(DHListenersTest, test_dh_state_listener_on_state_changed_basic, TestSize.Level1)
{
    auto listener = std::make_shared<DHStateListener>();
    EXPECT_NO_FATAL_FAILURE(listener->OnStateChanged("test_network_id", "test_dh_id", BusinessState::IDLE));
}

HWTEST_F(DHListenersTest, test_dh_sink_state_listener_ctor, TestSize.Level1)
{
    auto listener = std::make_shared<DHSinkStateListener>();
    EXPECT_NE(listener, nullptr);
}

HWTEST_F(DHListenersTest, test_dh_sink_state_listener_on_state_changed_basic, TestSize.Level1)
{
    auto listener = std::make_shared<DHSinkStateListener>();
    EXPECT_NO_FATAL_FAILURE(listener->OnStateChanged("test_network_id", "test_dh_id", BusinessSinkState::IDLE));
}

HWTEST_F(DHListenersTest, test_dh_data_sync_trigger_listener_ctor, TestSize.Level1)
{
    auto listener = std::make_shared<DHDataSyncTriggerListener>();
    EXPECT_NE(listener, nullptr);
}

HWTEST_F(DHListenersTest, test_dh_data_sync_trigger_listener_on_data_sync_trigger_basic, TestSize.Level1)
{
    auto listener = std::make_shared<DHDataSyncTriggerListener>();
    EXPECT_NO_FATAL_FAILURE(listener->OnDataSyncTrigger("test_network_id"));
}