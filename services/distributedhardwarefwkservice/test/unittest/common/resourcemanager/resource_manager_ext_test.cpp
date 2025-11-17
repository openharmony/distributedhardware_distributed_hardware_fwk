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

#include <gtest/gtest.h>
#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#include "cJSON.h"

#include "constants.h"
#include "capability_info.h"
#include "capability_info_manager.h"
#include "dh_context.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "dh_utils_tool.h"
#include "get_dh_descriptors_callback_stub.h"
#include "task_board.h"
#include "impl_utils.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "ResourceManagerExtTest"

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
constexpr uint32_t TEST_SIZE_0 = 0;
constexpr uint32_t TEST_SIZE_5 = 5;
constexpr uint32_t TEST_SIZE_10 = 10;
constexpr uint32_t MAX_DB_RECORD_LENGTH = 10005;
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

class ResourceManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

public:
    class TestGetDistributedHardwareCallback : public GetDhDescriptorsCallbackStub {
    public:
        TestGetDistributedHardwareCallback() = default;
        virtual ~TestGetDistributedHardwareCallback() = default;
    protected:
        void OnSuccess(const std::string &networkId, const std::vector<DHDescriptor> &descriptors,
            EnableStep enableStep) override;
        void OnError(const std::string &networkId, int32_t error) override;
    };
};

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

void ResourceManagerTest::TestGetDistributedHardwareCallback::OnSuccess(
    const std::string &networkId, const std::vector<DHDescriptor> &descriptors, EnableStep enableStep)
{
    (void)networkId;
    (void)descriptors;
    (void)enableStep;
}

void ResourceManagerTest::TestGetDistributedHardwareCallback::OnError(const std::string &networkId, int32_t error)
{
    (void)networkId;
    (void)error;
}

