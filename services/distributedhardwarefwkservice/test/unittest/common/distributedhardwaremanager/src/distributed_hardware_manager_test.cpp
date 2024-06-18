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

#include "distributed_hardware_manager_test.h"

#include <chrono>
#include <thread>
#include <vector>

#include "constants.h"
#include "dh_context.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "distributed_hardware_manager.h"
#include "task_board.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
/* save networkId and uuid */
const std::vector<std::pair<std::string, std::string>> TEST_DEVICES = {
    { "11111111111111111111111111111111", "22222222222222222222222222222222" },
    { "33333333333333333333333333333333", "44444444444444444444444444444444" },
};
constexpr uint16_t TEST_DEV_TYPE_PAD = 0x11;

void DistributedHardwareManagerTest::SetUpTestCase(void) {}

void DistributedHardwareManagerTest::TearDownTestCase(void) {}

void DistributedHardwareManagerTest::SetUp()
{
    for (auto device : TEST_DEVICES) {
        DHContext::GetInstance().RemoveOnlineDeviceByUUID(device.second);
    }
}

void DistributedHardwareManagerTest::TearDown() {}

/**
 * @tc.name: sendOnLineEvent_test_001
 * @tc.desc: Verify the SendOnLineEvent success
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareManagerTest, sendOnLineEvent_test_001, TestSize.Level0)
{
    auto onlineResult = DistributedHardwareManager::GetInstance().SendOnLineEvent(TEST_DEVICES[0].first,
        TEST_DEVICES[0].second, TEST_DEVICES[0].second, TEST_DEV_TYPE_PAD);
    ASSERT_EQ(DH_FWK_SUCCESS, onlineResult);

    ASSERT_EQ(DH_FWK_SUCCESS, TaskBoard::GetInstance().WaitForALLTaskFinish());
    ASSERT_TRUE(TaskBoard::GetInstance().IsAllTaskFinish());
}

/**
 * @tc.name: sendOnLineEvent_test_002
 * @tc.desc: Verify the SendOnLineEvent repeate online
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareManagerTest, sendOnLineEvent_test_002, TestSize.Level0)
{
    auto onlineResult = DistributedHardwareManager::GetInstance().SendOnLineEvent(TEST_DEVICES[0].first,
        TEST_DEVICES[0].second, TEST_DEVICES[0].second, TEST_DEV_TYPE_PAD);
    ASSERT_EQ(DH_FWK_SUCCESS, onlineResult);

    onlineResult = DistributedHardwareManager::GetInstance().SendOnLineEvent(TEST_DEVICES[0].first,
        TEST_DEVICES[0].second, TEST_DEVICES[0].second, TEST_DEV_TYPE_PAD);
    ASSERT_EQ(DH_FWK_SUCCESS, onlineResult);

    ASSERT_EQ(DH_FWK_SUCCESS, TaskBoard::GetInstance().WaitForALLTaskFinish());
    ASSERT_TRUE(TaskBoard::GetInstance().IsAllTaskFinish());
}

/**
 * @tc.name: sendOnLineEvent_test_003
 * @tc.desc: Verify the SendOnLineEvent failed for uuid is empty
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareManagerTest, sendOnLineEvent_test_003, TestSize.Level0)
{
    auto onlineResult = DistributedHardwareManager::GetInstance().SendOnLineEvent("", "", "", TEST_DEV_TYPE_PAD);
    ASSERT_EQ(ERR_DH_FWK_PARA_INVALID, onlineResult);

    onlineResult =
        DistributedHardwareManager::GetInstance().SendOnLineEvent(TEST_DEVICES[0].first, "", "", TEST_DEV_TYPE_PAD);
    ASSERT_EQ(ERR_DH_FWK_PARA_INVALID, onlineResult);

    ASSERT_EQ(DH_FWK_SUCCESS, TaskBoard::GetInstance().WaitForALLTaskFinish());
    ASSERT_TRUE(TaskBoard::GetInstance().IsAllTaskFinish());
}

/**
 * @tc.name: sendOffLineEvent_test_001
 * @tc.desc: Verify the SendOffLineEvent success
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareManagerTest, sendOffLineEvent_test_001, TestSize.Level0)
{
    auto onlineResult = DistributedHardwareManager::GetInstance().SendOnLineEvent(TEST_DEVICES[0].first,
        TEST_DEVICES[0].second, TEST_DEVICES[0].second, TEST_DEV_TYPE_PAD);
    ASSERT_EQ(DH_FWK_SUCCESS, onlineResult);

    auto offlineResult = DistributedHardwareManager::GetInstance().SendOffLineEvent(TEST_DEVICES[0].first,
        TEST_DEVICES[0].second, TEST_DEVICES[0].second, TEST_DEV_TYPE_PAD);
    ASSERT_EQ(DH_FWK_SUCCESS, offlineResult);

    ASSERT_EQ(DH_FWK_SUCCESS, TaskBoard::GetInstance().WaitForALLTaskFinish());
    ASSERT_TRUE(TaskBoard::GetInstance().IsAllTaskFinish());
}

/**
 * @tc.name: sendOffLineEvent_test_002
 * @tc.desc: Verify the SendOffLineEvent repeat
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareManagerTest, sendOffLineEvent_test_002, TestSize.Level0)
{
    auto onlineResult = DistributedHardwareManager::GetInstance().SendOnLineEvent(TEST_DEVICES[0].first,
        TEST_DEVICES[0].second, TEST_DEVICES[0].second, TEST_DEV_TYPE_PAD);
    ASSERT_EQ(DH_FWK_SUCCESS, onlineResult);

    auto offlineResult = DistributedHardwareManager::GetInstance().SendOffLineEvent(TEST_DEVICES[0].first,
        TEST_DEVICES[0].second, TEST_DEVICES[0].second, TEST_DEV_TYPE_PAD);
    ASSERT_EQ(DH_FWK_SUCCESS, offlineResult);

    offlineResult = DistributedHardwareManager::GetInstance().SendOffLineEvent(TEST_DEVICES[0].first,
        TEST_DEVICES[0].second, TEST_DEVICES[0].second, TEST_DEV_TYPE_PAD);
    ASSERT_EQ(DH_FWK_SUCCESS, offlineResult);

    ASSERT_EQ(DH_FWK_SUCCESS, TaskBoard::GetInstance().WaitForALLTaskFinish());
    ASSERT_TRUE(TaskBoard::GetInstance().IsAllTaskFinish());
}

/**
 * @tc.name: sendOffLineEvent_test_003
 * @tc.desc: Verify the SendOffLineEvent failed for uuid is empty
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareManagerTest, sendOffLineEvent_test_003, TestSize.Level0)
{
    auto offlineResult =
        DistributedHardwareManager::GetInstance().SendOffLineEvent("", "", "", TEST_DEV_TYPE_PAD);
    ASSERT_EQ(ERR_DH_FWK_PARA_INVALID, offlineResult);

    offlineResult =
        DistributedHardwareManager::GetInstance().SendOffLineEvent(TEST_DEVICES[0].first, "", "", TEST_DEV_TYPE_PAD);
    ASSERT_EQ(ERR_DH_FWK_PARA_INVALID, offlineResult);

    ASSERT_EQ(DH_FWK_SUCCESS, TaskBoard::GetInstance().WaitForALLTaskFinish());
    ASSERT_TRUE(TaskBoard::GetInstance().IsAllTaskFinish());
}

/**
 * @tc.name: Release_001
 * @tc.desc: Verify the Release function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareManagerTest, Release_001, TestSize.Level0)
{
    EXPECT_EQ(DH_FWK_SUCCESS, DistributedHardwareManager::GetInstance().Release());
}

/**
 * @tc.name: GetComponentVersion_001
 * @tc.desc: Verify the GetComponentVersion function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareManagerTest, RGetComponentVersion_001, TestSize.Level0)
{
    std::unordered_map<DHType, std::string> versionMap;
    EXPECT_NE(DH_FWK_SUCCESS, DistributedHardwareManager::GetInstance().GetComponentVersion(versionMap));
}

/**
 * @tc.name: SendOnLineEvent_001
 * @tc.desc: Verify the SendOnLineEvent function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareManagerTest, SendOnLineEvent_001, TestSize.Level0)
{
    uint32_t MAX_ID_LEN = 257;
    std::string networkId1;
    std::string networkId2;
    networkId2.resize(MAX_ID_LEN);

    std::string networkId3 = "networkId";
    std::string uuid1;
    std::string uuid2;
    uuid2.resize(MAX_ID_LEN);
    std::string uuid3 = "uuid3";

    uint16_t deviceType = 0;
    int32_t ret1 =  DistributedHardwareManager::GetInstance().SendOnLineEvent(networkId1, uuid3, uuid3, deviceType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret1);
    int32_t ret2 =  DistributedHardwareManager::GetInstance().SendOnLineEvent(networkId2, uuid3, uuid3, deviceType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret2);
    int32_t ret3 =  DistributedHardwareManager::GetInstance().SendOnLineEvent(networkId3, uuid3, uuid3, deviceType);
    EXPECT_EQ(DH_FWK_SUCCESS, ret3);
    int32_t ret4 =  DistributedHardwareManager::GetInstance().SendOnLineEvent(networkId3, uuid1, uuid1, deviceType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret4);
    int32_t ret5 =  DistributedHardwareManager::GetInstance().SendOnLineEvent(networkId3, uuid2, uuid2, deviceType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret5);
}

/**
 * @tc.name: SendOffLineEvent_001
 * @tc.desc: Verify the SendOffLineEvent function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareManagerTest, SendOffLineEvent_001, TestSize.Level0)
{
    uint32_t MAX_ID_LEN = 257;
    std::string networkId1;
    std::string networkId2;
    networkId2.resize(MAX_ID_LEN);

    std::string networkId3 = "networkId";
    std::string uuid1;
    std::string uuid2;
    uuid2.resize(MAX_ID_LEN);
    std::string uuid3 = "uuid3";

    uint16_t deviceType = 0;
    int32_t ret1 =  DistributedHardwareManager::GetInstance().SendOffLineEvent(networkId1, uuid3, uuid3, deviceType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret1);
    int32_t ret2 =  DistributedHardwareManager::GetInstance().SendOffLineEvent(networkId2, uuid3, uuid3, deviceType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret2);
    int32_t ret3 =  DistributedHardwareManager::GetInstance().SendOffLineEvent(networkId3, uuid3, uuid3, deviceType);
    EXPECT_EQ(DH_FWK_SUCCESS, ret3);
    int32_t ret4 =  DistributedHardwareManager::GetInstance().SendOffLineEvent(networkId3, uuid1, uuid3, deviceType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret4);
    int32_t ret5 =  DistributedHardwareManager::GetInstance().SendOffLineEvent(networkId3, uuid2, uuid3, deviceType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret5);
}

/**
 * @tc.name: LocalInit_001
 * @tc.desc: Verify the LocalInit function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareManagerTest, LocalInit_001, TestSize.Level0)
{
    int32_t ret =  DistributedHardwareManager::GetInstance().LocalInit();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
