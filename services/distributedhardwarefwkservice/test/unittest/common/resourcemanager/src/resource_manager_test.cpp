/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#define private public
#include "capability_info.h"
#include "capability_info_manager.h"
#include "version_info_event.h"
#undef private
#include "dh_context.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "cJSON.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "ResourceManagerTest"

namespace {
const string DATABASE_DIR = "/data/service/el1/public/database/dtbhardware_manager_service/";
const string DEV_ID_0 = "bb536a637105409e904d4da83790a4a7";
const string DEV_ID_1 = "bb536a637105409e904d4da83790a4a8";
const string TEST_DEV_NAME = "Dev1";
const string DH_ID_0 = "Camera_0";
const string DH_ID_1 = "Mic_0";
const string DH_ID_2 = "Gps_0";
const string DH_ID_3 = "Display_0";
const string DH_ID_4 = "Input_0";
const string DH_ATTR_0 = "attr0";
const string DH_ATTR_1 = "attr1";
const string DH_SUBTYPE_0 = "camera";
const string DH_SUBTYPE_1 = "mic";
const string DH_SUBTYPE_2 = "gps";
const string DH_SUBTYPE_3 = "screen";
const string DH_SUBTYPE_4 = "input";
constexpr uint16_t TEST_DEV_TYPE_PAD = 0x11;
constexpr uint32_t TEST_DH_TYPE_CAMERA = 0x01;
constexpr uint32_t TEST_DH_TYPE_MIC = 0x02;
constexpr uint32_t TEST_DH_TYPE_GPS = 0x10;
constexpr uint32_t TEST_DH_TYPE_DISPLAY = 0x08;
constexpr uint32_t TEST_DH_TYPE_BUTTON = 0x20;
constexpr uint32_t TEST_SIZE_0 = 0;
constexpr uint32_t TEST_SIZE_2 = 2;
constexpr uint32_t TEST_SIZE_5 = 5;
constexpr uint32_t TEST_SIZE_10 = 10;
const std::string EMPTY_PREFIX = "";

const shared_ptr<CapabilityInfo> CAP_INFO_0 =
    make_shared<CapabilityInfo>(DH_ID_0, DEV_ID_0, TEST_DEV_NAME, TEST_DEV_TYPE_PAD, DHType::CAMERA, DH_ATTR_0,
    DH_SUBTYPE_0);
const shared_ptr<CapabilityInfo> CAP_INFO_1 =
    make_shared<CapabilityInfo>(DH_ID_1, DEV_ID_0, TEST_DEV_NAME, TEST_DEV_TYPE_PAD, DHType::AUDIO, DH_ATTR_0,
    DH_SUBTYPE_1);
const shared_ptr<CapabilityInfo> CAP_INFO_2 =
    make_shared<CapabilityInfo>(DH_ID_2, DEV_ID_0, TEST_DEV_NAME, TEST_DEV_TYPE_PAD, DHType::GPS, DH_ATTR_0,
    DH_SUBTYPE_2);
const shared_ptr<CapabilityInfo> CAP_INFO_3 =
    make_shared<CapabilityInfo>(DH_ID_3, DEV_ID_0, TEST_DEV_NAME, TEST_DEV_TYPE_PAD, DHType::SCREEN, DH_ATTR_0,
    DH_SUBTYPE_3);
const shared_ptr<CapabilityInfo> CAP_INFO_4 =
    make_shared<CapabilityInfo>(DH_ID_4, DEV_ID_0, TEST_DEV_NAME, TEST_DEV_TYPE_PAD, DHType::INPUT, DH_ATTR_0,
    DH_SUBTYPE_4);

const shared_ptr<CapabilityInfo> CAP_INFO_5 =
    make_shared<CapabilityInfo>(DH_ID_0, DEV_ID_1, TEST_DEV_NAME, TEST_DEV_TYPE_PAD, DHType::CAMERA, DH_ATTR_1,
    DH_SUBTYPE_0);
const shared_ptr<CapabilityInfo> CAP_INFO_6 =
    make_shared<CapabilityInfo>(DH_ID_1, DEV_ID_1, TEST_DEV_NAME, TEST_DEV_TYPE_PAD, DHType::AUDIO, DH_ATTR_1,
    DH_SUBTYPE_1);
const shared_ptr<CapabilityInfo> CAP_INFO_7 =
    make_shared<CapabilityInfo>(DH_ID_2, DEV_ID_1, TEST_DEV_NAME, TEST_DEV_TYPE_PAD, DHType::GPS, DH_ATTR_1,
    DH_SUBTYPE_2);
const shared_ptr<CapabilityInfo> CAP_INFO_8 =
    make_shared<CapabilityInfo>(DH_ID_3, DEV_ID_1, TEST_DEV_NAME, TEST_DEV_TYPE_PAD, DHType::SCREEN, DH_ATTR_1,
    DH_SUBTYPE_3);
const shared_ptr<CapabilityInfo> CAP_INFO_9 =
    make_shared<CapabilityInfo>(DH_ID_4, DEV_ID_1, TEST_DEV_NAME, TEST_DEV_TYPE_PAD, DHType::INPUT, DH_ATTR_1,
    DH_SUBTYPE_4);
}

