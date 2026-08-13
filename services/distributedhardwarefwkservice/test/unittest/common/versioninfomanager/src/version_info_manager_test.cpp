/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "version_info_manager_test.h"

#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>

#include "version_info_manager.h"
#include "version_manager.h"
#include "dh_context.h"
#include "distributed_hardware_log.h"
#include "dh_utils_tool.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "VersionInfoManagerTest"

namespace {
const string DATABASE_DIR = "/data/service/el1/public/database/dtbhardware_manager_service/";
const string DEV_ID_1 = "bb536a637105409e904d4da83790a4a7";
const string DEV_ID_2 = "bb536a637105409e904d4da83790a4a8";
const string DEV_ID_3 = "bb536a637105409e904d4da83790a4a9";
const string NAME_SCREEN = "distributed_screen";
const string NAME_CAMERA = "distributed_camera";
const string NAME_INPUT  = "distributed_input";
const string VERSION_1 = "1.0";
const string VERSION_2 = "2.0";
const string VERSION_3 = "3.0";
std::vector<VersionInfo> g_versionInfos;
constexpr uint32_t MAX_DB_RECORD_LENGTH = 10005;
}

std::vector<VersionInfo> CreateVersionInfos()
{
    CompVersion compVersions1 = {
        .name = NAME_CAMERA,
        .dhType = DHType::CAMERA,
        .handlerVersion = VERSION_1,
        .sourceVersion = VERSION_1,
        .sinkVersion = VERSION_1
    };

    CompVersion compVersions2 = {
        .name = NAME_SCREEN,
        .dhType = DHType::SCREEN,
        .handlerVersion = VERSION_2,
        .sourceVersion = VERSION_2,
        .sinkVersion = VERSION_2
    };

    CompVersion compVersions3 = {
        .name = NAME_INPUT,
        .dhType = DHType::INPUT,
        .handlerVersion = VERSION_3,
        .sourceVersion = VERSION_3,
        .sinkVersion = VERSION_3
    };

    VersionInfo verInfo1;
    verInfo1.deviceId = DEV_ID_1;
    verInfo1.dhVersion = VERSION_1;
    verInfo1.compVersions.insert(std::pair<DHType, CompVersion>(compVersions1.dhType, compVersions1));
    verInfo1.compVersions.insert(std::pair<DHType, CompVersion>(compVersions1.dhType, compVersions1));
    verInfo1.compVersions.insert(std::pair<DHType, CompVersion>(compVersions1.dhType, compVersions1));

    VersionInfo verInfo2;
    verInfo2.deviceId = DEV_ID_2;
    verInfo1.dhVersion = VERSION_2;
    verInfo1.compVersions.insert(std::pair<DHType, CompVersion>(compVersions2.dhType, compVersions2));
    verInfo1.compVersions.insert(std::pair<DHType, CompVersion>(compVersions2.dhType, compVersions2));
    verInfo1.compVersions.insert(std::pair<DHType, CompVersion>(compVersions2.dhType, compVersions2));

    VersionInfo verInfo3;
    verInfo3.deviceId = DEV_ID_3;
    verInfo1.dhVersion = VERSION_3;
    verInfo1.compVersions.insert(std::pair<DHType, CompVersion>(compVersions3.dhType, compVersions3));
    verInfo1.compVersions.insert(std::pair<DHType, CompVersion>(compVersions3.dhType, compVersions3));
    verInfo1.compVersions.insert(std::pair<DHType, CompVersion>(compVersions3.dhType, compVersions3));

    return std::vector<VersionInfo> { verInfo1, verInfo2, verInfo3 };
}

