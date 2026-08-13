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
#include "device_manager.h"
#include "device_manager_impl.h"

#include "device_param_mgr.h"
#include "dh_context.h"
#include "distributed_hardware_manager_factory.h"
#include "distributed_hardware_errno.h"
#include "mock_device_param_mgr.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace DistributedHardware {
namespace {
enum class Status : uint32_t {
    DEVICE_ONLINE = 0,
    DEVICE_OFFLINE = 1,
};

constexpr uint16_t TEST_DEV_TYPE_PAD = 0x11;
constexpr int32_t INVALID_OSTYPE = 10;
constexpr int32_t OLD_HO_DEVICE_TYPE = -1;
constexpr int32_t NEW_HO_DEVICE_TYPE = 11;
const std::string TEST_NETWORKID = "111111";
const std::string TEST_UUID = "222222";
const std::string TEST_UDID = "333333";
}

static int32_t g_InitDMValue = 0;
static int32_t g_UnInitDMValue = 0;
static int32_t g_RegisterDMValue = 0;
static int32_t g_UnRegisterDMValue = 0;

int32_t DeviceManagerImpl::InitDeviceManager(const std::string &pkgName,
    std::shared_ptr<DmInitCallback> dmInitCallback)
{
    return g_InitDMValue;
}

int32_t DeviceManagerImpl::UnInitDeviceManager(const std::string &pkgName)
{
    return g_UnInitDMValue;
}

int32_t DeviceManagerImpl::RegisterDevStateCallback(const std::string &pkgName,
    const std::string &extra, std::shared_ptr<DeviceStateCallback> callback)
{
    return g_RegisterDMValue;
}

int32_t DeviceManagerImpl::UnRegisterDevStateCallback(const std::string &pkgName)
{
    return g_UnRegisterDMValue;
}

class AccessManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
    std::shared_ptr<MockIDeviceParamMgr> deviceParamMgr_;
};

void AccessManagerTest::SetUp()
{
    DistributedHardwareManagerFactory::GetInstance().isInit_.store(true);
    g_InitDMValue = 0;
    g_UnInitDMValue = 0;
    g_RegisterDMValue = 0;
    g_UnRegisterDMValue = 0;
    auto deviceParamMgr = IDeviceParamMgr::GetOrCreateInstance();
    deviceParamMgr_ = std::static_pointer_cast<MockIDeviceParamMgr>(deviceParamMgr);
}

void AccessManagerTest::TearDown()
{
    IDeviceParamMgr::ReleaseInstance();
    deviceParamMgr_ = nullptr;
}

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
HWTEST_F(AccessManagerTest, Init_001, TestSize.Level1)
{
    g_InitDMValue = -1;
    auto ret = AccessManager::GetInstance()->Init();
    EXPECT_EQ(ERR_DH_FWK_ACCESS_INIT_DM_FAILED, ret);
}

HWTEST_F(AccessManagerTest, Init_002, TestSize.Level1)
{
    g_RegisterDMValue = -1;
    auto ret = AccessManager::GetInstance()->Init();
    EXPECT_EQ(ERR_DH_FWK_ACCESS_REGISTER_DM_FAILED, ret);
}

HWTEST_F(AccessManagerTest, Init_003, TestSize.Level1)
{
    g_RegisterDMValue = 0;
    auto ret = AccessManager::GetInstance()->Init();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: UnInit_001
 * @tc.desc: Verify the UnInit function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, UnInit_001, TestSize.Level1)
{
    g_UnInitDMValue = -1;
    auto ret = AccessManager::GetInstance()->UnInit();
    EXPECT_EQ(ERR_DH_FWK_ACCESS_UNINIT_DM_FAILED, ret);
}

HWTEST_F(AccessManagerTest, UnInit_002, TestSize.Level1)
{
    g_UnRegisterDMValue = -1;
    auto ret = AccessManager::GetInstance()->UnInit();
    EXPECT_EQ(ERR_DH_FWK_ACCESS_UNREGISTER_DM_FAILED, ret);
}

