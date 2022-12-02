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

#include "enabled_comps_dump_test.h"

#include <mutex>
#include <set>
#include <string>

#define private public
#include "enabled_comps_dump.h"
#undef private

using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
void EnabledCompsDumpTest::SetUpTestCase(void) {}

void EnabledCompsDumpTest::TearDownTestCase(void) {}

void EnabledCompsDumpTest::SetUp() {}

void EnabledCompsDumpTest::TearDown() {}

/**
 * @tc.name: DumpEnabledComp_001
 * @tc.desc: Verify the DumpEnabledComp function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(EnabledCompsDumpTest, DumpEnabledComp_001, TestSize.Level0)
{
    std::string networkId;
    DHType dhType = DHType::CAMERA;
    std::string dhId;
    EnabledCompsDump::GetInstance().DumpEnabledComp(networkId, dhType, dhId);
    EXPECT_EQ(false, EnabledCompsDump::GetInstance().compInfoSet_.empty());
}

/**
 * @tc.name: DumpDisabledComp_001
 * @tc.desc: Verify the DumpDisabledComp function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(EnabledCompsDumpTest, DumpDisabledComp_001, TestSize.Level0)
{
    std::string networkId;
    DHType dhType = DHType::CAMERA;
    std::string dhId;
    EnabledCompsDump::GetInstance().DumpEnabledComp(networkId, dhType, dhId);
    EXPECT_EQ(false, EnabledCompsDump::GetInstance().compInfoSet_.empty());
}

/**
 * @tc.name: Dump_001
 * @tc.desc: Verify the Dump function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(EnabledCompsDumpTest, Dump_001, TestSize.Level0)
{
    std::set<HidumpCompInfo> compInfoSet;
    EnabledCompsDump::GetInstance().Dump(compInfoSet);
    EXPECT_EQ(false, EnabledCompsDump::GetInstance().compInfoSet_.empty());
}
} // namespace DistributedHardware
} // namespace OHOS
