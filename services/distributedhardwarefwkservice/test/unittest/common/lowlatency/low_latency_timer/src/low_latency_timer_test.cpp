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

#include "low_latency_timer.h"
#include "mock/mock_res_sched_client.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>

using namespace testing::ext;
using namespace OHOS::DistributedHardware;
using namespace OHOS::ResourceSchedule;

class LowLatencyTimerTest : public testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(LowLatencyTimerTest, test_execute_inner_enable_mode, TestSize.Level1)
{
    auto timer = std::make_shared<LowLatencyTimer>("test_timer", 100);
    auto& mockClient = MockResSchedClient::GetInstance();
    EXPECT_CALL(mockClient, ReportData(_, 0, _)).Times(1);
    timer->StartTimer();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    timer->StopTimer();
}

HWTEST_F(LowLatencyTimerTest, test_handle_stop_timer_disable_mode, TestSize.Level1)
{
    auto timer = std::make_shared<LowLatencyTimer>("test_timer", 100);
    auto& mockClient = MockResSchedClient::GetInstance();
    EXPECT_CALL(mockClient, ReportData(_, 1, _)).Times(AtLeast(1));
    timer->StartTimer();
    timer->StopTimer();
}