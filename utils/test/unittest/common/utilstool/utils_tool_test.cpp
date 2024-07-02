/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "dh_utils_hitrace.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr int32_t UUID_LENGTH = 257;
}
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
 * @tc.name: GetAnonyString_001
 * @tc.desc: Verify the GetAnonyString function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(UtilsToolTest, GetAnonyString_001, TestSize.Level0)
{
    std::string value = "";
    auto ret = GetAnonyString(value);
    EXPECT_EQ("", ret);

    value = "11";
    ret = GetAnonyString(value);
    EXPECT_EQ("******", ret);

    value = "123456789";
    ret = GetAnonyString(value);
    EXPECT_EQ("1******9", ret);

    value = "111222333444555666777888999";
    ret = GetAnonyString(value);
    EXPECT_EQ("1112******8999", ret);
}

/**
 * @tc.name: GetAnonyInt32_001
 * @tc.desc: Verify the GetAnnoyInt32 function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(UtilsToolTest, GetAnonyInt32_001, TestSize.Level0)
{
    int32_t value = 123456;
    auto ret = GetAnonyInt32(value);
    EXPECT_EQ("1*****", ret);
}

/**
 * @tc.name: GetUUIDByDm_001
 * @tc.desc: Verify the GetUUIDBySoftBus function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(UtilsToolTest, GetUUIDByDm_001, TestSize.Level0)
{
    std::string networkId = "";
    std::string ret = GetUUIDByDm(networkId);
    EXPECT_EQ(0, ret.size());
}

/**
 * @tc.name: GetDeviceIdByUUID_001
 * @tc.desc: Verify the GetDeviceIdByUUID function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(UtilsToolTest, GetDeviceIdByUUID_001, TestSize.Level0)
{
    std::string uuidEmpty = "";
    std::string ret = GetDeviceIdByUUID(uuidEmpty);
    EXPECT_EQ(0, ret.size());

    std::string uuid(UUID_LENGTH, '1');
    ret = GetDeviceIdByUUID(uuid);
    EXPECT_EQ(0, ret.size());
}

/**
 * @tc.name: GetDeviceIdByUUID_002
 * @tc.desc: Verify the GetDeviceIdByUUID function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(UtilsToolTest, GetDeviceIdByUUID_002, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    DHQueryTraceStart(dhType);

    std::string uuid = "bb536a637105409e904d4da78290ab1";
    std::string ret = GetDeviceIdByUUID(uuid);
    EXPECT_NE(0, ret.size());
}
} // namespace DistributedHardware
} // namespace OHOS