/**
 * @tc.name: resource_manager_test_001
 * @tc.desc: Verify the CapabilityInfoManager UnInit function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_001, TestSize.Level1)
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
HWTEST_F(ResourceManagerTest, resource_manager_test_002, TestSize.Level1)
{
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->Init(), DH_FWK_SUCCESS);
}

/**
 * @tc.name: resource_manager_test_003
 * @tc.desc: Verify the CapabilityInfoManager SyncDeviceInfoFromDB function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_003, TestSize.Level1)
{
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_.clear();
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->SyncDeviceInfoFromDB(DEV_ID_0), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->globalCapInfoMap_.size(), TEST_SIZE_5);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->SyncDeviceInfoFromDB(DEV_ID_1), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->globalCapInfoMap_.size(), TEST_SIZE_10);
}

/**
 * @tc.name: resource_manager_test_005
 * @tc.desc: Verify the CapabilityInfoManager AddCapability function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_005, TestSize.Level1)
{
    vector<shared_ptr<CapabilityInfo>> resInfos { CAP_INFO_0, CAP_INFO_1, CAP_INFO_2, CAP_INFO_3, CAP_INFO_4,
        CAP_INFO_5, CAP_INFO_6, CAP_INFO_7, CAP_INFO_8, CAP_INFO_9 };
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->AddCapability(resInfos), DH_FWK_SUCCESS);
    EXPECT_EQ(CapabilityInfoManager::GetInstance()->globalCapInfoMap_.size(), TEST_SIZE_10);
}

/**
 * @tc.name: resource_manager_test_007
 * @tc.desc: Verify the CapabilityInfoManager RemoveCapabilityInfoByKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_007, TestSize.Level1)
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
 * @tc.name: resource_manager_test_010
 * @tc.desc: Verify the GetCapabilitiesByDeviceId function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_010, TestSize.Level1)
{
    vector<shared_ptr<CapabilityInfo>> capInfos;
    CapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId("", capInfos);
    EXPECT_EQ(capInfos.empty(), true);
    CapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(DEV_ID_0, capInfos);
    EXPECT_EQ(capInfos.size(), TEST_SIZE_5);
    CapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(DEV_ID_1, capInfos);
    EXPECT_EQ(capInfos.size(), TEST_SIZE_10);
}

/**
 * @tc.name: resource_manager_test_012
 * @tc.desc: Verify the GetDataByKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_012, TestSize.Level1)
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
HWTEST_F(ResourceManagerTest, resource_manager_test_013, TestSize.Level1)
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
 * @tc.name: resource_manager_test_016
 * @tc.desc: Verify the GetCapabilityKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_016, TestSize.Level1)
{
    std::string deviceId = "deviceIdtest";
    std::string dhId = "dhIdtest";
    std::string str = GetCapabilityKey(deviceId, dhId);
    EXPECT_EQ("deviceIdtest###dhIdtest", str);
}

/**
 * @tc.name: resource_manager_test_017
 * @tc.desc: Verify the IsCapKeyMatchDeviceId function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_017, TestSize.Level1)
{
    std::string key = "keytest";
    bool ret = IsCapKeyMatchDeviceId(key, DEV_ID_0);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: resource_manager_test_018
 * @tc.desc: Verify the IsCapKeyMatchDeviceId function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_018, TestSize.Level1)
{
    std::string key = "bb536a637105409e904d4da83790a4a7###keytest";
    bool ret = IsCapKeyMatchDeviceId(key, DEV_ID_0);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: resource_manager_test_019
 * @tc.desc: Verify the FromJsonString function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_019, TestSize.Level1)
{
    CapabilityInfo capaInfo;
    std::string jsonStr = "";
    int32_t ret = capaInfo.FromJsonString(jsonStr);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    jsonStr = "invalid JSON string";
    ret = capaInfo.FromJsonString(jsonStr);
    EXPECT_EQ(ERR_DH_FWK_JSON_PARSE_FAILED, ret);
}

/**
 * @tc.name: resource_manager_test_020
 * @tc.desc: Verify the Compare function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, resource_manager_test_020, TestSize.Level1)
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
HWTEST_F(ResourceManagerTest, OnChange_001, TestSize.Level1)
{
    std::vector<DistributedKv::Entry> inserts, updates, deleteds;
    DistributedKv::ChangeNotification changeIn(std::move(inserts), std::move(updates), std::move(deleteds), "", true);
    ASSERT_NO_FATAL_FAILURE(CapabilityInfoManager::GetInstance()->OnChange(changeIn));
}

HWTEST_F(ResourceManagerTest, OnChange_002, TestSize.Level1)
{
    DistributedKv::Entry insert, update, del;
    std::vector<DistributedKv::Entry> inserts, updates, deleteds;
    std::string tempStr;
    for (int32_t i = 1; i < MAX_DB_RECORD_LENGTH; i++) {
        tempStr = std::to_string(i);
        insert.key = tempStr.c_str();
        update.key = tempStr.c_str();
        del.key = tempStr.c_str();
        insert.value = tempStr.c_str();
        update.value = tempStr.c_str();
        del.value = tempStr.c_str();
        inserts.push_back(insert);
        updates.push_back(update);
        deleteds.push_back(del);
    }
    DistributedKv::ChangeNotification changeIn(std::move(inserts), std::move(updates), std::move(deleteds), "", true);
    ASSERT_NO_FATAL_FAILURE(CapabilityInfoManager::GetInstance()->OnChange(changeIn));
}

HWTEST_F(ResourceManagerTest, OnChange_003, TestSize.Level1)
{
    DistributedKv::Entry insert, update, del;
    insert.key = "insert_key";
    update.key = "update_key";
    del.key = "del_key";
    insert.value = "insert_value";
    update.value = "update_value";
    del.value = "del_value";
    std::vector<DistributedKv::Entry> inserts, updates, deleteds;
    inserts.push_back(insert);
    updates.push_back(update);
    deleteds.push_back(del);

    DistributedKv::ChangeNotification changeIn(std::move(inserts), std::move(updates), std::move(deleteds), "", true);
    ASSERT_NO_FATAL_FAILURE(CapabilityInfoManager::GetInstance()->OnChange(changeIn));
}

/**
 * @tc.name: OnChange_002
 * @tc.desc: Verify the OnChange function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, OnChange_004, TestSize.Level1)
{
    DistributedKv::DataOrigin origin;
    origin.id = {};
    origin.store = "";
    CapabilityInfoManager::Keys keys;
    keys[CapabilityInfoManager::OP_INSERT] = {"strBase"};
    keys[CapabilityInfoManager::OP_UPDATE] = {"strBase"};
    keys[CapabilityInfoManager::OP_DELETE] = {"strBase"};
    ASSERT_NO_FATAL_FAILURE(CapabilityInfoManager::GetInstance()->OnChange(origin, std::move(keys)));
}

/**
 * @tc.name: HandleCapabilityChange
 * @tc.desc: Verify the HandleCapabilityAddChange  HandleCapabilityUpdateChange HandleCapabilityDeleteChange function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ResourceManagerTest, HandleCapabilityChange_001, TestSize.Level1)
{
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, DH_ID, "111111");
    cJSON_AddStringToObject(jsonObj, DEV_ID, "222222");
    char* cjson = cJSON_PrintUnformatted(jsonObj);
    if (cjson == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    std::string jsonStr(cjson);
    std::vector<DistributedKv::Entry> insertRecords;
    DistributedKv::Entry entry;
    entry.key = "insert";
    entry.value = jsonStr.c_str();
    insertRecords.push_back(entry);
    ASSERT_NO_FATAL_FAILURE(CapabilityInfoManager::GetInstance()->HandleCapabilityAddChange(insertRecords));

    std::vector<DistributedKv::Entry> updateRecords;
    DistributedKv::Entry update;
    update.key = "update";
    update.value = jsonStr.c_str();
    updateRecords.push_back(update);
    ASSERT_NO_FATAL_FAILURE(CapabilityInfoManager::GetInstance()->HandleCapabilityUpdateChange(updateRecords));

    std::vector<DistributedKv::Entry> deleteRecords;
    DistributedKv::Entry del;
    del.key = "delete";
    del.value = jsonStr.c_str();
    deleteRecords.push_back(del);
    ASSERT_NO_FATAL_FAILURE(CapabilityInfoManager::GetInstance()->HandleCapabilityDeleteChange(deleteRecords));
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
}

HWTEST_F(ResourceManagerTest, HandleCapabilityChange_002, TestSize.Level1)
{
    std::string uuid = "123456789";
    std::string deviceId = Sha256(uuid);
    DeviceIdEntry idEntry = {
        .networkId = "",
        .uuid = uuid,
        .deviceId = deviceId
    };
    DHContext::GetInstance().devIdEntrySet_.insert(idEntry);
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, DH_ID, "111111");
    cJSON_AddStringToObject(jsonObj, DEV_ID, deviceId.c_str());
    char* cjson = cJSON_PrintUnformatted(jsonObj);
    if (cjson == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    std::string jsonStr(cjson);
    std::vector<DistributedKv::Entry> insertRecords;
    DistributedKv::Entry entry;
    entry.key = "insert";
    entry.value = jsonStr.c_str();
    insertRecords.push_back(entry);
    ASSERT_NO_FATAL_FAILURE(CapabilityInfoManager::GetInstance()->HandleCapabilityAddChange(insertRecords));

    std::vector<DistributedKv::Entry> updateRecords;
    DistributedKv::Entry update;
    update.key = "update";
    update.value = jsonStr.c_str();
    updateRecords.push_back(update);
    ASSERT_NO_FATAL_FAILURE(CapabilityInfoManager::GetInstance()->HandleCapabilityUpdateChange(updateRecords));

    std::vector<DistributedKv::Entry> deleteRecords;
    DistributedKv::Entry del;
    del.key = "delete";
    del.value = jsonStr.c_str();
    deleteRecords.push_back(del);
    ASSERT_NO_FATAL_FAILURE(CapabilityInfoManager::GetInstance()->HandleCapabilityDeleteChange(deleteRecords));
    DHContext::GetInstance().devIdEntrySet_.clear();
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
}

HWTEST_F(ResourceManagerTest, HandleCapabilityChange_003, TestSize.Level1)
{
    std::string uuid = "123456789";
    std::string deviceId = Sha256(uuid);
    DHContext::GetInstance().AddOnlineDevice("111111", uuid, "222222");
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, DH_ID, "111111");
    cJSON_AddStringToObject(jsonObj, DEV_ID, deviceId.c_str());
    char* cjson = cJSON_PrintUnformatted(jsonObj);
    if (cjson == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    std::string jsonStr(cjson);
    std::vector<DistributedKv::Entry> insertRecords;
    DistributedKv::Entry entry;
    entry.key = "insert";
    entry.value = jsonStr.c_str();
    insertRecords.push_back(entry);
    ASSERT_NO_FATAL_FAILURE(CapabilityInfoManager::GetInstance()->HandleCapabilityAddChange(insertRecords));

    std::vector<DistributedKv::Entry> updateRecords;
    DistributedKv::Entry update;
    update.key = "update";
    update.value = jsonStr.c_str();
    updateRecords.push_back(update);
    ASSERT_NO_FATAL_FAILURE(CapabilityInfoManager::GetInstance()->HandleCapabilityUpdateChange(updateRecords));

    std::string enabledDeviceKey = deviceId + RESOURCE_SEPARATOR + "111111";
    TaskParam taskParam;
    TaskBoard::GetInstance().SaveEnabledDevice(enabledDeviceKey, taskParam);
    ASSERT_NO_FATAL_FAILURE(CapabilityInfoManager::GetInstance()->HandleCapabilityUpdateChange(updateRecords));

    std::vector<DistributedKv::Entry> deleteRecords;
    DistributedKv::Entry del;
    del.key = "delete";
    del.value = jsonStr.c_str();
    deleteRecords.push_back(del);
    ASSERT_NO_FATAL_FAILURE(CapabilityInfoManager::GetInstance()->HandleCapabilityDeleteChange(deleteRecords));
    DHContext::GetInstance().devIdEntrySet_.clear();
    TaskBoard::GetInstance().RemoveEnabledDevice(enabledDeviceKey);
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
}

/**
 * @tc.name: GetCapabilitiesByDeviceId_001
 * @tc.desc: Verify the GetCapabilitiesByDeviceId function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ResourceManagerTest, GetCapabilitiesByDeviceId_001, TestSize.Level1)
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
HWTEST_F(ResourceManagerTest, HasCapability_001, TestSize.Level1)
{
    std::string deviceId;
    std::string dhId;
    bool ret = CapabilityInfoManager::GetInstance()->HasCapability(deviceId, dhId);
    EXPECT_EQ(false, ret);

    deviceId = "123456";
    ret = CapabilityInfoManager::GetInstance()->HasCapability(deviceId, dhId);
    EXPECT_EQ(false, ret);

    dhId = "111111";
    deviceId = "";
    ret = CapabilityInfoManager::GetInstance()->HasCapability(deviceId, dhId);
    EXPECT_EQ(false, ret);
}

HWTEST_F(ResourceManagerTest, HasCapability_002, TestSize.Level1)
{
    std::string devId = "123456";
    std::string dhID = "111111";
    std::string key = DEV_ID_0 + RESOURCE_SEPARATOR + DH_ID_0;
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = CAP_INFO_0;
    bool ret = CapabilityInfoManager::GetInstance()->HasCapability(devId, dhID);
    EXPECT_EQ(false, ret);

    ret = CapabilityInfoManager::GetInstance()->HasCapability(DEV_ID_0, DH_ID_0);
    EXPECT_EQ(true, ret);
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_.clear();
}

/**
 * @tc.name: SyncRemoteCapabilityInfos_001
 * @tc.desc: Verify the CapabilityInfoManager SyncRemoteCapabilityInfos function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, SyncRemoteCapabilityInfos_001, TestSize.Level1)
{
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_.clear();
    CapabilityInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    int32_t ret = CapabilityInfoManager::GetInstance()->SyncRemoteCapabilityInfos();
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

HWTEST_F(ResourceManagerTest, SyncRemoteCapabilityInfos_002, TestSize.Level1)
{
    CapabilityInfoManager::GetInstance()->Init();
    DHContext::GetInstance().AddOnlineDevice("111111", "222222", "333333");
    int32_t ret = CapabilityInfoManager::GetInstance()->SyncRemoteCapabilityInfos();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    CapabilityInfoManager::GetInstance()->UnInit();
    DHContext::GetInstance().RemoveOnlineDeviceIdEntryByNetworkId("333333");
}

/**
 * @tc.name: RemoveCapabilityInfoInDB_001
 * @tc.desc: Verify the RemoveCapabilityInfoInDB function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, RemoveCapabilityInfoInDB_001, TestSize.Level1)
{
    std::string deviceId = "";
    int32_t ret = CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoInDB(deviceId);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: RemoveCapabilityInfoInDB_002
 * @tc.desc: Verify the RemoveCapabilityInfoInDB function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, RemoveCapabilityInfoInDB_002, TestSize.Level1)
{
    std::string deviceId = "deviceIdEmpty";
    CapabilityInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    int32_t ret = CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoInDB(deviceId);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

HWTEST_F(ResourceManagerTest, RemoveCapabilityInfoInDB_003, TestSize.Level1)
{
    CapabilityInfoManager::GetInstance()->Init();
    std::string deviceId = "123456";
    std::string dhId = "111111";
    std::string key = deviceId + RESOURCE_SEPARATOR + dhId;
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = CAP_INFO_0;
    deviceId = "456789";
    int32_t ret = CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoInDB(deviceId);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    ret = CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoInMem(deviceId);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    key = deviceId + RESOURCE_SEPARATOR + dhId;
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = CAP_INFO_1;
    ret = CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoInDB(deviceId);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    deviceId = "123456";
    ret = CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoInMem(deviceId);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_.clear();
    CapabilityInfoManager::GetInstance()->UnInit();
}

HWTEST_F(ResourceManagerTest, RemoveCapabilityInfoByKey_001, TestSize.Level1)
{
    std::string key = "";
    int32_t ret = CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(key);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: HandleCapabilityDeleteChange_001
 * @tc.desc: Verify the HandleCapabilityDeleteChange function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, HandleCapabilityDeleteChange_001, TestSize.Level1)
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
HWTEST_F(ResourceManagerTest, GetDataByKey_001, TestSize.Level1)
{
    std::string key = "000";
    std::shared_ptr<CapabilityInfo> capInfoPtr;
    CapabilityInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    int32_t ret = CapabilityInfoManager::GetInstance()->GetDataByKey(key, capInfoPtr);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);

    key = "";
    CapabilityInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    ret = CapabilityInfoManager::GetInstance()->GetDataByKey(key, capInfoPtr);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: GetDataByKeyPrefix_001
 * @tc.desc: Verify the GetDataByKeyPrefix function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, GetDataByKeyPrefix_001, TestSize.Level1)
{
    std::string keyPrefix = "000";
    CapabilityInfoMap capabilityMap;
    CapabilityInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    int32_t ret = CapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(keyPrefix, capabilityMap);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);

    keyPrefix = "";
    CapabilityInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    ret = CapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(keyPrefix, capabilityMap);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: DumpCapabilityInfos_001
 * @tc.desc: Verify the DumpCapabilityInfos function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, DumpCapabilityInfos_001, TestSize.Level1)
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
 * @tc.name: GetEntriesByKeys_001
 * @tc.desc: Verify the GetEntriesByKeys function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(ResourceManagerTest, GetEntriesByKeys_001, TestSize.Level1)
{
    std::vector<std::string> keys {};
    auto entries = CapabilityInfoManager::GetInstance()->GetEntriesByKeys(keys);
    EXPECT_EQ(0, entries.size());

    CapabilityInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    std::string key = "key_1";
    keys.push_back(key);
    entries = CapabilityInfoManager::GetInstance()->GetEntriesByKeys(keys);
    EXPECT_EQ(0, entries.size());
}

HWTEST_F(ResourceManagerTest, SyncDeviceInfoFromDB_001, TestSize.Level1)
{
    std::string deviceId = "";
    auto ret = CapabilityInfoManager::GetInstance()->SyncDeviceInfoFromDB(deviceId);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    deviceId = "123456";
    ret = CapabilityInfoManager::GetInstance()->SyncDeviceInfoFromDB(deviceId);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

HWTEST_F(ResourceManagerTest, AddCapability_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<CapabilityInfo>> resInfos;
    for (int32_t i = 1; i < MAX_DB_RECORD_LENGTH; i++) {
        std::shared_ptr<CapabilityInfo> capInfoTest = make_shared<CapabilityInfo>(std::to_string(i), std::to_string(i),
            TEST_DEV_NAME, TEST_DEV_TYPE_PAD, DHType::CAMERA, DH_ATTR_0, DH_SUBTYPE_0);
        resInfos.push_back(capInfoTest);
    }
    auto ret = CapabilityInfoManager::GetInstance()->AddCapability(resInfos);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_RES_DB_DATA_INVALID, ret);
}

HWTEST_F(ResourceManagerTest, AddCapability_002, TestSize.Level1)
{
    CapabilityInfoManager::GetInstance()->Init();
    std::vector<std::shared_ptr<CapabilityInfo>> resInfos;
    std::shared_ptr<CapabilityInfo> capInfoTest = nullptr;
    resInfos.push_back(capInfoTest);
    auto ret = CapabilityInfoManager::GetInstance()->AddCapability(resInfos);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    CapabilityInfoManager::GetInstance()->UnInit();
}

HWTEST_F(ResourceManagerTest, AddCapabilityInMem_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<CapabilityInfo>> resInfos;
    auto ret = CapabilityInfoManager::GetInstance()->AddCapabilityInMem(resInfos);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_RES_DB_DATA_INVALID, ret);
}

HWTEST_F(ResourceManagerTest, AddCapabilityInMem_002, TestSize.Level1)
{
    std::vector<std::shared_ptr<CapabilityInfo>> resInfos;
    for (int32_t i = 1; i < MAX_DB_RECORD_LENGTH; i++) {
        std::shared_ptr<CapabilityInfo> capInfoTest = make_shared<CapabilityInfo>(std::to_string(i), std::to_string(i),
            TEST_DEV_NAME, TEST_DEV_TYPE_PAD, DHType::CAMERA, DH_ATTR_0, DH_SUBTYPE_0);
        resInfos.push_back(capInfoTest);
    }
    auto ret = CapabilityInfoManager::GetInstance()->AddCapabilityInMem(resInfos);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_RES_DB_DATA_INVALID, ret);
}

HWTEST_F(ResourceManagerTest, AddCapabilityInMem_003, TestSize.Level1)
{
    std::vector<shared_ptr<CapabilityInfo>> resInfos;
    resInfos.push_back(CAP_INFO_0);
    auto ret = CapabilityInfoManager::GetInstance()->AddCapabilityInMem(resInfos);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    std::shared_ptr<CapabilityInfo> capInfoTest = nullptr;
    resInfos.push_back(capInfoTest);
    ret = CapabilityInfoManager::GetInstance()->AddCapabilityInMem(resInfos);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(ResourceManagerTest, GetCapability_001, TestSize.Level1)
{
    std::string deviceId;
    std::string dhId;
    std::shared_ptr<CapabilityInfo> capPtr;
    auto ret = CapabilityInfoManager::GetInstance()->GetCapability(deviceId, dhId, capPtr);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = CapabilityInfoManager::GetInstance()->GetCapability(DEV_ID_0, dhId, capPtr);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = CapabilityInfoManager::GetInstance()->GetCapability(deviceId, DH_ID_0, capPtr);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(ResourceManagerTest, GetCapability_002, TestSize.Level1)
{
    std::string deviceId = "123456";
    std::string dhId = "111111";
    std::shared_ptr<CapabilityInfo> capPtr;
    std::string key = DEV_ID_0 + RESOURCE_SEPARATOR + DH_ID_0;
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = CAP_INFO_0;
    auto ret = CapabilityInfoManager::GetInstance()->GetCapability(deviceId, dhId, capPtr);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_CAPABILITY_MAP_NOT_FOUND, ret);

    ret = CapabilityInfoManager::GetInstance()->GetCapability(DEV_ID_0, DH_ID_0, capPtr);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(ResourceManagerTest, GetDataByDHType_001, TestSize.Level1)
{
    std::string key = DEV_ID_0 + RESOURCE_SEPARATOR + DH_ID_0;
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = CAP_INFO_0;
    CapabilityInfoMap capabilityMap;
    auto ret = CapabilityInfoManager::GetInstance()->GetDataByDHType(DHType::AUDIO, capabilityMap);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    ret = CapabilityInfoManager::GetInstance()->GetDataByDHType(DHType::CAMERA, capabilityMap);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_.clear();
}

HWTEST_F(ResourceManagerTest, GetDhSubtype_001, TestSize.Level1)
{
    std::string deviceId = "";
    std::string dhId = "";
    auto ret = CapabilityInfoManager::GetInstance()->GetDhSubtype(deviceId, dhId);
    EXPECT_EQ("", ret);

    deviceId = "deviceId_1";
    ret = CapabilityInfoManager::GetInstance()->GetDhSubtype(deviceId, dhId);
    EXPECT_EQ("", ret);

    deviceId = "";
    dhId = "dhId_1";
    ret = CapabilityInfoManager::GetInstance()->GetDhSubtype(deviceId, dhId);
    EXPECT_EQ("", ret);
}

HWTEST_F(ResourceManagerTest, GetDhSubtype_002, TestSize.Level1)
{
    std::string peeruuid = "123456789";
    std::string dhid = "audio_132";
    std::string deviceId = Sha256(peeruuid);

    std::shared_ptr<CapabilityInfo> capInfo = std::make_shared<CapabilityInfo>(
        dhid, deviceId, "devName_test", TEST_DEV_TYPE_PAD, DHType::AUDIO, "attrs", "subtype");
    std::string key = deviceId + "###" + dhid;
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capInfo;
    auto ret = CapabilityInfoManager::GetInstance()->GetDhSubtype("deviceId_1", dhid);
    EXPECT_EQ("", ret);

    ret = CapabilityInfoManager::GetInstance()->GetDhSubtype(deviceId, dhid);
    EXPECT_EQ("subtype", ret);

    std::shared_ptr<CapabilityInfo> capInfo1 = nullptr;
    std::string deviceId1 = "deviceId_2";
    std::string key1 = deviceId1 + "###" + dhid;
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_[key1] = capInfo1;
    ret = CapabilityInfoManager::GetInstance()->GetDhSubtype(deviceId1, dhid);
    EXPECT_EQ("", ret);
}
} // namespace DistributedHardware
} // namespace OHOS