HWTEST_F(AccessManagerTest, UnInit_003, TestSize.Level1)
{
    g_UnRegisterDMValue = 0;
    auto ret = AccessManager::GetInstance()->UnInit();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: OnRemoteDied_001
 * @tc.desc: Verify the OnRemoteDied function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnRemoteDied_001, TestSize.Level1)
{
    g_InitDMValue = -1;
    ASSERT_NO_FATAL_FAILURE(AccessManager::GetInstance()->OnRemoteDied());

    g_InitDMValue = 0;
    ASSERT_NO_FATAL_FAILURE(AccessManager::GetInstance()->OnRemoteDied());
}

/**
 * @tc.name: OnDeviceOnline_001
 * @tc.desc: Verify the OnDeviceOnline function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnDeviceOnline_001, TestSize.Level1)
{
    DmDeviceInfo deviceInfo = {
        .deviceId = "123456789",
        .deviceName = "deviceName_test",
        .deviceTypeId = 1,
        .networkId = ""
    };
    ASSERT_NO_FATAL_FAILURE(AccessManager::GetInstance()->OnDeviceOnline(deviceInfo));
}

HWTEST_F(AccessManagerTest, OnDeviceOnline_002, TestSize.Level1)
{
    DmDeviceInfo deviceInfo = {
        .deviceId = "123456789",
        .deviceName = "deviceName_test",
        .deviceTypeId = 1,
        .networkId = "123456789"
    };
    ASSERT_NO_FATAL_FAILURE(AccessManager::GetInstance()->OnDeviceOnline(deviceInfo));
}

/**
 * @tc.name: OnDeviceOffline_001
 * @tc.desc: Verify the OnDeviceOffline function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnDeviceOffline_001, TestSize.Level1)
{
    DmDeviceInfo deviceInfo;
    ASSERT_NO_FATAL_FAILURE(AccessManager::GetInstance()->OnDeviceOffline(deviceInfo));
}

HWTEST_F(AccessManagerTest, OnDeviceOffline_002, TestSize.Level1)
{
    DmDeviceInfo deviceInfo = {
        .deviceId = "123456789",
        .deviceName = "deviceName_test",
        .deviceTypeId = 1,
        .networkId = "111111"
    };
    ASSERT_NO_FATAL_FAILURE(AccessManager::GetInstance()->OnDeviceOffline(deviceInfo));
}

HWTEST_F(AccessManagerTest, OnDeviceOffline_003, TestSize.Level1)
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
 * @tc.name: CheckExitSAOrNot_001
 * @tc.desc: Verify the CheckExitSAOrNot function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, CheckExitSAOrNot_001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(DistributedHardwareManagerFactory::GetInstance().CheckExitSAOrNot());
}

/**
 * @tc.name: InitLocalDevInfo_001
 * @tc.desc: Verify the InitLocalDevInfo function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, InitLocalDevInfo_001, TestSize.Level1)
{
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().InitLocalDevInfo());
}

HWTEST_F(AccessManagerTest, SendOnLineEvent_002, TestSize.Level1)
{
    std::string networkId = "networkId_1";
    std::string uuid = "uuid_1";
    std::string udid = "udid_1";
    uint16_t deviceType = TEST_DEV_TYPE_PAD;
    int32_t osType = OLD_HO_DEVICE_TYPE;
    DistributedHardwareManagerFactory::GetInstance().flagUnInit_.store(false);
    DistributedHardwareManagerFactory::GetInstance().isInit_.store(true);
    DeviceParamMgr::GetInstance().isDeviceE2ESync_.store(true);
    int32_t ret =
        DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent(networkId, uuid, udid, deviceType, osType);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(AccessManagerTest, SendOnLineEvent_003, TestSize.Level1)
{
    std::string networkId = "networkId_2";
    std::string uuid = "uuid_2";
    std::string udid = "udid_2";
    uint16_t deviceType = TEST_DEV_TYPE_PAD;
    int32_t osType = NEW_HO_DEVICE_TYPE;
    DistributedHardwareManagerFactory::GetInstance().flagUnInit_.store(false);
    DistributedHardwareManagerFactory::GetInstance().isInit_.store(true);
    DeviceParamMgr::GetInstance().isDeviceE2ESync_.store(true);
    int32_t ret =
        DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent(networkId, uuid, udid, deviceType, osType);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(AccessManagerTest, SendOnLineEvent_004, TestSize.Level1)
{
    std::string networkId = "networkId_3";
    std::string uuid = "uuid_3";
    std::string udid = "udid_3";
    uint16_t deviceType = TEST_DEV_TYPE_PAD;
    int32_t osType = INVALID_OSTYPE;
    DistributedHardwareManagerFactory::GetInstance().flagUnInit_.store(false);
    DistributedHardwareManagerFactory::GetInstance().isInit_.store(true);
    DeviceParamMgr::GetInstance().isDeviceE2ESync_.store(true);
    int32_t ret =
        DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent(networkId, uuid, udid, deviceType, osType);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(AccessManagerTest, OnDeviceReady_001, TestSize.Level1)
{
    DmDeviceInfo deviceInfo1 = {
        .deviceId = "123456789",
        .networkId = ""
    };
    ASSERT_NO_FATAL_FAILURE(AccessManager::GetInstance()->OnDeviceReady(deviceInfo1));

    DmDeviceInfo deviceInfo2 = {
        .deviceId = "123456789",
        .networkId = "123456789"
    };
    ASSERT_NO_FATAL_FAILURE(AccessManager::GetInstance()->OnDeviceReady(deviceInfo2));
}

HWTEST_F(AccessManagerTest, OnDeviceReady_002, TestSize.Level1)
{
    DmDeviceInfo deviceInfo = {
        .deviceId = "123456789",
        .networkId = "123456789"
    };
    EXPECT_CALL(*deviceParamMgr_, GetDeviceSyncDataMode()).WillRepeatedly(Return(false));
    ASSERT_NO_FATAL_FAILURE(AccessManager::GetInstance()->OnDeviceReady(deviceInfo));
}

HWTEST_F(AccessManagerTest, OnDeviceReady_003, TestSize.Level1)
{
    DmDeviceInfo deviceInfo1 = {
        .deviceId = "123456789",
        .networkId = "123456789",
        .extraData = "{\"OS_TYPE\": 10}"
    };
    EXPECT_CALL(*deviceParamMgr_, GetDeviceSyncDataMode()).WillRepeatedly(Return(true));
    ASSERT_NO_FATAL_FAILURE(AccessManager::GetInstance()->OnDeviceReady(deviceInfo1));

    DmDeviceInfo deviceInfo2 = {
        .deviceId = "123456789",
        .networkId = "123456789",
        .extraData = "{\"OS_TYPE\": 11}"
    };
    EXPECT_CALL(*deviceParamMgr_, GetDeviceSyncDataMode()).WillRepeatedly(Return(true));
    ASSERT_NO_FATAL_FAILURE(AccessManager::GetInstance()->OnDeviceReady(deviceInfo2));

    DmDeviceInfo deviceInfo3 = {
        .deviceId = "123456789",
        .networkId = "123456789",
        .extraData = "{\"OS_TYPE\": -1}"
    };
    EXPECT_CALL(*deviceParamMgr_, GetDeviceSyncDataMode()).WillRepeatedly(Return(true));
    ASSERT_NO_FATAL_FAILURE(AccessManager::GetInstance()->OnDeviceReady(deviceInfo3));
}
HWTEST_F(AccessManagerTest, OnDeviceReady_004, TestSize.Level1)
{
    DmDeviceInfo deviceInfo = {
        .deviceId = "123456789",
        .networkId = "123456789",
    };
    EXPECT_CALL(*deviceParamMgr_, GetDeviceSyncDataMode()).WillRepeatedly(Return(false));
    ASSERT_NO_FATAL_FAILURE(AccessManager::GetInstance()->OnDeviceReady(deviceInfo));
}
} // namespace DistributedHardware
} // namespace OHOS
