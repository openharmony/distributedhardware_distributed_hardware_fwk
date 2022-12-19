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

#include "hidump_helper_test.h"

#include <mutex>
#include <set>
#include <string>

#define private public
#include "hidump_helper.h"
#undef private
#include "distributed_hardware_errno.h"

using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
void HidumpHelperTest::SetUpTestCase(void) {}

void HidumpHelperTest::TearDownTestCase(void) {}

void HidumpHelperTest::SetUp() {}

void HidumpHelperTest::TearDown() {}

/**
 * @tc.name: Dump_001
 * @tc.desc: Verify the Dump function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(HidumpHelperTest, Dump_001, TestSize.Level0)
{
    std::vector<std::string> args;
    std::string result;
    int32_t ret = HidumpHelper::GetInstance().Dump(args, result);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: ProcessDump_001
 * @tc.desc: Verify the ProcessDump function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(HidumpHelperTest, ProcessDump_001, TestSize.Level0)
{
    HidumpFlag flag = HidumpFlag::UNKNOWN;
    std::string result;
    int32_t ret = HidumpHelper::GetInstance().ProcessDump(flag, result);
    EXPECT_NE(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: ShowAllLoadedComps_001
 * @tc.desc: Verify the ShowAllLoadedComps function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(HidumpHelperTest, ShowAllLoadedComps_001, TestSize.Level0)
{
    std::string result;
    int32_t ret = HidumpHelper::GetInstance().ShowAllLoadedComps(result);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: ShowAllEnabledComps_001
 * @tc.desc: Verify the ShowAllEnabledComps function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(HidumpHelperTest, ShowAllEnabledComps_001, TestSize.Level0)
{
    std::string result;
    int32_t ret = HidumpHelper::GetInstance().ShowAllEnabledComps(result);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: ShowAllTaskInfos_001
 * @tc.desc: Verify the ShowAllTaskInfos function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(HidumpHelperTest, ShowAllTaskInfos_001, TestSize.Level0)
{
    std::string result;
    int32_t ret = HidumpHelper::GetInstance().ShowAllTaskInfos(result);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: ShowAllCapabilityInfos_001
 * @tc.desc: Verify the ShowAllCapabilityInfos function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(HidumpHelperTest, ShowAllCapabilityInfos_001, TestSize.Level0)
{
    std::string result;
    int32_t ret = HidumpHelper::GetInstance().ShowAllCapabilityInfos(result);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: ShowHelp_001
 * @tc.desc: Verify the ShowHelp function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(HidumpHelperTest, ShowHelp_001, TestSize.Level0)
{
    std::string result;
    int32_t ret = HidumpHelper::GetInstance().ShowHelp(result);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: ShowIllealInfomation_001
 * @tc.desc: Verify the ShowIllealInfomation function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(HidumpHelperTest, ShowIllealInfomation_001, TestSize.Level0)
{
    std::string result;
    int32_t ret = HidumpHelper::GetInstance().ShowIllealInfomation(result);
    EXPECT_NE(DH_FWK_SUCCESS, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
