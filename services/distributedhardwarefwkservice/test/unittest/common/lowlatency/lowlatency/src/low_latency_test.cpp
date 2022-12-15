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

#include "low_latency_test.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
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
    LowLatency::GetInstance().lowLatencySwitchSet_.clear();
}

/**
 * @tc.name: EnableLowLatency_001
 * @tc.desc: Verify the EnableLowLatency function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyTest, EnableLowLatency_001, TestSize.Level0)
{
    DHType dhType = DHType::UNKNOWN;
    LowLatency::GetInstance().EnableLowLatency(dhType);
    EXPECT_EQ(true, LowLatency::GetInstance().lowLatencySwitchSet_.empty());
}

/**
 * @tc.name: EnableLowLatency_002
 * @tc.desc: Verify the EnableLowLatency function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyTest, EnableLowLatency_002, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    LowLatency::GetInstance().EnableLowLatency(dhType);
    EXPECT_EQ(false, LowLatency::GetInstance().lowLatencySwitchSet_.empty());
}

/**
 * @tc.name: EnableLowLatency_003
 * @tc.desc: Verify the EnableLowLatency function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyTest, EnableLowLatency_003, TestSize.Level0)
{
    LowLatency::GetInstance().lowLatencySwitchSet_.insert(DHType::AUDIO);
    DHType dhType = DHType::CAMERA;
    LowLatency::GetInstance().EnableLowLatency(dhType);
    EXPECT_EQ(false, LowLatency::GetInstance().lowLatencySwitchSet_.empty());
}

/**
 * @tc.name: EnableLowLatency_004
 * @tc.desc: Verify the EnableLowLatency function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyTest, EnableLowLatency_004, TestSize.Level0)
{
    uint32_t MAX_SWITCH_SIZE = 256;
    for (uint32_t i = 0; i <= MAX_SWITCH_SIZE; ++i) {
        LowLatency::GetInstance().lowLatencySwitchSet_.insert(static_cast<DHType>(i));
    }
    DHType dhType = DHType::CAMERA;
    LowLatency::GetInstance().EnableLowLatency(dhType);
    EXPECT_EQ(false, LowLatency::GetInstance().lowLatencySwitchSet_.empty());
}

/**
 * @tc.name: DisableLowLatency_001
 * @tc.desc: Verify the DisableLowLatency function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyTest, DisableLowLatency_001, TestSize.Level0)
{
    DHType dhType = DHType::UNKNOWN;
    LowLatency::GetInstance().DisableLowLatency(dhType);
    EXPECT_EQ(true, LowLatency::GetInstance().lowLatencySwitchSet_.empty());
}

/**
 * @tc.name: DisableLowLatency_002
 * @tc.desc: Verify the DisableLowLatency function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyTest, DisableLowLatency_002, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    LowLatency::GetInstance().lowLatencySwitchSet_.insert(dhType);
    LowLatency::GetInstance().DisableLowLatency(dhType);
    EXPECT_EQ(true, LowLatency::GetInstance().lowLatencySwitchSet_.empty());
}

/**
 * @tc.name: CloseLowLatency_001
 * @tc.desc: Verify the CloseLowLatency function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(LowLatencyTest, CloseLowLatency_001, TestSize.Level0)
{
    LowLatency::GetInstance().CloseLowLatency();
    EXPECT_EQ(true, LowLatency::GetInstance().lowLatencySwitchSet_.empty());
}
} // namespace DistributedHardware
} // namespace OHOS
