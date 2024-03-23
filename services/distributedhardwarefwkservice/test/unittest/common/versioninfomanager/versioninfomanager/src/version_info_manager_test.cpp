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

#include "version_info_manager_test.h"

#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>

#define private public
#include "version_info_manager.h"
#include "version_manager.h"
#include "version_info_event.h"
#undef private
#include "dh_context.h"
#include "distributed_hardware_log.h"

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
HWTEST_F(VersionInfoManagerTest, version_info_manager_test_001, TestSize.Level0)
{
    EXPECT_EQ(VersionInfoManager::GetInstance()->Init(), DH_FWK_SUCCESS);
}

/**
 * @tc.name:version_info_manager_test_002
 * @tc.desc: Verify the VersionInfoManager AddVersion function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(VersionInfoManagerTest, version_info_manager_test_002, TestSize.Level0)
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
HWTEST_F(VersionInfoManagerTest, version_info_manager_test_003, TestSize.Level0)
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
HWTEST_F(VersionInfoManagerTest, version_info_manager_test_004, TestSize.Level0)
{
    EXPECT_EQ(VersionInfoManager::GetInstance()->SyncVersionInfoFromDB(DEV_ID_1), DH_FWK_SUCCESS);
}

/**
 * @tc.name:version_info_manager_test_005
 * @tc.desc: Verify the VersionInfoManager UnInit function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(VersionInfoManagerTest, version_info_manager_test_005, TestSize.Level0)
{
    EXPECT_EQ(VersionInfoManager::GetInstance()->UnInit(), DH_FWK_SUCCESS);
}

/**
 * @tc.name: UpdateVersionCache_001
 * @tc.desc: Verify the UpdateVersionCache function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, UpdateVersionCache_001, TestSize.Level0)
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
HWTEST_F(VersionInfoManagerTest, UpdateVersionCache_002, TestSize.Level0)
{
    VersionInfo versionInfo;
    VersionInfoManager::GetInstance()->UpdateVersionCache(versionInfo);
    EXPECT_EQ(DH_FWK_SUCCESS, VersionInfoManager::GetInstance()->Init());
}

/**
 * @tc.name: RemoveVersionInfoByDeviceId_001
 * @tc.desc: Verify the RemoveVersionInfoByDeviceId function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, RemoveVersionInfoByDeviceId_001, TestSize.Level0)
{
    std::string deviceId;
    VersionInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    int32_t ret = VersionInfoManager::GetInstance()->RemoveVersionInfoByDeviceId(deviceId);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

/**
 * @tc.name: RemoveVersionInfoByDeviceId_002
 * @tc.desc: Verify the RemoveVersionInfoByDeviceId function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, RemoveVersionInfoByDeviceId_002, TestSize.Level0)
{
    std::string deviceId = "deviceId";
    std::string appId;
    std::string storeId;
    std::shared_ptr<DistributedKv::KvStoreObserver> changeListener = nullptr;
    VersionInfoManager::GetInstance()->dbAdapterPtr_ = std::make_shared<MockDBAdapter>(appId, storeId, changeListener);
    int32_t ret =  VersionInfoManager::GetInstance()->RemoveVersionInfoByDeviceId(deviceId);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL, ret);
}

/**
 * @tc.name: SyncVersionInfoFromDB_001
 * @tc.desc: Verify the SyncVersionInfoFromDB function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, SyncVersionInfoFromDB_001, TestSize.Level0)
{
    std::string deviceId;
    VersionInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    int32_t ret = VersionInfoManager::GetInstance()->SyncVersionInfoFromDB(deviceId);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

/**
 * @tc.name: SyncVersionInfoFromDB_002
 * @tc.desc: Verify the SyncVersionInfoFromDB function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, SyncVersionInfoFromDB_002, TestSize.Level0)
{
    std::string deviceId = "deviceId";
    std::string appId;
    std::string storeId;
    std::shared_ptr<DistributedKv::KvStoreObserver> changeListener = nullptr;
    VersionInfoManager::GetInstance()->dbAdapterPtr_ = std::make_shared<MockDBAdapter>(appId, storeId, changeListener);
    int32_t ret =  VersionInfoManager::GetInstance()->SyncVersionInfoFromDB(deviceId);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL, ret);
}

/**
 * @tc.name: SyncVersionInfoFromDB_003
 * @tc.desc: Verify the SyncVersionInfoFromDB function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, SyncVersionInfoFromDB_003, TestSize.Level0)
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
HWTEST_F(VersionInfoManagerTest, SyncRemoteVersionInfos_001, TestSize.Level0)
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
HWTEST_F(VersionInfoManagerTest, SyncRemoteVersionInfos_002, TestSize.Level0)
{
    std::string appId;
    std::string storeId;
    std::shared_ptr<DistributedKv::KvStoreObserver> changeListener = nullptr;
    VersionInfoManager::GetInstance()->dbAdapterPtr_ = std::make_shared<MockDBAdapter>(appId, storeId, changeListener);
    int32_t ret = VersionInfoManager::GetInstance()->SyncRemoteVersionInfos();
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL, ret);
}

/**
 * @tc.name: OnChange_001
 * @tc.desc: Verify the OnChange function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, OnChange_001, TestSize.Level0)
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

    VersionInfoManager::GetInstance()->OnChange(changeIn);
    EXPECT_EQ(DH_FWK_SUCCESS, VersionInfoManager::GetInstance()->Init());
}

/**
 * @tc.name: HandleVersionAddChange_001
 * @tc.desc: Verify the HandleVersionAddChange function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, HandleVersionAddChange_001, TestSize.Level0)
{
    std::vector<DistributedKv::Entry> insertRecords;
    DistributedKv::Entry entry;
    entry.key = "strBase";
    entry.value = "strBase";
    insertRecords.push_back(entry);
    VersionInfoManager::GetInstance()->HandleVersionAddChange(insertRecords);
    EXPECT_EQ(DH_FWK_SUCCESS, VersionInfoManager::GetInstance()->Init());
}

/**
 * @tc.name: HandleVersionUpdateChange_001
 * @tc.desc: Verify the HandleVersionUpdateChange function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, HandleVersionUpdateChange_001, TestSize.Level0)
{
    std::vector<DistributedKv::Entry> updateRecords;
    DistributedKv::Entry entry;
    entry.key = "strBase";
    entry.value = "strBase";
    updateRecords.push_back(entry);
    VersionInfoManager::GetInstance()->HandleVersionUpdateChange(updateRecords);
    EXPECT_EQ(DH_FWK_SUCCESS, VersionInfoManager::GetInstance()->Init());
}

/**
 * @tc.name: HandleVersionDeleteChange_001
 * @tc.desc: Verify the HandleVersionDeleteChange function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(VersionInfoManagerTest, HandleVersionDeleteChange_001, TestSize.Level0)
{
    std::vector<DistributedKv::Entry> deleteRecords;
    DistributedKv::Entry entry;
    entry.key = "strBase";
    entry.value = "strBase";
    deleteRecords.push_back(entry);
    VersionInfoManager::GetInstance()->HandleVersionDeleteChange(deleteRecords);
    EXPECT_EQ(DH_FWK_SUCCESS, VersionInfoManager::GetInstance()->Init());
}

/**
 * @tc.name: GetVersionInfoByDeviceId_001
 * @tc.desc: Verify the VersionInfoManager GetVersionInfoByDeviceId function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(VersionInfoManagerTest, GetVersionInfoByDeviceId_001, TestSize.Level0)
{
    std::string deviceId;
    VersionInfo versionInfo;
    VersionInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    int32_t ret = VersionInfoManager::GetInstance()->GetVersionInfoByDeviceId(deviceId, versionInfo);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

} // namespace DistributedHardware
} // namespace OHOS