void ResourceManagerTest::SetUpTestCase(void)
{
    auto ret = mkdir(DATABASE_DIR.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (ret != 0) {
        DHLOGE("mkdir failed, path: %{public}s, errno : %{public}d", DATABASE_DIR.c_str(), errno);
    }
}

void ResourceManagerTest::TearDownTestCase(void)
{
    auto ret = remove(DATABASE_DIR.c_str());
    if (ret != 0) {
        DHLOGE("remove dir failed, path: %{public}s, errno : %{public}d", DATABASE_DIR.c_str(), errno);
    }
}

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
    map<CapabilityInfoFilter, string> queryMap2 { { CapabilityInfoFilter::FILTER_DEVICE_NAME, TEST_DEV_NAME } };
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
        to_string(TEST_DH_TYPE_GPS) } };
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
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(CAP_INFO_0->GetDeviceId(), capMap),
        DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(CAP_INFO_1->GetDeviceId(), capMap),
        DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(CAP_INFO_2->GetDeviceId(), capMap),
        DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(CAP_INFO_3->GetDeviceId(), capMap),
        DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(CAP_INFO_4->GetDeviceId(), capMap),
        DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(CAP_INFO_5->GetDeviceId(), capMap),
        DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(CAP_INFO_6->GetDeviceId(), capMap),
        DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(CAP_INFO_7->GetDeviceId(), capMap),
        DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(CAP_INFO_8->GetDeviceId(), capMap),
        DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(CAP_INFO_9->GetDeviceId(), capMap),
        DH_FWK_SUCCESS);
    EXPECT_EQ(capMap.size(), TEST_SIZE_10);
}

