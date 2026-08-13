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

#include "low_latency_test.h"

#include "low_latency.h"
#include "low_latency_timer.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {
    const std::string LOW_LATENCY_TIMER_ID = "low_latency_timer_id";
    constexpr int32_t LOW_LATENCY_DELAY_MS = 50 * 1000;
    constexpr uint32_t MAX_SWITCH_SIZE = 256;
}

void LowLatencyTest::SetUpTestCase()
{
}

void LowLatencyTest::TearDownTestCase()
{
}

void LowLatencyTest::SetUp()
{
}

void LowLatencyTest::TearDown()
{
}

/**
 * @tc.name: EnableLowLatency_001
 * @tc.desc: Verify the EnableLowLatency function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyTest, EnableLowLatency_001, TestSize.Level1)
{
    DHType dhType = DHType::UNKNOWN;
    LowLatency::GetInstance().EnableLowLatency(dhType);
    EXPECT_EQ(true, LowLatency::GetInstance().lowLatencySwitchSet_.empty());

    dhType = DHType::MAX_DH;
    LowLatency::GetInstance().EnableLowLatency(dhType);
    EXPECT_EQ(true, LowLatency::GetInstance().lowLatencySwitchSet_.empty());
}

/**
 * @tc.name: EnableLowLatency_002
 * @tc.desc: Verify the EnableLowLatency function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyTest, EnableLowLatency_002, TestSize.Level1)
{
    DHType dhType = DHType::CAMERA;
    LowLatency::GetInstance().lowLatencyTimer_ = nullptr;
    LowLatency::GetInstance().EnableLowLatency(dhType);
    EXPECT_EQ(false, LowLatency::GetInstance().lowLatencySwitchSet_.empty());

    dhType = DHType::AUDIO;
    LowLatency::GetInstance().EnableLowLatency(dhType);
    EXPECT_EQ(false, LowLatency::GetInstance().lowLatencySwitchSet_.empty());
}

/**
 * @tc.name: EnableLowLatency_003
 * @tc.desc: Verify the EnableLowLatency function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyTest, EnableLowLatency_003, TestSize.Level1)
{
    DHType dhType = DHType::CAMERA;
    LowLatency::GetInstance().lowLatencySwitchSet_.clear();
    LowLatency::GetInstance().lowLatencyTimer_ = std::make_shared<LowLatencyTimer>(LOW_LATENCY_TIMER_ID,
        LOW_LATENCY_DELAY_MS);
    LowLatency::GetInstance().EnableLowLatency(dhType);
    EXPECT_EQ(false, LowLatency::GetInstance().lowLatencySwitchSet_.empty());

    LowLatency::GetInstance().EnableLowLatency(dhType);
    EXPECT_EQ(false, LowLatency::GetInstance().lowLatencySwitchSet_.empty());
}

/**
 * @tc.name: EnableLowLatency_004
 * @tc.desc: Verify the EnableLowLatency function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyTest, EnableLowLatency_004, TestSize.Level1)
{
    for (uint32_t i = 0; i <= MAX_SWITCH_SIZE; ++i) {
        LowLatency::GetInstance().lowLatencySwitchSet_.insert(static_cast<DHType>(i));
    }
    DHType dhType = DHType::CAMERA;
    LowLatency::GetInstance().EnableLowLatency(dhType);
    EXPECT_EQ(false, LowLatency::GetInstance().lowLatencySwitchSet_.empty());
    LowLatency::GetInstance().lowLatencySwitchSet_.clear();
}

/**
 * @tc.name: DisableLowLatency_001
 * @tc.desc: Verify the DisableLowLatency function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyTest, DisableLowLatency_001, TestSize.Level1)
{
    DHType dhType = DHType::UNKNOWN;
    LowLatency::GetInstance().DisableLowLatency(dhType);
    EXPECT_EQ(true, LowLatency::GetInstance().lowLatencySwitchSet_.empty());

    dhType = DHType::MAX_DH;
    LowLatency::GetInstance().DisableLowLatency(dhType);
    EXPECT_EQ(true, LowLatency::GetInstance().lowLatencySwitchSet_.empty());
}

/**
 * @tc.name: DisableLowLatency_002
 * @tc.desc: Verify the DisableLowLatency function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyTest, DisableLowLatency_002, TestSize.Level1)
{
    LowLatency::GetInstance().lowLatencySwitchSet_.clear();
    DHType dhType = DHType::CAMERA;
    LowLatency::GetInstance().lowLatencyTimer_ = nullptr;
    LowLatency::GetInstance().DisableLowLatency(dhType);
    EXPECT_EQ(true, LowLatency::GetInstance().lowLatencySwitchSet_.empty());

    LowLatency::GetInstance().lowLatencyTimer_ = nullptr;
    DHType dhType1 = DHType::AUDIO;
    LowLatency::GetInstance().lowLatencySwitchSet_.insert(dhType);
    LowLatency::GetInstance().lowLatencySwitchSet_.insert(dhType1);
    LowLatency::GetInstance().DisableLowLatency(dhType);
    EXPECT_EQ(false, LowLatency::GetInstance().lowLatencySwitchSet_.empty());
}

HWTEST_F(LowLatencyTest, DisableLowLatency_003, TestSize.Level1)
{
    DHType dhType = DHType::AUDIO;
    LowLatency::GetInstance().lowLatencyTimer_ = std::make_shared<LowLatencyTimer>(LOW_LATENCY_TIMER_ID,
        LOW_LATENCY_DELAY_MS);
    LowLatency::GetInstance().DisableLowLatency(dhType);
    EXPECT_EQ(true, LowLatency::GetInstance().lowLatencySwitchSet_.empty());

    DHType dhType1 = DHType::CAMERA;
    LowLatency::GetInstance().lowLatencySwitchSet_.insert(dhType);
    LowLatency::GetInstance().lowLatencySwitchSet_.insert(dhType1);
    LowLatency::GetInstance().DisableLowLatency(dhType);
    EXPECT_EQ(false, LowLatency::GetInstance().lowLatencySwitchSet_.empty());
}
} // namespace DistributedHardware
} // namespace OHOS
