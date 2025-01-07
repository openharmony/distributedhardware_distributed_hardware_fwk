/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "access_manager.h"

#include "gtest/gtest.h"

#include "dm_device_info.h"

#include "dh_context.h"
#include "distributed_hardware_manager_factory.h"
#include "distributed_hardware_errno.h"
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {
enum class Status : uint32_t {
    DEVICE_ONLINE = 0,
    DEVICE_OFFLINE = 1,
};

constexpr uint16_t TEST_DEV_TYPE_PAD = 0x11;
constexpr int32_t INVALID_OSTYPE = 10;
const std::string TEST_NETWORKID = "111111";
const std::string TEST_UUID = "222222";
const std::string TEST_UDID = "333333";
}

class AccessManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AccessManagerTest::SetUp()
{
    DistributedHardwareManagerFactory::GetInstance().isInit_.store(true);
}

void AccessManagerTest::TearDown() {}

void AccessManagerTest::SetUpTestCase() {}

void AccessManagerTest::TearDownTestCase() {}

/**
 * @tc.name: SendOnLineEvent_001
 * @tc.desc: Verify the online success
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AccessManagerTest, SendOnLineEvent_001, TestSize.Level1)
{
    int32_t ostype = -1;
    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, TEST_UUID, TEST_NETWORKID);
    auto ret = DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent(TEST_NETWORKID, TEST_UUID, TEST_UDID,
        TEST_DEV_TYPE_PAD, ostype);
    EXPECT_EQ(ERR_DH_FWK_HARDWARE_MANAGER_DEVICE_REPEAT_ONLINE, ret);

    DHContext::GetInstance().devIdEntrySet_.clear();
    ret = DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent(TEST_NETWORKID, TEST_UUID, TEST_UDID,
        TEST_DEV_TYPE_PAD, ostype);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    DHContext::GetInstance().devIdEntrySet_.clear();
    ret = DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent(TEST_NETWORKID, TEST_UUID, TEST_UDID,
        TEST_DEV_TYPE_PAD, INVALID_OSTYPE);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: SendOffLineEvent_001
 * @tc.desc: Verify the offline success
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, SendOffLineEvent_001, TestSize.Level1)
{
    DHContext::GetInstance().devIdEntrySet_.clear();
    auto ret = DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent("", TEST_UUID, TEST_UDID,
        TEST_DEV_TYPE_PAD);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
    ret = DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent(TEST_NETWORKID, "", TEST_UDID,
        TEST_DEV_TYPE_PAD);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
    ret = DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent(TEST_NETWORKID, TEST_UUID, "",
        TEST_DEV_TYPE_PAD);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, TEST_UUID, TEST_NETWORKID);
    ret = DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent(TEST_NETWORKID, TEST_UUID, TEST_UDID,
        TEST_DEV_TYPE_PAD);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    DHContext::GetInstance().devIdEntrySet_.clear();
}

/**
 * @tc.name: Init_001
 * @tc.desc: Verify the Init function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, Init_001, TestSize.Level0)
{
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
}

/**
 * @tc.name: OnRemoteDied_001
 * @tc.desc: Verify the OnRemoteDied function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnRemoteDied_001, TestSize.Level0)
{
    AccessManager::GetInstance()->OnRemoteDied();
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
}

/**
 * @tc.name: OnDeviceOnline_001
 * @tc.desc: Verify the OnDeviceOnline function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnDeviceOnline_001, TestSize.Level0)
{
    DmDeviceInfo deviceInfo = {
        .deviceId = "123456789",
        .deviceName = "deviceName_test",
        .deviceTypeId = 1,
        .networkId = ""
    };
    AccessManager::GetInstance()->OnDeviceOnline(deviceInfo);
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
}

HWTEST_F(AccessManagerTest, OnDeviceOnline_002, TestSize.Level0)
{
    DmDeviceInfo deviceInfo = {
        .deviceId = "123456789",
        .deviceName = "deviceName_test",
        .deviceTypeId = 1,
        .networkId = "123456789"
    };
    AccessManager::GetInstance()->OnDeviceOnline(deviceInfo);
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
}

/**
 * @tc.name: OnDeviceOffline_001
 * @tc.desc: Verify the OnDeviceOffline function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnDeviceOffline_001, TestSize.Level0)
{
    DmDeviceInfo deviceInfo;
    ASSERT_NO_FATAL_FAILURE(AccessManager::GetInstance()->OnDeviceOffline(deviceInfo));
}

HWTEST_F(AccessManagerTest, OnDeviceOffline_002, TestSize.Level0)
{
    DmDeviceInfo deviceInfo = {
        .deviceId = "123456789",
        .deviceName = "deviceName_test",
        .deviceTypeId = 1,
        .networkId = "111111"
    };
    ASSERT_NO_FATAL_FAILURE(AccessManager::GetInstance()->OnDeviceOffline(deviceInfo));
}

HWTEST_F(AccessManagerTest, OnDeviceOffline_003, TestSize.Level0)
{
    DmDeviceInfo deviceInfo = {
        .deviceId = "123456789",
        .deviceName = "deviceName_test",
        .deviceTypeId = 1,
        .networkId = "111111"
    };
    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, "", TEST_NETWORKID);
    ASSERT_NO_FATAL_FAILURE(AccessManager::GetInstance()->OnDeviceOffline(deviceInfo));
    DHContext::GetInstance().RemoveOnlineDeviceIdEntryByNetworkId(TEST_NETWORKID);

    DHContext::GetInstance().AddOnlineDevice("", TEST_UUID, TEST_NETWORKID);
    ASSERT_NO_FATAL_FAILURE(AccessManager::GetInstance()->OnDeviceOffline(deviceInfo));
    DHContext::GetInstance().RemoveOnlineDeviceIdEntryByNetworkId(TEST_NETWORKID);

    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, TEST_UUID, TEST_NETWORKID);
    ASSERT_NO_FATAL_FAILURE(AccessManager::GetInstance()->OnDeviceOffline(deviceInfo));
    DHContext::GetInstance().RemoveOnlineDeviceIdEntryByNetworkId(TEST_NETWORKID);
}

/**
 * @tc.name: UnInit_001
 * @tc.desc: Verify the UnInit function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, UnInit_001, TestSize.Level0)
{
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->UnInit());
}

/**
 * @tc.name: CheckExitSAOrNot_001
 * @tc.desc: Verify the CheckExitSAOrNot function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, CheckExitSAOrNot_001, TestSize.Level0)
{
    ASSERT_NO_FATAL_FAILURE(DistributedHardwareManagerFactory::GetInstance().CheckExitSAOrNot());
}

/**
 * @tc.name: InitLocalDevInfo_001
 * @tc.desc: Verify the InitLocalDevInfo function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, InitLocalDevInfo_001, TestSize.Level0)
{
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().InitLocalDevInfo());
}
} // namespace DistributedHardware
} // namespace OHOS
