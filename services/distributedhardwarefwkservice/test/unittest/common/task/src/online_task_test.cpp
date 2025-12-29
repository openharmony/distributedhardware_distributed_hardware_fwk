/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_ONLINE_TASK_TEST_H
#define OHOS_DISTRIBUTED_ONLINE_TASK_TEST_H

#include <gtest/gtest.h>

#include "device_manager.h"
#include "dm_device_info.h"

#include "capability_info_manager.h"
#include "capability_info.h"
#include "dh_utils_tool.h"
#include "dh_context.h"
#include "local_capability_info_manager.h"
#include "meta_info_manager.h"
#include "meta_capability_info.h"
#include "mock_dh_utils_tool.h"
#include "online_task.h"
#include "offline_task.h"
#include "task.h"
#include "task_board.h"

using namespace testing::ext;
using namespace std;
using namespace testing;

namespace OHOS {
namespace DistributedHardware {
namespace {
    const std::string NETWORKID_TEST = "111111";
    const std::string UUID_TEST = "222222";
    const std::string UDID_TEST = "333333";
    const std::string DHID_TEST = "audio_1";
    const std::string DEV_NAME_TEST = "phone";
    const DHType DH_TYPE_TEST = DHType::AUDIO;
    const uint16_t DEV_TYPE_TEST = 14;
    constexpr int32_t NEW_HO_DEVICE_TYPE_TEST = 11;
    constexpr int32_t INVALID_OSTYPE = 0;
}

class OnlineTaskTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

private:
    std::shared_ptr<MockDhUtilTool> utilTool_;
};

void OnlineTaskTest::SetUpTestCase()
{
}

void OnlineTaskTest::TearDownTestCase()
{
}

void OnlineTaskTest::SetUp()
{
    auto utilTool = IDhUtilTool::GetOrCreateInstance();
    utilTool_ = std::static_pointer_cast<MockDhUtilTool>(utilTool);
}

void OnlineTaskTest::TearDown()
{
    IDhUtilTool::ReleaseInstance();
    utilTool_ = nullptr;
}

HWTEST_F(OnlineTaskTest, CreateEnableTask_001, TestSize.Level1)
{
    OnLineTask onlineTask(NETWORKID_TEST, UUID_TEST, UDID_TEST, DHID_TEST, DH_TYPE_TEST);
    std::string deviceId = Sha256(UUID_TEST);
    std::string udidHash = Sha256(UDID_TEST);
    std::string capinfoKey = deviceId + RESOURCE_SEPARATOR + DHID_TEST;
    std::string metaInfoKey = udidHash + RESOURCE_SEPARATOR + DHID_TEST;
    std::shared_ptr<CapabilityInfo> capPtr = nullptr;
    std::shared_ptr<MetaCapabilityInfo> metaCapPtr = nullptr;
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_[capinfoKey] = capPtr;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[capinfoKey] = capPtr;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[metaInfoKey] = metaCapPtr;
    ASSERT_NO_FATAL_FAILURE(onlineTask.CreateEnableTask());
}

#ifdef UT_COVER_SPECIAL
HWTEST_F(OnlineTaskTest, CreateEnableSinkTask_001, TestSize.Level1)
{
    OnLineTask onlineTask(NETWORKID_TEST, UUID_TEST, UDID_TEST, DHID_TEST, DH_TYPE_TEST);
    std::string deviceId = Sha256(UUID_TEST);
    std::string udidHash = Sha256(UDID_TEST);
    std::string metaInfoKey = udidHash + RESOURCE_SEPARATOR + DHID_TEST;
    std::shared_ptr<MetaCapabilityInfo> metaCapPtr = nullptr;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[metaInfoKey] = metaCapPtr;
    DeviceInfo deviceInfo(NETWORKID_TEST, UUID_TEST, deviceId, UDID_TEST, udidHash, DEV_NAME_TEST, DEV_TYPE_TEST);
    EXPECT_CALL(*utilTool_, GetLocalDeviceInfo()).WillRepeatedly(Return(deviceInfo));
    ASSERT_NO_FATAL_FAILURE(onlineTask.CreateEnableSinkTask());
}
#endif

HWTEST_F(OnlineTaskTest, CreateDisableTask_001, TestSize.Level1)
{
    OffLineTask offlineTask(NETWORKID_TEST, UUID_TEST, UDID_TEST, DHID_TEST, DH_TYPE_TEST);
    DHContext::GetInstance().AddOnlineDevice(UDID_TEST, UUID_TEST, NETWORKID_TEST);
    std::string deviceId = Sha256(UUID_TEST);
    std::string udidHash = Sha256(UDID_TEST);
    std::string capinfoKey = deviceId + RESOURCE_SEPARATOR + DHID_TEST;
    std::string metaInfoKey = udidHash + RESOURCE_SEPARATOR + DHID_TEST;
    std::shared_ptr<CapabilityInfo> capPtr = nullptr;
    std::shared_ptr<MetaCapabilityInfo> metaCapPtr = nullptr;
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_[capinfoKey] = capPtr;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[metaInfoKey] = metaCapPtr;
    ASSERT_NO_FATAL_FAILURE(offlineTask.CreateDisableTask());
}

#ifdef UT_COVER_SPECIAL
HWTEST_F(OnlineTaskTest, CreateDisableSinkTask_001, TestSize.Level1)
{
    OffLineTask offlineTask(NETWORKID_TEST, UUID_TEST, UDID_TEST, DHID_TEST, DH_TYPE_TEST);
    std::string deviceId = Sha256(UUID_TEST);
    std::string udidHash = Sha256(UDID_TEST);
    std::string metaInfoKey = udidHash + RESOURCE_SEPARATOR + DHID_TEST;
    std::shared_ptr<MetaCapabilityInfo> metaCapPtr = nullptr;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[metaInfoKey] = metaCapPtr;
    DeviceInfo deviceInfo(NETWORKID_TEST, UUID_TEST, deviceId, UDID_TEST, udidHash, DEV_NAME_TEST, DEV_TYPE_TEST);
    EXPECT_CALL(*utilTool_, GetLocalDeviceInfo()).WillRepeatedly(Return(deviceInfo));
    ASSERT_NO_FATAL_FAILURE(offlineTask.CreateDisableSinkTask());
}
#endif

HWTEST_F(OnlineTaskTest, DumpAllTasks_001, TestSize.Level1)
{
    std::vector<TaskDump> taskInfos;
    std::string taskId = "task_001";
    std::shared_ptr<Task> task = nullptr;
    TaskBoard::GetInstance().tasks_.emplace(taskId, task);
    TaskBoard::GetInstance().DumpAllTasks(taskInfos);
    ASSERT_TRUE(taskInfos.empty());
    TaskBoard::GetInstance().tasks_.clear();
}

HWTEST_F(OnlineTaskTest, IsAllDisableTaskFinish_001, TestSize.Level1)
{
    std::string taskId = "task_001";
    std::shared_ptr<Task> task = nullptr;
    TaskBoard::GetInstance().tasks_.emplace(taskId, task);
    auto ret = TaskBoard::GetInstance().IsAllDisableTaskFinish();
    EXPECT_EQ(ret, true);
    TaskBoard::GetInstance().tasks_.clear();
}

HWTEST_F(OnlineTaskTest, IsAllDisableTaskFinish_002, TestSize.Level1)
{
    std::string taskId = "task_002";
    std::shared_ptr<Task> onlineTask = std::make_shared<OnLineTask>("networkId", "uuid", "udid", "dhId", DHType::AUDIO);
    TaskBoard::GetInstance().tasks_.emplace(taskId, onlineTask);
    auto ret = TaskBoard::GetInstance().IsAllDisableTaskFinish();
    EXPECT_EQ(ret, true);
    TaskBoard::GetInstance().tasks_.clear();
}

HWTEST_F(OnlineTaskTest, CreateMetaEnableTask_001, TestSize.Level1)
{
    OnLineTask onlineTask(NETWORKID_TEST, UUID_TEST, UDID_TEST, DHID_TEST, DH_TYPE_TEST);
    ASSERT_NO_FATAL_FAILURE(onlineTask.CreateMetaEnableTask());
}

HWTEST_F(OnlineTaskTest, CreateMetaEnableTask_002, TestSize.Level1)
{
    OnLineTask onlineTask(NETWORKID_TEST, UUID_TEST, UDID_TEST, DHID_TEST, DH_TYPE_TEST);
    DHContext::GetInstance().AddOnlineDeviceOSType(NETWORKID_TEST, NEW_HO_DEVICE_TYPE_TEST);
    ASSERT_NO_FATAL_FAILURE(onlineTask.CreateMetaEnableTask());
}

HWTEST_F(OnlineTaskTest, CreateMetaDisableTask_001, TestSize.Level1)
{
    OffLineTask offlineTask(NETWORKID_TEST, UUID_TEST, UDID_TEST, DHID_TEST, DH_TYPE_TEST);
    ASSERT_NO_FATAL_FAILURE(offlineTask.CreateMetaDisableTask());
}

HWTEST_F(OnlineTaskTest, CreateMetaDisableTask_002, TestSize.Level1)
{
    OffLineTask offlineTask(NETWORKID_TEST, UUID_TEST, UDID_TEST, DHID_TEST, DH_TYPE_TEST);
    DHContext::GetInstance().AddOnlineDeviceOSType(NETWORKID_TEST, NEW_HO_DEVICE_TYPE_TEST);
    ASSERT_NO_FATAL_FAILURE(offlineTask.CreateMetaDisableTask());
}

HWTEST_F(OnlineTaskTest, NotifyFatherFinish_001, TestSize.Level1)
{
    OffLineTask offlineTask(NETWORKID_TEST, UUID_TEST, UDID_TEST, DHID_TEST, DH_TYPE_TEST);
    std::string taskId = "offline_task_1";
    std::string disabletaskId = "disable_1";
    offlineTask.unFinishChildrenTasks_.insert(taskId);
    offlineTask.unFinishChildrenTasks_.insert(disabletaskId);
    ASSERT_NO_FATAL_FAILURE(offlineTask.NotifyFatherFinish(disabletaskId));

    offlineTask.unFinishChildrenTasks_.erase(taskId);
    ASSERT_NO_FATAL_FAILURE(offlineTask.NotifyFatherFinish(taskId));
}

HWTEST_F(OnlineTaskTest, CreateDisableTask_002, TestSize.Level1)
{
    OffLineTask offlineTask(NETWORKID_TEST, UUID_TEST, UDID_TEST, DHID_TEST, DH_TYPE_TEST);
    ASSERT_NO_FATAL_FAILURE(offlineTask.CreateDisableTask());

    std::shared_ptr<CapabilityInfo> localCapInfo = nullptr;
    std::string deviceId = Sha256(UUID_TEST);
    std::string key = deviceId + "###" + DHID_TEST;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = localCapInfo;
    ASSERT_NO_FATAL_FAILURE(offlineTask.CreateDisableTask());
}

HWTEST_F(OnlineTaskTest, DoSyncInfo_001, TestSize.Level1)
{
    OnLineTask onlineTask(NETWORKID_TEST, UUID_TEST, UDID_TEST, DHID_TEST, DH_TYPE_TEST);
    MetaInfoManager::GetInstance()->eventHandler_ = nullptr;
    ASSERT_NO_FATAL_FAILURE(onlineTask.DoSyncInfo());
}

HWTEST_F(OnlineTaskTest, DoSyncInfo_002, TestSize.Level1)
{
    OnLineTask onlineTask(NETWORKID_TEST, UUID_TEST, UDID_TEST, DHID_TEST, DH_TYPE_TEST);
    MetaInfoManager::GetInstance()->Init();
    ASSERT_NO_FATAL_FAILURE(onlineTask.DoSyncInfo());
}
} // namespace DistributedHardware
} // namespace OHOS
#endif