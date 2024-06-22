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

#include "constants.h"
#include "component_loader.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "distributed_hardware_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void DistributedHardwareManagerTest::SetUpTestCase(void) {}

void DistributedHardwareManagerTest::TearDownTestCase(void) {}

void DistributedHardwareManagerTest::SetUp() {}

void DistributedHardwareManagerTest::TearDown() {}

/**
 * @tc.name: Initialize_001
 * @tc.desc: Verify the Initialize success
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareManagerTest, Initialize_001, TestSize.Level0)
{
    auto ret = DistributedHardwareManager::GetInstance().Initialize();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    ret = DistributedHardwareManager::GetInstance().Initialize();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
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

/**
 * @tc.name: GetComponentVersion_001
 * @tc.desc: Verify the GetComponentVersion function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareManagerTest, GetComponentVersion_001, TestSize.Level0)
{
    ComponentLoader::GetInstance().isLocalVersionInit_.store(false);
    std::unordered_map<DHType, std::string> versionMap;
    int32_t ret =  DistributedHardwareManager::GetInstance().GetComponentVersion(versionMap);
    EXPECT_EQ(ERR_DH_FWK_LOADER_GET_LOCAL_VERSION_FAIL, ret);

    ComponentLoader::GetInstance().isLocalVersionInit_.store(true);
    ret =  DistributedHardwareManager::GetInstance().GetComponentVersion(versionMap);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
