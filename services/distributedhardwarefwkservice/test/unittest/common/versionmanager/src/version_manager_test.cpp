/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "version_manager_test.h"

#include "component_loader.h"
#include "version_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void VersionManagerTest::SetUpTestCase(void) {}

void VersionManagerTest::TearDownTestCase(void) {}

void VersionManagerTest::SetUp()
{
    ComponentLoader::GetInstance().Init();
}

void VersionManagerTest::TearDown()
{
    ComponentLoader::GetInstance().UnInit();
}

namespace {
const std::string TEST_DEVICE_ID_1 = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_DEVICE_ID_2 = "bb536a637105409e904d4da83790a4a8";
const std::string TEST_DEVICE_ID_3 = "bb536a637105409e904d4da83790a4a9";
const std::string TEST_DEVICE_ID_4 = "bb536a637105409e904d4da83790a4a0";
const std::string TEST_HANDLER_VERSION_1 = "1.0";
const std::string TEST_HANDLER_VERSION_2 = "3.0";
const std::string TEST_HANDLER_VERSION_3 = "5.0";
const std::string TEST_SOURCE_VERSION_1 = "2.2";
const std::string TEST_SOURCE_VERSION_2 = "2.0";
const std::string TEST_SOURCE_VERSION_3 = "6.0";
const std::string TEST_SINK_VERSION_1 = "2.4";
const std::string TEST_SINK_VERSION_2 = "2.0";
const std::string TEST_SINK_VERSION_3 = "2.0";
const std::string TEST_DH_VERSION = "3.1";
const std::string TEST_COMPONENT_NAME_1 = "distributed_camera";
const std::string TEST_COMPONENT_NAME_2 = "distributed_audio";
const std::string TEST_COMPONENT_NAME_3 = "distributed_mic";
}

/**
 * @tc.name: version_manager_test_001
 * @tc.desc: Verify the Init function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSKN
 */