void VersionInfoManagerTest::SetUpTestCase(void)
{
    auto ret = mkdir(DATABASE_DIR.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (ret != 0) {
        DHLOGE("mkdir failed, path: %{public}s, errno : %{public}d", DATABASE_DIR.c_str(), errno);
    }

    g_versionInfos = CreateVersionInfos();
}

void VersionInfoManagerTest::TearDownTestCase(void)
{
    auto ret = remove(DATABASE_DIR.c_str());
    if (ret != 0) {
        DHLOGE("remove dir failed, path: %{public}s, errno : %{public}d", DATABASE_DIR.c_str(), errno);
    }
}

void VersionInfoManagerTest::SetUp()
{
}

void VersionInfoManagerTest::TearDown()
{
}

/**
 * @tc.name:version_info_manager_test_001
 * @tc.desc: Verify the VersionInfoManager Init function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(VersionInfoManagerTest, version_info_manager_test_001, TestSize.Level1)
{
    EXPECT_EQ(VersionInfoManager::GetInstance()->Init(), DH_FWK_SUCCESS);
}

/**
 * @tc.name:version_info_manager_test_002
 * @tc.desc: Verify the VersionInfoManager AddVersion function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(VersionInfoManagerTest, version_info_manager_test_002, TestSize.Level1)
{
    for (const auto& verInfo : g_versionInfos) {
        EXPECT_EQ(VersionInfoManager::GetInstance()->AddVersion(verInfo), DH_FWK_SUCCESS);
    }
}

/**
 * @tc.name:version_info_manager_test_003
 * @tc.desc: Verify the VersionInfoManager GetVersionInfoByDeviceId function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(VersionInfoManagerTest, version_info_manager_test_003, TestSize.Level1)
{
    VersionInfo versionInfo;
    for (const auto& verInfo : g_versionInfos) {
        EXPECT_EQ(VersionInfoManager::GetInstance()->GetVersionInfoByDeviceId(verInfo.deviceId, versionInfo),
            DH_FWK_SUCCESS);
    }
}

/**
 * @tc.name:version_info_manager_test_004
 * @tc.desc: Verify the VersionInfoManager SyncVersionInfoFromDB function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(VersionInfoManagerTest, version_info_manager_test_004, TestSize.Level1)
{
    EXPECT_EQ(VersionInfoManager::GetInstance()->SyncVersionInfoFromDB(DEV_ID_1), DH_FWK_SUCCESS);
}

/**
 * @tc.name:version_info_manager_test_005
 * @tc.desc: Verify the VersionInfoManager UnInit function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(VersionInfoManagerTest, version_info_manager_test_005, TestSize.Level1)
{
    EXPECT_EQ(VersionInfoManager::GetInstance()->UnInit(), DH_FWK_SUCCESS);
}

HWTEST_F(VersionInfoManagerTest, version_info_manager_test_006, TestSize.Level1)
{
    VersionInfoManager::GetInstance()->dbAdapterPtr_= nullptr;
    auto ret = VersionInfoManager::GetInstance()->UnInit();
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_UNINIT_DB_FAILED, ret);
}

/**
 * @tc.name: UpdateVersionCache_001
 * @tc.desc: Verify the UpdateVersionCache function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, UpdateVersionCache_001, TestSize.Level1)
{
    VersionInfo versionInfo;
    versionInfo.deviceId = "deviceId";
    versionInfo.dhVersion = "dhVersion";
    VersionInfoManager::GetInstance()->UpdateVersionCache(versionInfo);
    EXPECT_EQ(DH_FWK_SUCCESS, VersionInfoManager::GetInstance()->Init());
}

/**
 * @tc.name: UpdateVersionCache_002
 * @tc.desc: Verify the UpdateVersionCache function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, UpdateVersionCache_002, TestSize.Level1)
{
    std::string testUdid = "111111";
    std::string testUuid = "222222";
    std::string testNetworkId = "333333";
    std::string testDeviceId = Sha256(testUuid);
    VersionInfo versionInfo = {
        .deviceId = testDeviceId,
        .dhVersion = "1.0"
    };
    DHContext::GetInstance().AddOnlineDevice(testUdid, testUuid, testNetworkId);
    VersionInfoManager::GetInstance()->UpdateVersionCache(versionInfo);
    EXPECT_FALSE(versionInfo.dhVersion.empty());
}

/**
 * @tc.name: RemoveVersionInfoByDeviceId_001
 * @tc.desc: Verify the RemoveVersionInfoByDeviceId function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, RemoveVersionInfoByDeviceId_001, TestSize.Level1)
{
    std::string deviceId = DEV_ID_1;
    VersionInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    int32_t ret = VersionInfoManager::GetInstance()->RemoveVersionInfoByDeviceId(deviceId);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);

    deviceId = "";
    VersionInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    ret = VersionInfoManager::GetInstance()->RemoveVersionInfoByDeviceId(deviceId);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: RemoveVersionInfoByDeviceId_002
 * @tc.desc: Verify the RemoveVersionInfoByDeviceId function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, RemoveVersionInfoByDeviceId_002, TestSize.Level1)
{
    std::string deviceId = "deviceId";
    std::string appId;
    std::string storeId;
    std::shared_ptr<DistributedKv::KvStoreObserver> changeListener = nullptr;
    VersionInfoManager::GetInstance()->dbAdapterPtr_ =
        std::make_shared<MockDBAdapter>(appId.c_str(), storeId.c_str(), changeListener);
    int32_t ret =  VersionInfoManager::GetInstance()->RemoveVersionInfoByDeviceId(deviceId);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL, ret);
}

/**
 * @tc.name: SyncVersionInfoFromDB_001
 * @tc.desc: Verify the SyncVersionInfoFromDB function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, SyncVersionInfoFromDB_001, TestSize.Level1)
{
    std::string deviceId = DEV_ID_1;
    VersionInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    int32_t ret = VersionInfoManager::GetInstance()->SyncVersionInfoFromDB(deviceId);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);

    deviceId = "";
    VersionInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    ret = VersionInfoManager::GetInstance()->SyncVersionInfoFromDB(deviceId);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: SyncVersionInfoFromDB_002
 * @tc.desc: Verify the SyncVersionInfoFromDB function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, SyncVersionInfoFromDB_002, TestSize.Level1)
{
    std::string deviceId = "deviceId";
    std::string appId;
    std::string storeId;
    std::shared_ptr<DistributedKv::KvStoreObserver> changeListener = nullptr;
    VersionInfoManager::GetInstance()->dbAdapterPtr_ =
        std::make_shared<MockDBAdapter>(appId.c_str(), storeId.c_str(), changeListener);
    int32_t ret =  VersionInfoManager::GetInstance()->SyncVersionInfoFromDB(deviceId);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL, ret);
}

/**
 * @tc.name: SyncVersionInfoFromDB_003
 * @tc.desc: Verify the SyncVersionInfoFromDB function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, SyncVersionInfoFromDB_003, TestSize.Level1)
{
    std::string deviceId = "device";
    int32_t ret =  VersionInfoManager::GetInstance()->SyncVersionInfoFromDB(deviceId);
    EXPECT_NE(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: SyncRemoteVersionInfos_001
 * @tc.desc: Verify the SyncRemoteVersionInfos function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, SyncRemoteVersionInfos_001, TestSize.Level1)
{
    VersionInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    int32_t ret = VersionInfoManager::GetInstance()->SyncRemoteVersionInfos();
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

/**
 * @tc.name: SyncRemoteVersionInfos_002
 * @tc.desc: Verify the SyncRemoteVersionInfos function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, SyncRemoteVersionInfos_002, TestSize.Level1)
{
    std::string udid = "udid_123456";
    std::string uuid = "uuid_123456";
    std::string networkId = "networkId_123456";
    DHContext::GetInstance().AddOnlineDevice(udid, uuid, networkId);
    std::string appId;
    std::string storeId;
    std::shared_ptr<DistributedKv::KvStoreObserver> changeListener = nullptr;
    VersionInfoManager::GetInstance()->dbAdapterPtr_ =
        std::make_shared<MockDBAdapter>(appId.c_str(), storeId.c_str(), changeListener);
    int32_t ret = VersionInfoManager::GetInstance()->SyncRemoteVersionInfos();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: OnChange_001
 * @tc.desc: Verify the OnChange function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, OnChange_001, TestSize.Level1)
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
    ASSERT_NO_FATAL_FAILURE(VersionInfoManager::GetInstance()->OnChange(changeIn));
}

HWTEST_F(VersionInfoManagerTest, OnChange_002, TestSize.Level1)
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
    ASSERT_NO_FATAL_FAILURE(VersionInfoManager::GetInstance()->OnChange(changeIn));
}

HWTEST_F(VersionInfoManagerTest, OnChange_003, TestSize.Level1)
{
    DistributedKv::Entry insert, update, del;
    std::vector<DistributedKv::Entry> inserts, updates, deleteds;
    inserts.push_back(insert);
    updates.push_back(update);
    deleteds.push_back(del);
    DistributedKv::ChangeNotification changeIn(std::move(inserts), std::move(updates), std::move(deleteds), "", true);
    ASSERT_NO_FATAL_FAILURE(VersionInfoManager::GetInstance()->OnChange(changeIn));
}

/**
 * @tc.name: HandleVersionChange_001
 * @tc.desc: Verify the HandleVersionAddChange HandleVersionUpdateChange HandleVersionDeleteChange function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, HandleVersionChange_001, TestSize.Level1)
{
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, DEV_ID, "222222");
    cJSON_AddStringToObject(jsonObj, DH_VER, "1.0");
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
    ASSERT_NO_FATAL_FAILURE(VersionInfoManager::GetInstance()->HandleVersionAddChange(insertRecords));

    std::vector<DistributedKv::Entry> updateRecords;
    DistributedKv::Entry update;
    update.key = "update";
    update.value = jsonStr.c_str();
    updateRecords.push_back(update);
    ASSERT_NO_FATAL_FAILURE(VersionInfoManager::GetInstance()->HandleVersionUpdateChange(updateRecords));

    std::vector<DistributedKv::Entry> deleteRecords;
    DistributedKv::Entry del;
    del.key = "delete";
    del.value = jsonStr.c_str();
    deleteRecords.push_back(del);
    ASSERT_NO_FATAL_FAILURE(VersionInfoManager::GetInstance()->HandleVersionDeleteChange(deleteRecords));
}

/**
 * @tc.name: HandleVersionChange_002
 * @tc.desc: Verify the HandleVersionDeleteChange function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, HandleVersionChange_002, TestSize.Level1)
{
    std::string uuid = "123456789";
    DHContext::GetInstance().AddOnlineDevice("111111", uuid, "222222");
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, DEV_ID, "222222");
    cJSON_AddStringToObject(jsonObj, DH_VER, "1.0");
    char* cjson = cJSON_PrintUnformatted(jsonObj);
    if (cjson == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    std::string jsonStr(cjson);
    std::vector<DistributedKv::Entry> deleteRecords;
    DistributedKv::Entry del;
    del.key = "delete";
    del.value = jsonStr.c_str();
    deleteRecords.push_back(del);
    ASSERT_NO_FATAL_FAILURE(VersionInfoManager::GetInstance()->HandleVersionDeleteChange(deleteRecords));
}

/**
 * @tc.name: GetVersionInfoByDeviceId_001
 * @tc.desc: Verify the VersionInfoManager GetVersionInfoByDeviceId function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(VersionInfoManagerTest, GetVersionInfoByDeviceId_001, TestSize.Level1)
{
    std::string deviceId = DEV_ID_1;
    VersionInfo versionInfo;
    VersionInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    int32_t ret = VersionInfoManager::GetInstance()->GetVersionInfoByDeviceId(deviceId, versionInfo);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);

    deviceId = "";
    VersionInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    ret = VersionInfoManager::GetInstance()->GetVersionInfoByDeviceId(deviceId, versionInfo);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(VersionInfoManagerTest, GetVersionInfoByDeviceId_002, TestSize.Level1)
{
    std::string deviceId = DEV_ID_1;
    VersionInfo versionInfo;
    VersionInfoManager::GetInstance()->Init();
    int32_t ret = VersionInfoManager::GetInstance()->GetVersionInfoByDeviceId(deviceId, versionInfo);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
