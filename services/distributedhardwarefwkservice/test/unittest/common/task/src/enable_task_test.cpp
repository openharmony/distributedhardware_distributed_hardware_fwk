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

#include "enable_task_test.h"

#include "ffrt.h"

#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {

const std::string DEV_ID_1 = "bb536a637105409e904d4da83790a4a8";
const std::string DEV_NETWORK_ID_1 = "nt36a637105409e904d4da83790a4a8";
const std::string DEV_DID_1 = "2144a637105409e904d4da83790a4a8";

const TaskParam TASK_PARAM_1 = {
    .networkId = DEV_NETWORK_ID_1,
    .uuid = DEV_ID_1,
    .udid = DEV_DID_1,
    .dhId = "",
    .dhType = DHType::UNKNOWN
};

}

void EnableTaskTest::SetUpTestCase()
{}

void EnableTaskTest::TearDownTestCase()
{}

void EnableTaskTest::SetUp()
{
    auto componentManager = IComponentManager::GetOrCreateInstance();
    componentManager_ = std::static_pointer_cast<MockComponentManager>(componentManager);
    auto utilTool = IDhUtilTool::GetOrCreateInstance();
    utilTool_ = std::static_pointer_cast<MockDhUtilTool>(utilTool);
}

void EnableTaskTest::TearDown()
{
    IComponentManager::ReleaseInstance();
    componentManager_ = nullptr;
    IDhUtilTool::ReleaseInstance();
    utilTool_ = nullptr;
}

