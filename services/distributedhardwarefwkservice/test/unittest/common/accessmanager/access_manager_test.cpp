/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <algorithm>
#include <chrono>
#include <securec.h>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

#define private public
#include "access_manager.h"
#include "distributed_hardware_manager_factory.h"
#undef private
#include "dm_device_info.h"
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

/* save networkId and uuid */
const std::vector<std::pair<std::string, std::string>> TEST_DEVICES = {
    { "11111111111111111111111111111111", "22222222222222222222222222222222" },
    { "33333333333333333333333333333333", "44444444444444444444444444444444" },
    { "55555555555555555555555555555555", "66666666666666666666666666666666" },
    { "77777777777777777777777777777777", "88888888888888888888888888888888" },
};
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
    DistributedHardwareManagerFactory::GetInstance().isInit = true;
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
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());

    for (const auto &dev : TEST_DEVICES) {
        auto ret =
            DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent(dev.first, dev.second, TEST_DEV_TYPE_PAD);
        ASSERT_EQ(DH_FWK_SUCCESS, ret);
        ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());
    }
}

/**
 * @tc.name: SendOnLineEvent_002
 * @tc.desc: Verify the online failed
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AccessManagerTest, SendOnLineEvent_002, TestSize.Level1)
{
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());

    auto ret =
        DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent("", TEST_DEVICES[0].second, TEST_DEV_TYPE_PAD);
    ASSERT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());

    ret =
        DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent(TEST_DEVICES[0].first, "", TEST_DEV_TYPE_PAD);
    ASSERT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());
}

/**
 * @tc.name: SendOffLineEvent_001
 * @tc.desc: Verify the offline success
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, SendOffLineEvent_001, TestSize.Level1)
{
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());

    for (const auto &dev : TEST_DEVICES) {
        auto ret =
            DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent(dev.first, dev.second, TEST_DEV_TYPE_PAD);
        ASSERT_NE(DH_FWK_SUCCESS, ret);
        ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());
    }

    for (const auto &dev : TEST_DEVICES) {
        auto ret =
            DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent(dev.first, dev.second, TEST_DEV_TYPE_PAD);
        ASSERT_EQ(DH_FWK_SUCCESS, ret);
    }
    ASSERT_FALSE(DistributedHardwareManagerFactory::GetInstance().IsInit());
}

/**
 * @tc.name: SendOffLineEvent_002
 * @tc.desc: Verify the offline failed
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, SendOffLineEvent_002, TestSize.Level1)
{
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());

    auto ret = DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent("", TEST_DEVICES[0].second,
        TEST_DEV_TYPE_PAD);
    ASSERT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());

    ret =
        DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent(TEST_DEVICES[0].first, "", TEST_DEV_TYPE_PAD);
    ASSERT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());
}

/**
 * @tc.name: SendOffLineEvent_003
 * @tc.desc: Verify the SendOffLineEvent function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, SendOffLineEvent_003, TestSize.Level0)
{
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());
    uint32_t maxIdLen = 257;
    std::string networkId;
    std::string networkId1;
    networkId1.resize(maxIdLen);
    std::string networkId2 = "networkId3";
    std::string uuid;
    std::string uuid1;
    uuid1.resize(maxIdLen);
    std::string uuid2 = "uuid3";
    uint16_t deviceType = 1;
    int32_t ret = DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent(networkId, uuid, deviceType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    int32_t ret1 = DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent(networkId1, uuid, deviceType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret1);

    int32_t ret2 = DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent(networkId2, uuid, deviceType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret2);

    int32_t ret3 = DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent(networkId2, uuid1, deviceType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret3);

    int32_t ret4 = DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent(networkId2, uuid2, deviceType);
    EXPECT_EQ(ERR_DH_FWK_HARDWARE_MANAGER_DEVICE_REPEAT_OFFLINE, ret4);
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
    DmDeviceInfo deviceInfo;
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
    AccessManager::GetInstance()->OnDeviceOffline(deviceInfo);
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
}

/**
 * @tc.name: OnDeviceOffline_002
 * @tc.desc: Verify the OnDeviceOffline function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnDeviceOffline_002, TestSize.Level0)
{
    DmDeviceInfo deviceInfo;
    std::string devId = "000001";
    int32_t ret = memcpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, devId.c_str(), devId.length());
    if (ret != EOK) {
        return;
    }
    AccessManager::GetInstance()->OnDeviceOffline(deviceInfo);
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
}

/**
 * @tc.name: OnDeviceOffline_003
 * @tc.desc: Verify the OnDeviceOffline function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnDeviceOffline_003, TestSize.Level0)
{
    DmDeviceInfo deviceInfo;
    std::string devId = "000001";
    int32_t ret = memcpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, devId.c_str(), devId.length());
    if (ret != EOK) {
        return;
    }
    std::string netId = "000002";
    ret = memcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, netId.c_str(), netId.length());
    if (ret != EOK) {
        return;
    }
    AccessManager::GetInstance()->OnDeviceOffline(deviceInfo);
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
}

/**
 * @tc.name: OnDeviceReady_001
 * @tc.desc: Verify the OnDeviceReady function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnDeviceReady_001, TestSize.Level0)
{
    DmDeviceInfo deviceInfo;
    AccessManager::GetInstance()->OnDeviceReady(deviceInfo);
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
}

/**
 * @tc.name: OnDeviceReady_002
 * @tc.desc: Verify the OnDeviceReady function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnDeviceReady_002, TestSize.Level0)
{
    DmDeviceInfo deviceInfo;
    std::string devId = "000001";
    int32_t ret = memcpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, devId.c_str(), devId.length());
    if (ret != EOK) {
        return;
    }
    AccessManager::GetInstance()->OnDeviceReady(deviceInfo);
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
}

/**
 * @tc.name: OnDeviceReady_003
 * @tc.desc: Verify the OnDeviceReady function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnDeviceReady_003, TestSize.Level0)
{
    DmDeviceInfo deviceInfo;
    std::string devId = "000001";
    int32_t ret = memcpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, devId.c_str(), devId.length());
    if (ret != EOK) {
        return;
    }
    std::string netId = "000002";
    ret = memcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, netId.c_str(), netId.length());
    if (ret != EOK) {
        return;
    }
    AccessManager::GetInstance()->OnDeviceReady(deviceInfo);
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
}

/**
 * @tc.name: OnDeviceChanged_001
 * @tc.desc: Verify the OnDeviceChanged function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, OnDeviceChanged_001, TestSize.Level0)
{
    DmDeviceInfo deviceInfo;
    AccessManager::GetInstance()->OnDeviceChanged(deviceInfo);
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->Init());
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
 * @tc.name: UnInit_002
 * @tc.desc: Verify the Init function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, UnInit_002, TestSize.Level0)
{
    DistributedHardwareManagerFactory::GetInstance().UnInit();
    bool ret = DistributedHardwareManagerFactory::GetInstance().IsInit();
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: RegisterDevStateCallback_001
 * @tc.desc: Verify the RegisterDevStateCallback function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, RegisterDevStateCallback_001, TestSize.Level0)
{
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->RegisterDevStateCallback());
}

/**
 * @tc.name: UnRegisterDevStateCallback_001
 * @tc.desc: Verify the UnRegisterDevStateCallback function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, UnRegisterDevStateCallback_001, TestSize.Level0)
{
    EXPECT_EQ(DH_FWK_SUCCESS, AccessManager::GetInstance()->UnRegisterDevStateCallback());
}

/**
 * @tc.name: GetComponentVersion_001
 * @tc.desc: Verify the GetComponentVersion function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, GetComponentVersion_001, TestSize.Level0)
{
    std::unordered_map<DHType, std::string> versionMap;
    int32_t ret = DistributedHardwareManagerFactory::GetInstance().GetComponentVersion(versionMap);
    EXPECT_NE(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: CheckExitSAOrNot_001
 * @tc.desc: Verify the CheckExitSAOrNot function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(AccessManagerTest, CheckExitSAOrNot_001, TestSize.Level0)
{
    DistributedHardwareManagerFactory::GetInstance().CheckExitSAOrNot();
    ASSERT_TRUE(DistributedHardwareManagerFactory::GetInstance().IsInit());
}
} // namespace DistributedHardware
} // namespace OHOS
