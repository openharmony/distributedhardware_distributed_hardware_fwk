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

#include "gtest/gtest.h"

#include "constants.h"
#include "dh_context.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {
const std::string TEST_NETWORKID = "111111";
const std::string TEST_UUID = "222222";
const std::string TEST_UDID = "333333";
}

class DhContextTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DhContextTest::SetUp() {}

void DhContextTest::TearDown() {}

void DhContextTest::SetUpTestCase() {}

void DhContextTest::TearDownTestCase() {}

HWTEST_F(DhContextTest, AddOnlineDevice_001, TestSize.Level1)
{
    DHContext::GetInstance().AddOnlineDevice("", "", "");
    EXPECT_EQ(true, DHContext::GetInstance().devIdEntrySet_.empty());

    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, "", "");
    EXPECT_EQ(true, DHContext::GetInstance().devIdEntrySet_.empty());

    DHContext::GetInstance().AddOnlineDevice("", TEST_UUID, "");
    EXPECT_EQ(true, DHContext::GetInstance().devIdEntrySet_.empty());

    DHContext::GetInstance().AddOnlineDevice("", "", TEST_NETWORKID);
    EXPECT_EQ(true, DHContext::GetInstance().devIdEntrySet_.empty());

    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, TEST_UUID, "");
    EXPECT_EQ(true, DHContext::GetInstance().devIdEntrySet_.empty());

    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, "", TEST_NETWORKID);
    EXPECT_EQ(true, DHContext::GetInstance().devIdEntrySet_.empty());

    DHContext::GetInstance().AddOnlineDevice("", TEST_UUID, TEST_NETWORKID);
    EXPECT_EQ(true, DHContext::GetInstance().devIdEntrySet_.empty());
}

HWTEST_F(DhContextTest, AddOnlineDevice_002, TestSize.Level1)
{
    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, TEST_UUID, TEST_NETWORKID);
    EXPECT_EQ(false, DHContext::GetInstance().devIdEntrySet_.empty());
}

HWTEST_F(DhContextTest, RemoveOnlineDeviceByUUID_001, TestSize.Level1)
{
    DHContext::GetInstance().RemoveOnlineDeviceByUUID("123");
    EXPECT_EQ(false, DHContext::GetInstance().devIdEntrySet_.empty());

    DHContext::GetInstance().RemoveOnlineDeviceByUUID(TEST_UUID);
    EXPECT_EQ(true, DHContext::GetInstance().devIdEntrySet_.empty());
}

HWTEST_F(DhContextTest, IsDeviceOnline_001, TestSize.Level1)
{
    bool ret = DHContext::GetInstance().IsDeviceOnline(TEST_UUID);
    EXPECT_EQ(false, ret);

    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, TEST_UUID, TEST_NETWORKID);
    ret = DHContext::GetInstance().IsDeviceOnline(TEST_UUID);
    EXPECT_EQ(true, ret);
}

HWTEST_F(DhContextTest, GetNetworkIdByUUID_001, TestSize.Level1)
{
    auto ret = DHContext::GetInstance().GetNetworkIdByUUID(TEST_UUID);
    EXPECT_EQ(TEST_NETWORKID, ret);

    DHContext::GetInstance().devIdEntrySet_.clear();
    ret = DHContext::GetInstance().GetNetworkIdByUUID(TEST_UUID);
    EXPECT_EQ("", ret);
}

HWTEST_F(DhContextTest, GetUdidHashIdByUUID_001, TestSize.Level1)
{
    auto ret = DHContext::GetInstance().GetUdidHashIdByUUID(TEST_UUID);
    EXPECT_EQ("", ret);

    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, TEST_UUID, TEST_NETWORKID);
    ret = DHContext::GetInstance().GetUdidHashIdByUUID(TEST_UUID);
    EXPECT_NE("", ret);
}

HWTEST_F(DhContextTest, GetUUIDByNetworkId_001, TestSize.Level1)
{
    auto ret = DHContext::GetInstance().GetUUIDByNetworkId(TEST_NETWORKID);
    EXPECT_EQ(TEST_UUID, ret);

    DHContext::GetInstance().devIdEntrySet_.clear();
    ret = DHContext::GetInstance().GetUUIDByNetworkId(TEST_NETWORKID);
    EXPECT_EQ("", ret);
}

HWTEST_F(DhContextTest, GetUDIDByNetworkId_001, TestSize.Level1)
{
    auto ret = DHContext::GetInstance().GetUDIDByNetworkId(TEST_NETWORKID);
    EXPECT_EQ("", ret);

    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, TEST_UUID, TEST_NETWORKID);
    ret = DHContext::GetInstance().GetUDIDByNetworkId(TEST_NETWORKID);
    EXPECT_EQ(TEST_UDID, ret);
}

HWTEST_F(DhContextTest, GetUUIDByDeviceId_001, TestSize.Level1)
{
    std::string deviceId = Sha256(TEST_UUID);
    auto ret = DHContext::GetInstance().GetUUIDByDeviceId(deviceId);
    EXPECT_EQ(TEST_UUID, ret);

    DHContext::GetInstance().devIdEntrySet_.clear();
    ret = DHContext::GetInstance().GetUUIDByDeviceId(deviceId);
    EXPECT_EQ("", ret);
}

HWTEST_F(DhContextTest, GetNetworkIdByDeviceId_001, TestSize.Level1)
{
    std::string deviceId = Sha256(TEST_UUID);
    auto ret = DHContext::GetInstance().GetNetworkIdByDeviceId(deviceId);
    EXPECT_EQ("", ret);

    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, TEST_UUID, TEST_NETWORKID);
    ret = DHContext::GetInstance().GetNetworkIdByDeviceId(deviceId);
    EXPECT_EQ(TEST_NETWORKID, ret);
}

HWTEST_F(DhContextTest, GetDeviceIdByDBGetPrefix_001, TestSize.Level1)
{
    std::string prefix = "";
    auto ret = DHContext::GetInstance().GetDeviceIdByDBGetPrefix(prefix);
    EXPECT_EQ("", ret);

    prefix = "prefix_test";
    ret = DHContext::GetInstance().GetDeviceIdByDBGetPrefix(prefix);
    EXPECT_EQ(prefix, ret);

    prefix = "prefix" + RESOURCE_SEPARATOR;
    ret = DHContext::GetInstance().GetDeviceIdByDBGetPrefix(prefix);
    EXPECT_EQ("prefix", ret);
}
}
}