HWTEST_F(EnableTaskTest, RegisterHardware_001, TestSize.Level0)
{
    auto enableTask = std::make_shared<EnableTask>(TASK_PARAM_1.networkId, TASK_PARAM_1.uuid, TASK_PARAM_1.udid,
        TASK_PARAM_1.dhId, TASK_PARAM_1.dhType);
    enableTask->SetCallingUid(1);
    auto ret = enableTask->RegisterHardware();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(EnableTaskTest, RegisterHardware_002, TestSize.Level0)
{
    auto enableTask = std::make_shared<EnableTask>(TASK_PARAM_1.networkId, TASK_PARAM_1.uuid, TASK_PARAM_1.udid,
        TASK_PARAM_1.dhId, TASK_PARAM_1.dhType);
    enableTask->SetCallingPid(1);
    auto ret = enableTask->RegisterHardware();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(EnableTaskTest, RegisterHardware_003, TestSize.Level0)
{
    auto enableTask = std::make_shared<EnableTask>(TASK_PARAM_1.networkId, TASK_PARAM_1.uuid, TASK_PARAM_1.udid,
        TASK_PARAM_1.dhId, TASK_PARAM_1.dhType);
    enableTask->SetCallingUid(1);
    enableTask->SetCallingPid(1);
    auto ret = enableTask->RegisterHardware();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(EnableTaskTest, RegisterHardware_004, TestSize.Level0)
{
    auto enableTask = std::make_shared<EnableTask>(TASK_PARAM_1.networkId, TASK_PARAM_1.uuid, TASK_PARAM_1.udid,
        TASK_PARAM_1.dhId, TASK_PARAM_1.dhType);
    EXPECT_CALL(*utilTool_, GetLocalUdid()).WillRepeatedly(Return("udid_test"));
    EXPECT_CALL(*componentManager_, CheckDemandStart(_, _, _)).Times(1).WillRepeatedly(Return(-1));
    auto ret = enableTask->RegisterHardware();
    EXPECT_EQ(-1, ret);
}

HWTEST_F(EnableTaskTest, DoAutoEnable_001, TestSize.Level0)
{
    auto enableTask = std::make_shared<EnableTask>(TASK_PARAM_1.networkId, TASK_PARAM_1.uuid, TASK_PARAM_1.udid,
        TASK_PARAM_1.dhId, TASK_PARAM_1.dhType);
    EXPECT_CALL(*utilTool_, GetLocalUdid()).WillRepeatedly(Return("udid_test"));
    EXPECT_CALL(*componentManager_, CheckDemandStart(_, _, _)).Times(1).WillRepeatedly(
        DoAll(SetArgReferee<2>(false), Return(0)));
    auto ret = enableTask->DoAutoEnable();
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_NO_NEED_ENABLE, ret);
}

HWTEST_F(EnableTaskTest, DoAutoEnable_002, TestSize.Level0)
{
    auto enableTask = std::make_shared<EnableTask>(TASK_PARAM_1.networkId, TASK_PARAM_1.uuid, TASK_PARAM_1.udid,
        TASK_PARAM_1.dhId, TASK_PARAM_1.dhType);
    EXPECT_CALL(*utilTool_, GetLocalUdid()).WillRepeatedly(Return("udid_test"));
    EXPECT_CALL(*componentManager_, CheckDemandStart(_, _, _)).Times(1).WillRepeatedly(
        DoAll(SetArgReferee<2>(true), Return(0)));
    EXPECT_CALL(*componentManager_, EnableSource(_, _, _, _)).Times(1).WillRepeatedly(Return(-1));
    auto ret = enableTask->DoAutoEnable();
    EXPECT_EQ(-1, ret);
}

HWTEST_F(EnableTaskTest, DoAutoEnable_003, TestSize.Level0)
{
    auto enableTask = std::make_shared<EnableTask>(TASK_PARAM_1.networkId, TASK_PARAM_1.uuid, TASK_PARAM_1.udid,
        TASK_PARAM_1.dhId, TASK_PARAM_1.dhType);
    EXPECT_CALL(*utilTool_, GetLocalUdid()).WillRepeatedly(Return("udid_test"));
    EXPECT_CALL(*componentManager_, CheckDemandStart(_, _, _)).Times(1).WillRepeatedly(
        DoAll(SetArgReferee<2>(true), Return(0)));
    EXPECT_CALL(*componentManager_, EnableSource(_, _, _, _)).Times(1).WillRepeatedly(Return(0));
    auto ret = enableTask->DoAutoEnable();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(EnableTaskTest, DoAutoEnable_005, TestSize.Level0)
{
    auto enableTask = std::make_shared<EnableTask>(TASK_PARAM_1.networkId, TASK_PARAM_1.uuid, TASK_PARAM_1.udid,
        TASK_PARAM_1.dhId, TASK_PARAM_1.dhType);
    EXPECT_CALL(*utilTool_, GetLocalUdid()).WillRepeatedly(Return(DEV_DID_1));
    EXPECT_CALL(*componentManager_, CheckSinkConfigStart(_, _)).Times(1).WillRepeatedly(Return(-1));
    auto ret = enableTask->DoAutoEnable();
    EXPECT_EQ(-1, ret);
}

HWTEST_F(EnableTaskTest, DoAutoEnable_006, TestSize.Level0)
{
    auto enableTask = std::make_shared<EnableTask>(TASK_PARAM_1.networkId, TASK_PARAM_1.uuid, TASK_PARAM_1.udid,
        TASK_PARAM_1.dhId, TASK_PARAM_1.dhType);
    EXPECT_CALL(*utilTool_, GetLocalUdid()).WillRepeatedly(Return(DEV_DID_1));
    EXPECT_CALL(*componentManager_, CheckSinkConfigStart(_, _)).Times(1).WillRepeatedly(
        DoAll(SetArgReferee<1>(false), Return(0)));
    auto ret = enableTask->DoAutoEnable();
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_NO_NEED_ENABLE, ret);
}

HWTEST_F(EnableTaskTest, DoAutoEnable_007, TestSize.Level0)
{
    auto enableTask = std::make_shared<EnableTask>(TASK_PARAM_1.networkId, TASK_PARAM_1.uuid, TASK_PARAM_1.udid,
        TASK_PARAM_1.dhId, TASK_PARAM_1.dhType);
    EXPECT_CALL(*utilTool_, GetLocalUdid()).WillRepeatedly(Return(DEV_DID_1));
    EXPECT_CALL(*componentManager_, CheckSinkConfigStart(_, _)).Times(1).WillRepeatedly(
        DoAll(SetArgReferee<1>(true), Return(0)));
    EXPECT_CALL(*componentManager_, EnableSink(_, _, _)).Times(1).WillRepeatedly(Return(-1));
    auto ret = enableTask->DoAutoEnable();
    EXPECT_EQ(-1, ret);
}

HWTEST_F(EnableTaskTest, DoAutoEnable_008, TestSize.Level0)
{
    auto enableTask = std::make_shared<EnableTask>(TASK_PARAM_1.networkId, TASK_PARAM_1.uuid, TASK_PARAM_1.udid,
        TASK_PARAM_1.dhId, TASK_PARAM_1.dhType);
    EXPECT_CALL(*utilTool_, GetLocalUdid()).WillRepeatedly(Return(DEV_DID_1));
    EXPECT_CALL(*componentManager_, CheckSinkConfigStart(_, _)).Times(1).WillRepeatedly(
        DoAll(SetArgReferee<1>(true), Return(0)));
    EXPECT_CALL(*componentManager_, EnableSink(_, _, _)).Times(1).WillRepeatedly(Return(0));
    auto ret = enableTask->DoAutoEnable();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(EnableTaskTest, DoActiveEnable_001, TestSize.Level0)
{
    auto enableTask = std::make_shared<EnableTask>(TASK_PARAM_1.networkId, TASK_PARAM_1.uuid, TASK_PARAM_1.udid,
        TASK_PARAM_1.dhId, TASK_PARAM_1.dhType);
    enableTask->SetEffectSink(true);
    EXPECT_CALL(*componentManager_, EnableSink(_, _, _)).Times(1).WillRepeatedly(Return(0));
    auto ret = enableTask->DoActiveEnable();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(EnableTaskTest, DoActiveEnable_002, TestSize.Level0)
{
    auto enableTask = std::make_shared<EnableTask>(TASK_PARAM_1.networkId, TASK_PARAM_1.uuid, TASK_PARAM_1.udid,
        TASK_PARAM_1.dhId, TASK_PARAM_1.dhType);
    enableTask->SetEffectSink(true);
    EXPECT_CALL(*componentManager_, EnableSink(_, _, _)).Times(1).WillRepeatedly(Return(-1));
    auto ret = enableTask->DoActiveEnable();
    EXPECT_EQ(-1, ret);
}

HWTEST_F(EnableTaskTest, DoActiveEnable_003, TestSize.Level0)
{
    auto enableTask = std::make_shared<EnableTask>(TASK_PARAM_1.networkId, TASK_PARAM_1.uuid, TASK_PARAM_1.udid,
        TASK_PARAM_1.dhId, TASK_PARAM_1.dhType);
    enableTask->SetEffectSource(true);
    EXPECT_CALL(*componentManager_, EnableSource(_, _, _, _)).Times(1).WillRepeatedly(Return(0));
    auto ret = enableTask->DoActiveEnable();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(EnableTaskTest, DoActiveEnable_004, TestSize.Level0)
{
    auto enableTask = std::make_shared<EnableTask>(TASK_PARAM_1.networkId, TASK_PARAM_1.uuid, TASK_PARAM_1.udid,
        TASK_PARAM_1.dhId, TASK_PARAM_1.dhType);
    enableTask->SetEffectSource(true);
    EXPECT_CALL(*componentManager_, EnableSource(_, _, _, _)).Times(1).WillRepeatedly(Return(-1));
    auto ret = enableTask->DoActiveEnable();
    EXPECT_EQ(-1, ret);
}
} // namespace DistributedHardware
} // namespace OHOS