HWTEST_F(VersionManagerTest, version_manager_test_001, TestSize.Level0)
{
    auto ret = VersionManager::GetInstance().Init();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

void CompVersionGetValue(CompVersion &cVs, std::string name, DHType dhType, std::string handlerVersion,
    std::string sourceVersion, std::string sinkVersion)
{
    cVs.name = name;
    cVs.dhType = dhType;
    cVs.handlerVersion = handlerVersion;
    cVs.sourceVersion = sourceVersion;
    cVs.sinkVersion = sinkVersion;
}

/**
 * @tc.name: version_manager_test_002
 * @tc.desc: Verify AddDHVersion function
 * @tc.type: FUNC
 * @tc.require: AR000GHSKN
 */
HWTEST_F(VersionManagerTest, version_manager_test_002, TestSize.Level0)
{
    DHVersion dhVersion;
    CompVersion cVs1;
    CompVersionGetValue(cVs1, TEST_COMPONENT_NAME_1, DHType::CAMERA, TEST_HANDLER_VERSION_1, TEST_SOURCE_VERSION_1,
        TEST_SINK_VERSION_1);
    CompVersion cVs2;
    CompVersionGetValue(cVs2, TEST_COMPONENT_NAME_2, DHType::AUDIO, TEST_HANDLER_VERSION_2, TEST_SOURCE_VERSION_2,
        TEST_SINK_VERSION_2);
    CompVersion cVs3;
    CompVersionGetValue(cVs3, TEST_COMPONENT_NAME_3, DHType::SCREEN, TEST_HANDLER_VERSION_3, TEST_SOURCE_VERSION_3,
        TEST_SINK_VERSION_3);
    dhVersion.uuid = TEST_DEVICE_ID_1;
    dhVersion.dhVersion = TEST_DH_VERSION;
    dhVersion.compVersions.insert(std::make_pair(cVs1.dhType, cVs1));
    dhVersion.compVersions.insert(std::make_pair(cVs2.dhType, cVs2));
    dhVersion.compVersions.insert(std::make_pair(cVs3.dhType, cVs3));
    int32_t ret = VersionManager::GetInstance().AddDHVersion(dhVersion.uuid, dhVersion);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    dhVersion.uuid = TEST_DEVICE_ID_2;
    ret = VersionManager::GetInstance().AddDHVersion(dhVersion.uuid, dhVersion);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: version_manager_test_003
 * @tc.desc: Verify GetLocalDeviceVersion function
 * @tc.type: FUNC
 * @tc.require: AR000GHSKN
 */
HWTEST_F(VersionManagerTest, version_manager_test_003, TestSize.Level0)
{
    std::string strVersion = VersionManager::GetInstance().GetLocalDeviceVersion();
    EXPECT_EQ(DH_LOCAL_VERSION, strVersion);
}

/**
 * @tc.name: version_manager_test_004
 * @tc.desc: Verify GetDHVersion function
 * @tc.type: FUNC
 * @tc.require: AR000GHSKN
 */
HWTEST_F(VersionManagerTest, version_manager_test_004, TestSize.Level0)
{
    DHVersion dhVersion;
    int32_t ret = VersionManager::GetInstance().GetDHVersion(TEST_DEVICE_ID_2, dhVersion);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    EXPECT_EQ(TEST_HANDLER_VERSION_2, dhVersion.compVersions[DHType::AUDIO].handlerVersion);
    EXPECT_EQ(TEST_DH_VERSION, dhVersion.dhVersion);
    ret = VersionManager::GetInstance().GetDHVersion(TEST_DEVICE_ID_3, dhVersion);
    EXPECT_EQ(ERR_DH_FWK_VERSION_DEVICE_ID_NOT_EXIST, ret);
}

/**
 * @tc.name: version_manager_test_005
 * @tc.desc: Verify RemoveDHVersion function
 * @tc.type: FUNC
 * @tc.require: AR000GHSKN
 */
HWTEST_F(VersionManagerTest, version_manager_test_005, TestSize.Level0)
{
    int32_t ret = VersionManager::GetInstance().RemoveDHVersion(TEST_DEVICE_ID_2);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    ret = VersionManager::GetInstance().RemoveDHVersion(TEST_DEVICE_ID_4);
    EXPECT_EQ(ERR_DH_FWK_VERSION_DEVICE_ID_NOT_EXIST, ret);
    ret = VersionManager::GetInstance().RemoveDHVersion(TEST_DEVICE_ID_2);
    EXPECT_EQ(ERR_DH_FWK_VERSION_DEVICE_ID_NOT_EXIST, ret);
    ret = VersionManager::GetInstance().RemoveDHVersion(TEST_DEVICE_ID_1);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: version_manager_test_006
 * @tc.desc: Verify GetCompVersion function
 * @tc.type: FUNC
 * @tc.require: AR000GHSKN
 */
HWTEST_F(VersionManagerTest, version_manager_test_006, TestSize.Level0)
{
    DHVersion dhVersion;
    CompVersion cVs1;
    CompVersionGetValue(cVs1, TEST_COMPONENT_NAME_1, DHType::CAMERA, TEST_HANDLER_VERSION_1, TEST_SOURCE_VERSION_1,
        TEST_SINK_VERSION_1);
    dhVersion.uuid = TEST_DEVICE_ID_1;
    dhVersion.dhVersion = TEST_DH_VERSION;
    dhVersion.compVersions.insert(std::make_pair(cVs1.dhType, cVs1));
    int32_t ret = VersionManager::GetInstance().AddDHVersion(dhVersion.uuid, dhVersion);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    ret = VersionManager::GetInstance().GetCompVersion(TEST_DEVICE_ID_1, DHType::CAMERA, cVs1);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: version_manager_test_007
 * @tc.desc: Verify GetCompVersion function
 * @tc.type: FUNC
 * @tc.require: AR000GHSKN
 */
HWTEST_F(VersionManagerTest, version_manager_test_007, TestSize.Level0)
{
    DHVersion dhVersion;
    CompVersion cVs1;
    CompVersionGetValue(cVs1, TEST_COMPONENT_NAME_1, DHType::CAMERA, TEST_HANDLER_VERSION_1, TEST_SOURCE_VERSION_1,
        TEST_SINK_VERSION_1);
    dhVersion.uuid = TEST_DEVICE_ID_1;
    dhVersion.dhVersion = TEST_DH_VERSION;
    dhVersion.compVersions.insert(std::make_pair(cVs1.dhType, cVs1));
    int32_t ret = VersionManager::GetInstance().AddDHVersion(dhVersion.uuid, dhVersion);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    ret = VersionManager::GetInstance().GetCompVersion(TEST_DEVICE_ID_1, DHType::AUDIO, cVs1);
    EXPECT_EQ(ERR_DH_FWK_TYPE_NOT_EXIST, ret);
}

/**
 * @tc.name: version_manager_test_008
 * @tc.desc: Verify the Init function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSKN
 */
HWTEST_F(VersionManagerTest, version_manager_test_008, TestSize.Level0)
{
    ComponentLoader::GetInstance().isLocalVersionInit_.store(false);
    int32_t ret = VersionManager::GetInstance().Init();
    EXPECT_EQ(ERR_DH_FWK_LOADER_GET_LOCAL_VERSION_FAIL, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
