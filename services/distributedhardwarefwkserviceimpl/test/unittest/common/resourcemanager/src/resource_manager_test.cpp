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

#include "resource_manager_test.h"

#include <vector>

#include "capability_info.h"
#define private public
#include "capability_info_manager.h"
#undef private
#include "dh_context.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "ResourceManagerTest"

namespace {
const string DEV_ID_0 = "bb536a637105409e904d4da83790a4a7";
const string DEV_ID_1 = "bb536a637105409e904d4da83790a4a8";
const string DEV_NAME = "Dev1";
const string DH_ID_0 = "Camera_0";
const string DH_ID_1 = "Mic_0";
const string DH_ID_2 = "Speaker_0";
const string DH_ID_3 = "Display_0";
const string DH_ID_4 = "Input_0";
const string DH_ATTR_0 = "attr0";
const string DH_ATTR_1 = "attr1";
constexpr uint16_t TEST_DEV_TYPE_PAD = 0x11;
constexpr uint32_t TEST_DH_TYPE_CAMERA = 0x01;
constexpr uint32_t TEST_DH_TYPE_MIC = 0x02;
constexpr uint32_t TEST_DH_TYPE_SPEAKER = 0x4;
constexpr uint32_t TEST_DH_TYPE_DISPLAY = 0x08;
constexpr uint32_t TEST_DH_TYPE_BUTTON = 0x20;
constexpr uint32_t TEST_SIZE_0 = 0;
constexpr uint32_t TEST_SIZE_2 = 2;
constexpr uint32_t TEST_SIZE_5 = 5;
constexpr uint32_t TEST_SIZE_10 = 10;
const std::string EMPTY_PREFIX = "";

const shared_ptr<CapabilityInfo> CAP_INFO_0 =
    make_shared<CapabilityInfo>(DH_ID_0, DEV_ID_0, DEV_NAME, TEST_DEV_TYPE_PAD, DHType::CAMERA, DH_ATTR_0);
const shared_ptr<CapabilityInfo> CAP_INFO_1 =
    make_shared<CapabilityInfo>(DH_ID_1, DEV_ID_0, DEV_NAME, TEST_DEV_TYPE_PAD, DHType::MIC, DH_ATTR_0);
const shared_ptr<CapabilityInfo> CAP_INFO_2 =
    make_shared<CapabilityInfo>(DH_ID_2, DEV_ID_0, DEV_NAME, TEST_DEV_TYPE_PAD, DHType::SPEAKER, DH_ATTR_0);
const shared_ptr<CapabilityInfo> CAP_INFO_3 =
    make_shared<CapabilityInfo>(DH_ID_3, DEV_ID_0, DEV_NAME, TEST_DEV_TYPE_PAD, DHType::DISPLAY, DH_ATTR_0);
const shared_ptr<CapabilityInfo> CAP_INFO_4 =
    make_shared<CapabilityInfo>(DH_ID_4, DEV_ID_0, DEV_NAME, TEST_DEV_TYPE_PAD, DHType::BUTTON, DH_ATTR_0);

const shared_ptr<CapabilityInfo> CAP_INFO_5 =
    make_shared<CapabilityInfo>(DH_ID_0, DEV_ID_1, DEV_NAME, TEST_DEV_TYPE_PAD, DHType::CAMERA, DH_ATTR_1);
const shared_ptr<CapabilityInfo> CAP_INFO_6 =
    make_shared<CapabilityInfo>(DH_ID_1, DEV_ID_1, DEV_NAME, TEST_DEV_TYPE_PAD, DHType::MIC, DH_ATTR_1);
const shared_ptr<CapabilityInfo> CAP_INFO_7 =
    make_shared<CapabilityInfo>(DH_ID_2, DEV_ID_1, DEV_NAME, TEST_DEV_TYPE_PAD, DHType::SPEAKER, DH_ATTR_1);
const shared_ptr<CapabilityInfo> CAP_INFO_8 =
    make_shared<CapabilityInfo>(DH_ID_3, DEV_ID_1, DEV_NAME, TEST_DEV_TYPE_PAD, DHType::DISPLAY, DH_ATTR_1);
const shared_ptr<CapabilityInfo> CAP_INFO_9 =
    make_shared<CapabilityInfo>(DH_ID_4, DEV_ID_1, DEV_NAME, TEST_DEV_TYPE_PAD, DHType::BUTTON, DH_ATTR_1);
}

