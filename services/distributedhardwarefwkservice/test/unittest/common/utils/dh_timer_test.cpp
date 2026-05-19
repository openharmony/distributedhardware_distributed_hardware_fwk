/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "mock/mock_dh_timer.h"
#include <chrono>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <thread>

using namespace testing::ext;
using namespace testing;
using namespace OHOS;
using namespace OHOS::DistributedHardware;

class DHTimerTest : public testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(DHTimerTest, test_start_timer_success, TestSize.Level1)
{
    auto timer = std::make_shared<MockDHTimer>("test_timer", 100);
    EXPECT_CALL(*timer, ExecuteInner()).Times(AtLeast(1));
    timer->StartTimer();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    timer->StopTimer();
}

HWTEST_F(DHTimerTest, test_stop_timer_success, TestSize.Level1)
{
    auto timer = std::make_shared<MockDHTimer>("test_timer", 100);
    EXPECT_CALL(*timer, HandleStopTimer()).Times(AtLeast(1));
    timer->StartTimer();
    timer->StopTimer();
}

HWTEST_F(DHTimerTest, test_timer_lifecycle, TestSize.Level1)
{
    auto timer = std::make_shared<MockDHTimer>("test_timer", 100);
    EXPECT_CALL(*timer, ExecuteInner()).Times(AtLeast(1));
    EXPECT_CALL(*timer, HandleStopTimer()).Times(AtLeast(1));
    timer->StartTimer();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    timer->StopTimer();
}

HWTEST_F(DHTimerTest, test_stop_timer_without_start, TestSize.Level1)
{
    auto timer = std::make_shared<MockDHTimer>("test_timer", 100);
    EXPECT_CALL(*timer, HandleStopTimer()).Times(AtLeast(1));
    timer->StopTimer();
}