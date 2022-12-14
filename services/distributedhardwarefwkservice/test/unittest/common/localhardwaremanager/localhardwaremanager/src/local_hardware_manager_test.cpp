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

#include "local_hardware_manager_test.h"

#define private public
#include "capability_info_manager.h"
#include "component_loader.h"
#include "dh_context.h"
#include "local_hardware_manager.h"
#undef private

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void LocalHardwareManagerTest::SetUpTestCase(void)
{
    ComponentLoader::GetInstance().Init();
}

void LocalHardwareManagerTest::TearDownTestCase(void)
{
    ComponentLoader::GetInstance().UnInit();
}

void LocalHardwareManagerTest::SetUp() {}

void LocalHardwareManagerTest::TearDown() {}

/**
 * @tc.name: local_hardware_manager_test_001
 * @tc.desc: Verify the Init function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(LocalHardwareManagerTest, local_hardware_manager_test_001, TestSize.Level0)
{
    LocalHardwareManager::GetInstance().Init();
}

/**
 * @tc.name: local_hardware_manager_test_002
 * @tc.desc: Verify the UnInit function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(LocalHardwareManagerTest, local_hardware_manager_test_002, TestSize.Level0)
{
    LocalHardwareManager::GetInstance().UnInit();
}

/**
 * @tc.name: Init_001
 * @tc.desc: Verify the Init function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(LocalHardwareManagerTest, Init_001, TestSize.Level0)
{
    ComponentLoader::GetInstance().UnInit();
    LocalHardwareManager::GetInstance().Init();
    EXPECT_EQ(true, LocalHardwareManager::GetInstance().pluginListenerMap_.empty());
}

/**
 * @tc.name: CheckNonExistCapabilityInfo_001
 * @tc.desc: Verify the CheckNonExistCapabilityInfo function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(LocalHardwareManagerTest, CheckNonExistCapabilityInfo_001, TestSize.Level0)
{
    std::vector<DHItem> dhItems;
    DHType dhType =  DHType::INPUT;
    LocalHardwareManager::GetInstance().CheckNonExistCapabilityInfo(dhItems, dhType);
    EXPECT_EQ(true, LocalHardwareManager::GetInstance().pluginListenerMap_.empty());
}

/**
 * @tc.name: GetLocalCapabilityMapByPrefix_001
 * @tc.desc: Verify the GetLocalCapabilityMapByPrefix function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(LocalHardwareManagerTest, GetLocalCapabilityMapByPrefix_001, TestSize.Level0)
{
    DHType dhType =  DHType::INPUT;
    CapabilityInfoMap capabilityInfoMap;
    LocalHardwareManager::GetInstance().GetLocalCapabilityMapByPrefix(dhType, capabilityInfoMap);
    EXPECT_EQ(true, LocalHardwareManager::GetInstance().pluginListenerMap_.empty());
}

/**
 * @tc.name: GetLocalCapabilityMapByPrefix_002
 * @tc.desc: Verify the GetLocalCapabilityMapByPrefix function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(LocalHardwareManagerTest, GetLocalCapabilityMapByPrefix_002, TestSize.Level0)
{
    uint32_t invalid = 0x09;
    DHType dhType = static_cast<DHType>(invalid);
    CapabilityInfoMap capabilityInfoMap;
    DHContext::GetInstance().devInfo_.deviceId = "deviceId";
    LocalHardwareManager::GetInstance().GetLocalCapabilityMapByPrefix(dhType, capabilityInfoMap);
    EXPECT_EQ(true, LocalHardwareManager::GetInstance().pluginListenerMap_.empty());
}
} // namespace DistributedHardware
} // namespace OHOS