void ResourceManagerTest::SetUpTestCase(void) {}

void ResourceManagerTest::TearDownTestCase(void) {}

void ResourceManagerTest::SetUp()
{
    vector<shared_ptr<CapabilityInfo>> resInfos { CAP_INFO_0, CAP_INFO_1, CAP_INFO_2, CAP_INFO_3, CAP_INFO_4,
        CAP_INFO_5, CAP_INFO_6, CAP_INFO_7, CAP_INFO_8, CAP_INFO_9 };
    CapabilityInfoManager::GetInstance()->AddCapability(resInfos);
}

void ResourceManagerTest::TearDown()
{
    CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_0->GetKey());
    CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_1->GetKey());
    CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_2->GetKey());
    CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_3->GetKey());
    CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_4->GetKey());
    CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_5->GetKey());
    CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_6->GetKey());
    CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_7->GetKey());
    CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_8->GetKey());
    CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_9->GetKey());
}

/**
 * @tc.name: resource_manager_test_001
 * @tc.desc: Verify the CapabilityInfoManager UnInit function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_001, TestSize.Level0)
{
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->Init(), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->UnInit(), DH_FWK_SUCCESS);
}

/**
 * @tc.name: resource_manager_test_002
 * @tc.desc: Verify the CapabilityInfoManager Init function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_002, TestSize.Level0)
{
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->Init(), DH_FWK_SUCCESS);
}

/**
 * @tc.name: resource_manager_test_003
 * @tc.desc: Verify the CapabilityInfoManager SyncDeviceInfoFromDB function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_003, TestSize.Level0)
{
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_.clear();
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->SyncDeviceInfoFromDB(DEV_ID_0), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->globalCapInfoMap_.size(), TEST_SIZE_5);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->SyncDeviceInfoFromDB(DEV_ID_1), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->globalCapInfoMap_.size(), TEST_SIZE_10);
}

/**
 * @tc.name: resource_manager_test_004
 * @tc.desc: Verify the CapabilityInfoManager SyncRemoteCapabilityInfos function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_004, TestSize.Level0)
{
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_.clear();
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->SyncRemoteCapabilityInfos(), DH_FWK_SUCCESS);
}

/**
 * @tc.name: resource_manager_test_005
 * @tc.desc: Verify the CapabilityInfoManager AddCapability function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_005, TestSize.Level0)
{
    vector<shared_ptr<CapabilityInfo>> resInfos { CAP_INFO_0, CAP_INFO_1, CAP_INFO_2, CAP_INFO_3, CAP_INFO_4,
        CAP_INFO_5, CAP_INFO_6, CAP_INFO_7, CAP_INFO_8, CAP_INFO_9 };
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->AddCapability(resInfos), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->globalCapInfoMap_.size(), TEST_SIZE_10);
}

/**
 * @tc.name: resource_manager_test_006
 * @tc.desc: Verify the CapabilityInfoManager AddCapabilityInMem function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_006, TestSize.Level0)
{
    vector<shared_ptr<CapabilityInfo>> resInfos { CAP_INFO_0, CAP_INFO_1, CAP_INFO_2, CAP_INFO_3, CAP_INFO_4,
        CAP_INFO_5, CAP_INFO_6, CAP_INFO_7, CAP_INFO_8, CAP_INFO_9 };
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->AddCapabilityInMem(resInfos), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->globalCapInfoMap_.size(), TEST_SIZE_10);
}


/**
 * @tc.name: resource_manager_test_007
 * @tc.desc: Verify the CapabilityInfoManager RemoveCapabilityInfoByKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_007, TestSize.Level0)
{
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_0->GetKey()), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_1->GetKey()), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_2->GetKey()), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_3->GetKey()), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_4->GetKey()), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_5->GetKey()), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_6->GetKey()), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_7->GetKey()), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_8->GetKey()), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(CAP_INFO_9->GetKey()), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->globalCapInfoMap_.size(), TEST_SIZE_0);
}

/**
 * @tc.name: resource_manager_test_008
 * @tc.desc: Verify the CapabilityInfoManager QueryCapabilityByFilters function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_008, TestSize.Level0)
{
    map<CapabilityInfoFilter, string> queryMap0 { { CapabilityInfoFilter::FILTER_DEVICE_ID, DEV_ID_0 } };
    map<CapabilityInfoFilter, string> queryMap1 { { CapabilityInfoFilter::FILTER_DEVICE_ID, DEV_ID_1 } };
    map<CapabilityInfoFilter, string> queryMap2 { { CapabilityInfoFilter::FILTER_DEVICE_NAME, DEV_NAME } };
    map<CapabilityInfoFilter, string> queryMap3 { { CapabilityInfoFilter::FILTER_DH_ID, DH_ID_0 } };
    map<CapabilityInfoFilter, string> queryMap4 { { CapabilityInfoFilter::FILTER_DH_ID, DH_ID_1 } };
    map<CapabilityInfoFilter, string> queryMap5 { { CapabilityInfoFilter::FILTER_DH_ID, DH_ID_2 } };
    map<CapabilityInfoFilter, string> queryMap6 { { CapabilityInfoFilter::FILTER_DH_ID, DH_ID_3 } };
    map<CapabilityInfoFilter, string> queryMap7 { { CapabilityInfoFilter::FILTER_DH_ID, DH_ID_4 } };
    map<CapabilityInfoFilter, string> queryMap8 { { CapabilityInfoFilter::FILTER_DEVICE_TYPE,
        to_string(TEST_DEV_TYPE_PAD) } };
    map<CapabilityInfoFilter, string> queryMap9 { { CapabilityInfoFilter::FILTER_DH_TYPE,
        to_string(TEST_DH_TYPE_CAMERA) } };
    map<CapabilityInfoFilter, string> queryMap10 { { CapabilityInfoFilter::FILTER_DH_TYPE,
        to_string(TEST_DH_TYPE_MIC) } };
    map<CapabilityInfoFilter, string> queryMap11 { { CapabilityInfoFilter::FILTER_DH_TYPE,
        to_string(TEST_DH_TYPE_SPEAKER) } };
    map<CapabilityInfoFilter, string> queryMap12 { { CapabilityInfoFilter::FILTER_DH_TYPE,
        to_string(TEST_DH_TYPE_DISPLAY) } };
    map<CapabilityInfoFilter, string> queryMap13 { { CapabilityInfoFilter::FILTER_DH_TYPE,
        to_string(TEST_DH_TYPE_BUTTON) } };
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->QueryCapabilityByFilters(queryMap0).size(), TEST_SIZE_5);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->QueryCapabilityByFilters(queryMap1).size(), TEST_SIZE_5);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->QueryCapabilityByFilters(queryMap2).size(), TEST_SIZE_10);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->QueryCapabilityByFilters(queryMap3).size(), TEST_SIZE_2);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->QueryCapabilityByFilters(queryMap4).size(), TEST_SIZE_2);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->QueryCapabilityByFilters(queryMap5).size(), TEST_SIZE_2);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->QueryCapabilityByFilters(queryMap6).size(), TEST_SIZE_2);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->QueryCapabilityByFilters(queryMap7).size(), TEST_SIZE_2);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->QueryCapabilityByFilters(queryMap8).size(), TEST_SIZE_10);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->QueryCapabilityByFilters(queryMap9).size(), TEST_SIZE_2);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->QueryCapabilityByFilters(queryMap10).size(), TEST_SIZE_2);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->QueryCapabilityByFilters(queryMap11).size(), TEST_SIZE_2);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->QueryCapabilityByFilters(queryMap12).size(), TEST_SIZE_2);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->QueryCapabilityByFilters(queryMap13).size(), TEST_SIZE_2);
}

/**
 * @tc.name: resource_manager_test_009
 * @tc.desc: Verify the HasCapability function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_009, TestSize.Level0)
{
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->HasCapability(DEV_ID_0, DH_ID_0), true);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->HasCapability(DEV_ID_0, DH_ID_1), true);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->HasCapability(DEV_ID_0, DH_ID_2), true);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->HasCapability(DEV_ID_0, DH_ID_3), true);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->HasCapability(DEV_ID_0, DH_ID_4), true);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->HasCapability(DEV_ID_1, DH_ID_0), true);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->HasCapability(DEV_ID_1, DH_ID_1), true);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->HasCapability(DEV_ID_1, DH_ID_2), true);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->HasCapability(DEV_ID_1, DH_ID_3), true);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->HasCapability(DEV_ID_1, DH_ID_4), true);
}

/**
 * @tc.name: resource_manager_test_010
 * @tc.desc: Verify the GetCapabilitiesByDeviceId function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_010, TestSize.Level0)
{
    vector<shared_ptr<CapabilityInfo>> capInfos;
    CapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(DEV_ID_0, capInfos);
    EXPECT_EQ(capInfos.size(), TEST_SIZE_5);
    CapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(DEV_ID_1, capInfos);
    EXPECT_EQ(capInfos.size(), TEST_SIZE_10);
}

/**
 * @tc.name: resource_manager_test_011
 * @tc.desc: Verify the GetCapability function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_011, TestSize.Level0)
{
    shared_ptr<CapabilityInfo> capInfo;
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetCapability(DEV_ID_0, DH_ID_0, capInfo), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetCapability(DEV_ID_0, DH_ID_1, capInfo), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetCapability(DEV_ID_0, DH_ID_2, capInfo), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetCapability(DEV_ID_0, DH_ID_3, capInfo), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetCapability(DEV_ID_0, DH_ID_4, capInfo), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetCapability(DEV_ID_1, DH_ID_0, capInfo), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetCapability(DEV_ID_1, DH_ID_1, capInfo), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetCapability(DEV_ID_1, DH_ID_2, capInfo), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetCapability(DEV_ID_1, DH_ID_3, capInfo), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetCapability(DEV_ID_1, DH_ID_4, capInfo), DH_FWK_SUCCESS);
}

/**
 * @tc.name: resource_manager_test_012
 * @tc.desc: Verify the GetDataByKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_012, TestSize.Level0)
{
    shared_ptr<CapabilityInfo> capInfo;
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKey(CAP_INFO_0->GetKey(), capInfo), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKey(CAP_INFO_1->GetKey(), capInfo), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKey(CAP_INFO_2->GetKey(), capInfo), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKey(CAP_INFO_3->GetKey(), capInfo), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKey(CAP_INFO_4->GetKey(), capInfo), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKey(CAP_INFO_5->GetKey(), capInfo), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKey(CAP_INFO_6->GetKey(), capInfo), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKey(CAP_INFO_7->GetKey(), capInfo), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKey(CAP_INFO_8->GetKey(), capInfo), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKey(CAP_INFO_9->GetKey(), capInfo), DH_FWK_SUCCESS);
}

/**
 * @tc.name: resource_manager_test_013
 * @tc.desc: Verify the GetDataByKeyPrefix function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_013, TestSize.Level0)
{
    CapabilityInfoMap capMap;
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(EMPTY_PREFIX, capMap), DH_FWK_SUCCESS);
    EXPECT_EQ(capMap.size(), TEST_SIZE_10);
}
}
}
