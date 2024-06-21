/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "utils_tool_test.h"

#include <chrono>
#include <cstdint>
#include <gtest/gtest.h>
#include <set>
#include <string>
#include <thread>

#include "anonymous_string.h"
#include "dh_utils_tool.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void UtilsToolTest::SetUpTestCase(void)
{
}

void UtilsToolTest::TearDownTestCase(void)
{
}

void UtilsToolTest::SetUp()
{
}

void UtilsToolTest::TearDown()
{
}

/**
 * @tc.name: utils_tool_test_001
 * @tc.desc: Verify the utils tool GetCurrentTime function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(UtilsToolTest, utils_tool_test_001, TestSize.Level0)
{
    int64_t time1 = GetCurrentTime();
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
    int64_t time2 = GetCurrentTime();

    EXPECT_LT(time1, time2);
}

/**
 * @tc.name: utils_tool_test_002
 * @tc.desc: Verify the utils tool GetRandomID function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(UtilsToolTest, utils_tool_test_002, TestSize.Level0)
{
    uint32_t roundCnt = 100;
    std::set<std::string> idSet;

    for (uint32_t i = 0; i < roundCnt; i++) {
        idSet.insert(GetRandomID());
    }

    EXPECT_EQ(idSet.size(), roundCnt);
}

/**
 * @tc.name: utils_tool_test_003
 * @tc.desc: Verify the GetAnonyString function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(UtilsToolTest, utils_tool_test_003, TestSize.Level0)
{
    std::string str1 = "a";
    std::string str2 = "ab";
    std::string str3 = "abc";
    std::string str4 = "abcd";
    std::string str5 = "9a40932aff004e209e93524c6e35706b";

    ASSERT_STRNE(str1.c_str(), GetAnonyString(str1).c_str());
    ASSERT_STRNE(str2.c_str(), GetAnonyString(str2).c_str());
    ASSERT_STRNE(str3.c_str(), GetAnonyString(str3).c_str());
    ASSERT_STRNE(str4.c_str(), GetAnonyString(str4).c_str());
    ASSERT_STRNE(str5.c_str(), GetAnonyString(str5).c_str());
}

/**
 * @tc.name: utils_tool_test_004
 * @tc.desc: Verify the GetAnnoyInt32 function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(UtilsToolTest, utils_tool_test_004, TestSize.Level0)
{
    int32_t i1 = 0;
    int32_t i2 = 10;
    int32_t i3 = 555;
    int32_t i4 = 6666;
    int32_t i5 = 88888;

    ASSERT_STRNE(std::to_string(i1).c_str(), GetAnonyInt32(i1).c_str());
    ASSERT_STRNE(std::to_string(i2).c_str(), GetAnonyInt32(i2).c_str());
    ASSERT_STRNE(std::to_string(i3).c_str(), GetAnonyInt32(i3).c_str());
    ASSERT_STRNE(std::to_string(i4).c_str(), GetAnonyInt32(i4).c_str());
    ASSERT_STRNE(std::to_string(i5).c_str(), GetAnonyInt32(i5).c_str());
}

/**
 * @tc.name: utils_tool_test_005
 * @tc.desc: Verify the GetUUIDBySoftBus function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(UtilsToolTest, utils_tool_test_005, TestSize.Level0)
{
    std::string networkId = "";
    std::string ret = GetUUIDByDm(networkId);
    EXPECT_EQ(0, ret.size());
}

/**
 * @tc.name: utils_tool_test_006
 * @tc.desc: Verify the GetDeviceIdByUUID function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(UtilsToolTest, utils_tool_test_006, TestSize.Level0)
{
    std::string uuidEmpty = "";
    std::string ret = GetDeviceIdByUUID(uuidEmpty);
    ASSERT_EQ(0, ret.size());
}

/**
 * @tc.name: utils_tool_test_007
 * @tc.desc: Verify the GetDeviceIdByUUID function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(UtilsToolTest, utils_tool_test_007, TestSize.Level0)
{
    std::string uuid = "bb536a637105409e904d4da78290ab1";
    std::string ret = GetDeviceIdByUUID(uuid);
    ASSERT_NE(0, ret.size());
}
} // namespace DistributedHardware
} // namespace OHOS