/**
 * @tc.name: resource_manager_test_014
 * @tc.desc: Verify the RemoveCapabilityInfoInDB function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_014, TestSize.Level0)
{
    std::string deviceIdEmpty = "";
    int32_t ret = CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoInDB(deviceIdEmpty);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: resource_manager_test_015
 * @tc.desc: Verify the GetCapabilityByValue function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_015, TestSize.Level0)
{
    std::string value = "";
    std::shared_ptr<CapabilityInfo> capPtr = nullptr;
    int32_t ret = CapabilityUtils::GetCapabilityByValue(value, capPtr);
    EXPECT_EQ(ERR_DH_FWK_JSON_PARSE_FAILED, ret);
}

/**
 * @tc.name: resource_manager_test_016
 * @tc.desc: Verify the GetCapabilityKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_016, TestSize.Level0)
{
    std::string deviceId = "deviceIdtest";
    std::string dhId = "dhIdtest";
    std::string str = CapabilityUtils::GetCapabilityKey(deviceId, dhId);
    EXPECT_EQ("deviceIdtest###dhIdtest", str);
}

/**
 * @tc.name: resource_manager_test_017
 * @tc.desc: Verify the IsCapKeyMatchDeviceId function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_017, TestSize.Level0)
{
    std::string key = "keytest";
    bool ret = CapabilityUtils::IsCapKeyMatchDeviceId(key, DEV_ID_0);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: resource_manager_test_018
 * @tc.desc: Verify the IsCapKeyMatchDeviceId function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_018, TestSize.Level0)
{
    std::string key = "bb536a637105409e904d4da83790a4a7###keytest";
    bool ret = CapabilityUtils::IsCapKeyMatchDeviceId(key, DEV_ID_0);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: resource_manager_test_019
 * @tc.desc: Verify the FromJsonString function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_019, TestSize.Level0)
{
    CapabilityInfo capaInfo;
    std::string jsonStr = "";
    int32_t ret = capaInfo.FromJsonString(jsonStr);
    EXPECT_EQ(ERR_DH_FWK_JSON_PARSE_FAILED, ret);
}

/**
 * @tc.name: resource_manager_test_020
 * @tc.desc: Verify the Compare function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_020, TestSize.Level0)
{
    CapabilityInfo capaInfo(DH_ID_0, DEV_ID_0, TEST_DEV_NAME, TEST_DEV_TYPE_PAD, DHType::CAMERA,
        DH_ATTR_0, DH_SUBTYPE_0);
    bool ret = CAP_INFO_1->Compare(capaInfo);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: OnChange_001
 * @tc.desc: Verify the OnChange function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, OnChange_001, TestSize.Level0)
{
    DistributedKv::Entry insert, update, del;
    insert.key = "strBase";
    update.key = "strBase";
    del.key = "strBase";
    insert.value = "strBase";
    update.value = "strBase";
    del.value = "strBase";
    std::vector<DistributedKv::Entry> inserts, updates, deleteds;
    inserts.push_back(insert);
    updates.push_back(update);
    deleteds.push_back(del);

    DistributedKv::ChangeNotification changeIn(std::move(inserts), std::move(updates), std::move(deleteds), "", true);

    CapabilityInfoManager::GetInstance()->OnChange(changeIn);
    EXPECT_NE(nullptr, CapabilityInfoManager::GetInstance()->dbAdapterPtr_);
}

/**
 * @tc.name: OnChange_002
 * @tc.desc: Verify the OnChange function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, OnChange_002, TestSize.Level0)
{
    DistributedKv::DataOrigin origin;
    origin.id = {};
    origin.store = "";
    CapabilityInfoManager::Keys keys;
    keys[CapabilityInfoManager::OP_INSERT] = {"strBase"};
    keys[CapabilityInfoManager::OP_UPDATE] = {"strBase"};
    keys[CapabilityInfoManager::OP_DELETE] = {"strBase"};
    CapabilityInfoManager::GetInstance()->OnChange(origin, std::move(keys));
    EXPECT_NE(nullptr, CapabilityInfoManager::GetInstance()->dbAdapterPtr_);
}

/**
 * @tc.name: HandleCapabilityAddChange_001
 * @tc.desc: Verify the HandleCapabilityAddChange function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ResourceManagerTest, HandleCapabilityAddChange_001, TestSize.Level0)
{
    std::vector<DistributedKv::Entry> insertRecords;
    DistributedKv::Entry entry;
    entry.key = "strBase";
    entry.value = "strBase";
    insertRecords.push_back(entry);
    CapabilityInfoManager::GetInstance()->HandleCapabilityAddChange(insertRecords);
    EXPECT_NE(nullptr, CapabilityInfoManager::GetInstance()->dbAdapterPtr_);
}

/**
 * @tc.name: HandleCapabilityUpdateChange_001
 * @tc.desc: Verify the HandleCapabilityUpdateChange function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ResourceManagerTest, HandleCapabilityUpdateChange_001, TestSize.Level0)
{
    std::vector<DistributedKv::Entry> updateRecords;
    DistributedKv::Entry entry;
    entry.key = "strBase";
    entry.value = "strBase";
    updateRecords.push_back(entry);
    CapabilityInfoManager::GetInstance()->HandleCapabilityUpdateChange(updateRecords);
    EXPECT_NE(nullptr, CapabilityInfoManager::GetInstance()->dbAdapterPtr_);
}

/**
 * @tc.name: IsCapabilityMatchFilter_001
 * @tc.desc: Verify the IsCapabilityMatchFilter function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ResourceManagerTest, IsCapabilityMatchFilter_001, TestSize.Level0)
{
    std::shared_ptr<CapabilityInfo> cap = nullptr;
    CapabilityInfoFilter filter = CapabilityInfoFilter::FILTER_DH_ID;
    std::string value;
    bool ret = CapabilityInfoManager::GetInstance()->IsCapabilityMatchFilter(cap, filter, value);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: IsCapabilityMatchFilter_002
 * @tc.desc: Verify the IsCapabilityMatchFilter function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ResourceManagerTest, IsCapabilityMatchFilter_002, TestSize.Level0)
{
    std::shared_ptr<CapabilityInfo> cap = std::make_shared<CapabilityInfo>("", "", "", 0, DHType::UNKNOWN, "", "");
    CapabilityInfoFilter filter = CapabilityInfoFilter::FILTER_DH_ID;
    std::string value;
    bool ret = CapabilityInfoManager::GetInstance()->IsCapabilityMatchFilter(cap, filter, value);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: IsCapabilityMatchFilter_003
 * @tc.desc: Verify the IsCapabilityMatchFilter function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ResourceManagerTest, IsCapabilityMatchFilter_003, TestSize.Level0)
{
    std::shared_ptr<CapabilityInfo> cap = std::make_shared<CapabilityInfo>("", "", "", 0, DHType::UNKNOWN, "", "");
    CapabilityInfoFilter filter = CapabilityInfoFilter::FILTER_DEVICE_ID;
    std::string value;
    bool ret = CapabilityInfoManager::GetInstance()->IsCapabilityMatchFilter(cap, filter, value);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: IsCapabilityMatchFilter_004
 * @tc.desc: Verify the IsCapabilityMatchFilter function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ResourceManagerTest, IsCapabilityMatchFilter_004, TestSize.Level0)
{
    std::shared_ptr<CapabilityInfo> cap = std::make_shared<CapabilityInfo>("", "", "", 0, DHType::UNKNOWN, "", "");
    CapabilityInfoFilter filter = CapabilityInfoFilter::FILTER_DEVICE_NAME;
    std::string value;
    bool ret = CapabilityInfoManager::GetInstance()->IsCapabilityMatchFilter(cap, filter, value);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: IsCapabilityMatchFilter_005
 * @tc.desc: Verify the IsCapabilityMatchFilter function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ResourceManagerTest, IsCapabilityMatchFilter_005, TestSize.Level0)
{
    std::shared_ptr<CapabilityInfo> cap = std::make_shared<CapabilityInfo>("", "", "", 0, DHType::UNKNOWN, "", "");
    CapabilityInfoFilter filter = CapabilityInfoFilter::FILTER_DEVICE_TYPE;
    uint16_t devType = 123;
    std::string value = std::to_string(devType);
    bool ret = CapabilityInfoManager::GetInstance()->IsCapabilityMatchFilter(cap, filter, value);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: IsCapabilityMatchFilter_006
 * @tc.desc: Verify the IsCapabilityMatchFilter function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ResourceManagerTest, IsCapabilityMatchFilter_006, TestSize.Level0)
{
    std::shared_ptr<CapabilityInfo> cap = std::make_shared<CapabilityInfo>("", "", "", 0, DHType::UNKNOWN, "", "");
    CapabilityInfoFilter filter = CapabilityInfoFilter::FILTER_DH_TYPE;
    DHType dhType = DHType::AUDIO;
    std::string value = std::to_string(static_cast<uint32_t>(dhType));
    bool ret = CapabilityInfoManager::GetInstance()->IsCapabilityMatchFilter(cap, filter, value);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: IsCapabilityMatchFilter_007
 * @tc.desc: Verify the IsCapabilityMatchFilter function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ResourceManagerTest, IsCapabilityMatchFilter_007, TestSize.Level0)
{
    std::shared_ptr<CapabilityInfo> cap = std::make_shared<CapabilityInfo>("", "", "", 0, DHType::UNKNOWN, "", "");
    CapabilityInfoFilter filter = CapabilityInfoFilter::FILTER_DH_ATTRS;
    std::string value;
    bool ret = CapabilityInfoManager::GetInstance()->IsCapabilityMatchFilter(cap, filter, value);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: IsCapabilityMatchFilter_008
 * @tc.desc: Verify the IsCapabilityMatchFilter function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ResourceManagerTest, IsCapabilityMatchFilter_008, TestSize.Level0)
{
    std::shared_ptr<CapabilityInfo> cap = std::make_shared<CapabilityInfo>("", "", "", 0, DHType::UNKNOWN, "", "");
    uint32_t invalid = 6;
    CapabilityInfoFilter filter = static_cast<CapabilityInfoFilter>(invalid);
    std::string value;
    bool ret = CapabilityInfoManager::GetInstance()->IsCapabilityMatchFilter(cap, filter, value);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: GetCapabilitiesByDeviceId_001
 * @tc.desc: Verify the GetCapabilitiesByDeviceId function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ResourceManagerTest, GetCapabilitiesByDeviceId_001, TestSize.Level0)
{
    std::string deviceId;
    std::vector<std::shared_ptr<CapabilityInfo>> resInfos;
    CapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(deviceId, resInfos);
    EXPECT_NE(nullptr, CapabilityInfoManager::GetInstance()->dbAdapterPtr_);
}

/**
 * @tc.name: HasCapability_001
 * @tc.desc: Verify the HasCapability function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ResourceManagerTest, HasCapability_001, TestSize.Level0)
{
    std::string deviceId;
    std::string dhId;
    bool ret = CapabilityInfoManager::GetInstance()->HasCapability(deviceId, dhId);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: SyncRemoteCapabilityInfos_001
 * @tc.desc: Verify the CapabilityInfoManager SyncRemoteCapabilityInfos function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, SyncRemoteCapabilityInfos_001, TestSize.Level0)
{
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_.clear();
    CapabilityInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    int32_t ret = CapabilityInfoManager::GetInstance()->SyncRemoteCapabilityInfos();
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

/**
 * @tc.name: RemoveCapabilityInfoInDB_001
 * @tc.desc: Verify the RemoveCapabilityInfoInDB function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, RemoveCapabilityInfoInDB_001, TestSize.Level0)
{
    std::string deviceIdEmpty;
    uint32_t MAX_ID_LEN = 257;
    deviceIdEmpty.resize(MAX_ID_LEN);
    int32_t ret = CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoInDB(deviceIdEmpty);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: RemoveCapabilityInfoInDB_002
 * @tc.desc: Verify the RemoveCapabilityInfoInDB function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, RemoveCapabilityInfoInDB_002, TestSize.Level0)
{
    std::string deviceIdEmpty = "deviceIdEmpty";
    CapabilityInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    int32_t ret = CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoInDB(deviceIdEmpty);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

/**
 * @tc.name: HandleCapabilityDeleteChange_001
 * @tc.desc: Verify the HandleCapabilityDeleteChange function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, HandleCapabilityDeleteChange_001, TestSize.Level0)
{
    std::vector<DistributedKv::Entry> deleteRecords;
    DistributedKv::Entry entry;
    entry.key = "strBase";
    entry.value = "strBase";
    deleteRecords.push_back(entry);
    CapabilityInfoManager::GetInstance()->HandleCapabilityDeleteChange(deleteRecords);
    EXPECT_EQ(nullptr, CapabilityInfoManager::GetInstance()->dbAdapterPtr_);
}

/**
 * @tc.name: GetDataByKey_001
 * @tc.desc: Verify the GetDataByKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, GetDataByKey_001, TestSize.Level0)
{
    std::string key;
    std::shared_ptr<CapabilityInfo> capInfoPtr;
    CapabilityInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    int32_t ret = CapabilityInfoManager::GetInstance()->GetDataByKey(key, capInfoPtr);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

/**
 * @tc.name: GetDataByKeyPrefix_001
 * @tc.desc: Verify the GetDataByKeyPrefix function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, GetDataByKeyPrefix_001, TestSize.Level0)
{
    std::string keyPrefix;
    CapabilityInfoMap capabilityMap;
    CapabilityInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    int32_t ret = CapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(keyPrefix, capabilityMap);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

/**
 * @tc.name: DumpCapabilityInfos_001
 * @tc.desc: Verify the DumpCapabilityInfos function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, DumpCapabilityInfos_001, TestSize.Level0)
{
    std::vector<CapabilityInfo> capInfos;
    std::string dhId;
    std::string devId;
    std::string devName;
    uint16_t devType = 0;
    DHType dhType = DHType::GPS;
    std::string dhAttrs;
    std::string dhSubtype;
    CapabilityInfo info(dhId, devId, devName, devType, dhType, dhAttrs, dhSubtype);
    capInfos.push_back(info);
    CapabilityInfoManager::GetInstance()->DumpCapabilityInfos(capInfos);
    EXPECT_EQ(nullptr, CapabilityInfoManager::GetInstance()->dbAdapterPtr_);
}

/**
 * @tc.name: FromJson_001
 * @tc.desc: Verify the FromJson function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, FromJson_001, TestSize.Level0)
{
    std::string dhId;
    std::string devId;
    std::string devName;
    uint16_t devType = 0;
    DHType dhType = DHType::GPS;
    std::string dhAttrs;
    std::string dhSubtype;
    CapabilityInfo info(dhId, devId, devName, devType, dhType, dhAttrs, dhSubtype);

    cJSON* json = cJSON_CreateObject();
    const char* DH_ID = "dh_id";
    const char* DEV_ID = "dev_id";
    const char* DEV_NAME = "dev_name";
    const char* DEV_TYPE = "dev_type";
    const char* DH_TYPE = "dh_type";
    const char* DH_ATTRS = "dh_attrs";
    const char* DH_SUBTYPE = "dh_subtype";

    cJSON_AddStringToObject(json, DH_ID, "dh_id");
    cJSON_AddStringToObject(json, DEV_ID, "dev_id");
    cJSON_AddStringToObject(json, DEV_NAME, "dev_name");
    cJSON_AddNumberToObject(json, DEV_TYPE, devType);
    cJSON_AddStringToObject(json, DH_TYPE, "dh_type");
    cJSON_AddStringToObject(json, DH_ATTRS, "dh_attrs");
    cJSON_AddStringToObject(json, DH_SUBTYPE, "dh_subtype");

    char* cjson = cJSON_Print(json);
    std::string jsonStr(cjson);
    cJSON_free(cjson);
    cJSON_Delete(json);
    EXPECT_EQ(DH_FWK_SUCCESS, info.FromJsonString(jsonStr));
}

/**
 * @tc.name: GetEntriesByKeys_001
 * @tc.desc: Verify the GetEntriesByKeys function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, GetEntriesByKeys_001, TestSize.Level0)
{
    std::vector<std::string> keys {};
    auto entries = CapabilityInfoManager::GetInstance()->GetEntriesByKeys(keys);
    EXPECT_EQ(0, entries.size());
}
} // namespace DistributedHardware
} // namespace OHOS
