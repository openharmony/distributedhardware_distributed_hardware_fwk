/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "cJSON.h"

#include "constants.h"
#include "dh_utils_tool.h"
#include "dh_context.h"
#include "distributed_hardware_errno.h"
#include "meta_capability_info.h"
#include "meta_info_manager.h"
#include "task_board.h"
#include "impl_utils.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr uint16_t DEV_TYPE_TEST = 14;
    constexpr uint32_t MAX_DB_RECORD_LENGTH = 10005;
}

class MetaInfoMgrTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void MetaInfoMgrTest::SetUp() {}

void MetaInfoMgrTest::TearDown() {}

void MetaInfoMgrTest::SetUpTestCase() {}

void MetaInfoMgrTest::TearDownTestCase() {}

HWTEST_F(MetaInfoMgrTest, Init_001, TestSize.Level1)
{
    auto ret = MetaInfoManager::GetInstance()->Init();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    ret = MetaInfoManager::GetInstance()->UnInit();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(MetaInfoMgrTest, UnInit_001, TestSize.Level1)
{
    MetaInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    auto ret = MetaInfoManager::GetInstance()->UnInit();
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_UNINIT_DB_FAILED, ret);
}

HWTEST_F(MetaInfoMgrTest, AddMetaCapInfos_001, TestSize.Level1)
{
    std::string udidHash = "udidHash_test";
    std::vector<std::shared_ptr<MetaCapabilityInfo>> metaCapInfos;
    auto ret = MetaInfoManager::GetInstance()->AddMetaCapInfos(metaCapInfos);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_RES_DB_DATA_INVALID, ret);

    for (int32_t i = 1; i < MAX_DB_RECORD_LENGTH; i++) {
        std::shared_ptr<MetaCapabilityInfo> mateCapInfoTest = make_shared<MetaCapabilityInfo>(std::to_string(i),
            std::to_string(i), "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs", "subtype", udidHash,
            CompVersion{ .sinkVersion = "1.0" });
        metaCapInfos.push_back(mateCapInfoTest);
    }
    ret = MetaInfoManager::GetInstance()->AddMetaCapInfos(metaCapInfos);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_RES_DB_DATA_INVALID, ret);
}

HWTEST_F(MetaInfoMgrTest, AddMetaCapInfos_002, TestSize.Level1)
{
    std::string deviceId = "deviceId_test";
    std::string udidHash = "udidHash_test";
    std::string dhId = "dhId_test";
    CompVersion compVersion;
    compVersion.sinkVersion = "1.0";
    std::shared_ptr<MetaCapabilityInfo> metaCapInfo = std::make_shared<MetaCapabilityInfo>(
        dhId, deviceId, "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs_test", "subtype", udidHash, compVersion);
    std::vector<std::shared_ptr<MetaCapabilityInfo>> metaCapInfos;
    metaCapInfos.push_back(metaCapInfo);
    auto ret = MetaInfoManager::GetInstance()->AddMetaCapInfos(metaCapInfos);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);

    MetaInfoManager::GetInstance()->Init();
    std::shared_ptr<MetaCapabilityInfo> mateCapInfoTest = nullptr;
    metaCapInfos.push_back(mateCapInfoTest);
    ret = MetaInfoManager::GetInstance()->AddMetaCapInfos(metaCapInfos);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    MetaInfoManager::GetInstance()->UnInit();
}

HWTEST_F(MetaInfoMgrTest, SyncMetaInfoFromDB_001, TestSize.Level1)
{
    std::string udidHash = "";
    auto ret = MetaInfoManager::GetInstance()->SyncMetaInfoFromDB(udidHash);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(MetaInfoMgrTest, SyncMetaInfoFromDB_002, TestSize.Level1)
{
    std::string udidHash = "132456798";
    auto ret = MetaInfoManager::GetInstance()->SyncMetaInfoFromDB(udidHash);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

HWTEST_F(MetaInfoMgrTest, SyncMetaInfoFromDB_003, TestSize.Level1)
{
    std::string udidHash = "132456798";
    MetaInfoManager::GetInstance()->Init();
    auto ret = MetaInfoManager::GetInstance()->SyncMetaInfoFromDB(udidHash);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL, ret);
    MetaInfoManager::GetInstance()->UnInit();
}

HWTEST_F(MetaInfoMgrTest, SyncRemoteMetaInfos_001, TestSize.Level1)
{
    auto ret = MetaInfoManager::GetInstance()->SyncRemoteMetaInfos();
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);

    MetaInfoManager::GetInstance()->Init();
    ret = MetaInfoManager::GetInstance()->SyncRemoteMetaInfos();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    MetaInfoManager::GetInstance()->UnInit();
}

HWTEST_F(MetaInfoMgrTest, SyncRemoteMetaInfos_002, TestSize.Level1)
{
    std::string udid = "111111";
    std::string uuid = "222222";
    std::string networkId = "333333";
    DHContext::GetInstance().AddOnlineDevice(udid, uuid, networkId);
    MetaInfoManager::GetInstance()->Init();
    auto ret = MetaInfoManager::GetInstance()->SyncRemoteMetaInfos();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    MetaInfoManager::GetInstance()->UnInit();
    DHContext::GetInstance().RemoveOnlineDeviceIdEntryByNetworkId(networkId);
}

HWTEST_F(MetaInfoMgrTest, GetDataByKeyPrefix_001, TestSize.Level1)
{
    std::string keyPrefix = "";
    MetaCapInfoMap metaCapMap;
    auto ret = MetaInfoManager::GetInstance()->GetDataByKeyPrefix(keyPrefix, metaCapMap);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    keyPrefix = "keyPrefix_test";
    ret = MetaInfoManager::GetInstance()->GetDataByKeyPrefix(keyPrefix, metaCapMap);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

HWTEST_F(MetaInfoMgrTest, GetDataByKeyPrefix_002, TestSize.Level1)
{
    std::string keyPrefix = "keyPrefix_test";
    MetaCapInfoMap metaCapMap;
    MetaInfoManager::GetInstance()->Init();
    auto ret = MetaInfoManager::GetInstance()->GetDataByKeyPrefix(keyPrefix, metaCapMap);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL, ret);
    MetaInfoManager::GetInstance()->UnInit();
}

HWTEST_F(MetaInfoMgrTest, GetMetaCapInfo_001, TestSize.Level1)
{
    std::string udidHash = "";
    std::string dhId = "";
    std::shared_ptr<MetaCapabilityInfo> metaCapPtr;
    auto ret = MetaInfoManager::GetInstance()->GetMetaCapInfo(udidHash, dhId, metaCapPtr);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    udidHash = "123456789";
    ret = MetaInfoManager::GetInstance()->GetMetaCapInfo(udidHash, dhId, metaCapPtr);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    udidHash = "";
    dhId = "123456789";
    ret = MetaInfoManager::GetInstance()->GetMetaCapInfo(udidHash, dhId, metaCapPtr);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(MetaInfoMgrTest, GetMetaCapInfo_002, TestSize.Level1)
{
    std::string deviceId = "deviceId_test";
    std::string udidHash = "udidHash_test";
    std::string dhId = "dhId_test";
    CompVersion compVersion{ .sinkVersion = "1.0" };
    std::shared_ptr<MetaCapabilityInfo> metaCapPtr;
    std::shared_ptr<MetaCapabilityInfo> dhMetaCapInfo = std::make_shared<MetaCapabilityInfo>(
        dhId, deviceId, "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs_test", "subtype", udidHash, compVersion);
    std::string key = udidHash + "###" + dhId;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[key] = dhMetaCapInfo;
    auto ret = MetaInfoManager::GetInstance()->GetMetaCapInfo(deviceId, dhId, metaCapPtr);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_CAPABILITY_MAP_NOT_FOUND, ret);

    ret = MetaInfoManager::GetInstance()->GetMetaCapInfo(udidHash, dhId, metaCapPtr);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    MetaInfoManager::GetInstance()->globalMetaInfoMap_.clear();
}

HWTEST_F(MetaInfoMgrTest, GetMetaCapInfosByUdidHash_001, TestSize.Level1)
{
    std::string udidHash = "";
    std::vector<std::shared_ptr<MetaCapabilityInfo>> metaCapInfos;
    MetaInfoManager::GetInstance()->GetMetaCapInfosByUdidHash(udidHash, metaCapInfos);

    udidHash = "udidHash_test";
    std::string deviceId = "deviceId_test";
    std::string dhId = "dhId_test";
    std::shared_ptr<MetaCapabilityInfo> metaCapInfo = std::make_shared<MetaCapabilityInfo>(
        dhId, deviceId, "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs_test", "subtype", udidHash,
        CompVersion{ .sinkVersion = "1.0" });
    std::string key = udidHash + "###" + dhId;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[key] = metaCapInfo;
    MetaInfoManager::GetInstance()->GetMetaCapInfosByUdidHash(deviceId, metaCapInfos);
    EXPECT_TRUE(metaCapInfos.empty());

    MetaInfoManager::GetInstance()->GetMetaCapInfosByUdidHash(udidHash, metaCapInfos);
    EXPECT_FALSE(metaCapInfos.empty());
    MetaInfoManager::GetInstance()->globalMetaInfoMap_.clear();
}

HWTEST_F(MetaInfoMgrTest, GetMetaCapByValue_001, TestSize.Level1)
{
    std::string value = "";
    std::shared_ptr<MetaCapabilityInfo> metaCapPtr = nullptr;
    auto ret = MetaInfoManager::GetInstance()->GetMetaCapByValue(value, metaCapPtr);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    value = "invalid JSON string";
    ret = MetaInfoManager::GetInstance()->GetMetaCapByValue(value, metaCapPtr);
    EXPECT_EQ(ERR_DH_FWK_JSON_PARSE_FAILED, ret);

    std::string deviceId = "deviceId_test";
    std::string udidHash = "udidHash_test";
    std::string dhId = "dhId_test";
    metaCapPtr = std::make_shared<MetaCapabilityInfo>(
        dhId, deviceId, "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs_test", "subtype", udidHash,
        CompVersion{ .sinkVersion = "1.0" });
    ret = MetaInfoManager::GetInstance()->GetMetaCapByValue(value, metaCapPtr);
    EXPECT_EQ(ERR_DH_FWK_JSON_PARSE_FAILED, ret);
}

HWTEST_F(MetaInfoMgrTest, GetMetaDataByDHType_001, TestSize.Level1)
{
    std::string deviceId = "deviceId_test";
    std::string udidHash = "udidHash_test";
    std::string dhId = "dhId_test";
    MetaCapInfoMap metaInfoMap;
    std::shared_ptr<MetaCapabilityInfo> dhMetaCapInfo = std::make_shared<MetaCapabilityInfo>(
        dhId, deviceId, "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs_test", "subtype", udidHash,
        CompVersion{ .sinkVersion = "1.0" });
    std::string key = udidHash + "###" + dhId;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[key] = dhMetaCapInfo;
    auto ret = MetaInfoManager::GetInstance()->GetMetaDataByDHType(DHType::AUDIO, metaInfoMap);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    ret = MetaInfoManager::GetInstance()->GetMetaDataByDHType(DHType::CAMERA, metaInfoMap);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    MetaInfoManager::GetInstance()->globalMetaInfoMap_.clear();
}

HWTEST_F(MetaInfoMgrTest, SyncDataByNetworkId_001, TestSize.Level1)
{
    std::string networkId = "";
    auto ret = MetaInfoManager::GetInstance()->SyncDataByNetworkId(networkId);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    networkId = "networkId_test";
    ret = MetaInfoManager::GetInstance()->SyncDataByNetworkId(networkId);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

HWTEST_F(MetaInfoMgrTest, SyncDataByNetworkId_002, TestSize.Level1)
{
    std::string networkId = "networkId_test";
    MetaInfoManager::GetInstance()->Init();
    auto ret = MetaInfoManager::GetInstance()->SyncDataByNetworkId(networkId);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    MetaInfoManager::GetInstance()->UnInit();
}

HWTEST_F(MetaInfoMgrTest, RemoveMetaInfoInMemByUdid_001, TestSize.Level1)
{
    std::string peerudid = "123456789";
    std::string peerudIdHash = Sha256(peerudid);
    std::string deviceId = "deviceId_test";
    std::string udidHash = "udidHash_test";
    std::string dhId = "dhId_test";
    std::shared_ptr<MetaCapabilityInfo> metaCapPtr = std::make_shared<MetaCapabilityInfo>(
        dhId, deviceId, "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs_test", "subtype", udidHash,
        CompVersion{ .sinkVersion = "1.0" });
    std::string key = peerudIdHash + "###" + dhId;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[key] = metaCapPtr;
    auto ret = MetaInfoManager::GetInstance()->RemoveMetaInfoInMemByUdid(deviceId);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    ret = MetaInfoManager::GetInstance()->RemoveMetaInfoInMemByUdid(peerudid);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(MetaInfoMgrTest, ClearRemoteDeviceMetaInfoData_001, TestSize.Level1)
{
    MetaInfoManager::GetInstance()->Init();
    std::string peerudid = "";
    std::string peeruuid = "";
    auto ret = MetaInfoManager::GetInstance()->ClearRemoteDeviceMetaInfoData(peerudid, peeruuid);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL, ret);

    peerudid = "peerudid_test";
    peeruuid = "peeruuid_test";
    ret = MetaInfoManager::GetInstance()->ClearRemoteDeviceMetaInfoData(peerudid, peeruuid);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    MetaInfoManager::GetInstance()->UnInit();
    ret = MetaInfoManager::GetInstance()->ClearRemoteDeviceMetaInfoData(peerudid, peeruuid);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

HWTEST_F(MetaInfoMgrTest, OnChange_001, TestSize.Level1)
{
    DistributedKv::Entry insert, update, del;
    std::vector<DistributedKv::Entry> inserts, updates, deleteds;
    inserts.push_back(insert);
    updates.push_back(update);
    deleteds.push_back(del);
    DistributedKv::ChangeNotification changeIn(std::move(inserts), std::move(updates), std::move(deleteds), "", true);
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->OnChange(changeIn));
}

HWTEST_F(MetaInfoMgrTest, OnChange_002, TestSize.Level1)
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
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->OnChange(changeIn));
}

HWTEST_F(MetaInfoMgrTest, OnChange_003, TestSize.Level1)
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
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->OnChange(changeIn));
}

HWTEST_F(MetaInfoMgrTest, OnChange_004, TestSize.Level1)
{
    DistributedKv::DataOrigin origin;
    origin.id = {};
    origin.store = "";
    MetaInfoManager::Keys keys;
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->OnChange(origin, std::move(keys)));
}

HWTEST_F(MetaInfoMgrTest, OnChange_005, TestSize.Level1)
{
    DistributedKv::DataOrigin origin;
    origin.id = {};
    origin.store = "";
    MetaInfoManager::Keys keys;
    keys[MetaInfoManager::OP_INSERT] = {"insert"};
    keys[MetaInfoManager::OP_UPDATE] = {"update"};
    keys[MetaInfoManager::OP_DELETE] = {"delete"};
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->OnChange(origin, std::move(keys)));
}

HWTEST_F(MetaInfoMgrTest, HandleMetaCapabilityAddChange_001, TestSize.Level1)
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
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->HandleMetaCapabilityAddChange(insertRecords));
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
}

HWTEST_F(MetaInfoMgrTest, HandleMetaCapabilityAddChange_002, TestSize.Level1)
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
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->HandleMetaCapabilityAddChange(insertRecords));
    DHContext::GetInstance().devIdEntrySet_.clear();
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
}

HWTEST_F(MetaInfoMgrTest, HandleMetaCapabilityAddChange_003, TestSize.Level1)
{
    std::string uuid = "123456789";
    std::string deviceId = Sha256(uuid);
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
    DHContext::GetInstance().AddOnlineDevice("111111", uuid, "222222");
    std::vector<DistributedKv::Entry> insertRecords;
    DistributedKv::Entry entry;
    entry.key = "insert";
    entry.value = jsonStr.c_str();
    insertRecords.push_back(entry);
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->HandleMetaCapabilityAddChange(insertRecords));
    DHContext::GetInstance().RemoveOnlineDeviceIdEntryByNetworkId("222222");
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
}

HWTEST_F(MetaInfoMgrTest, HandleMetaCapabilityUpdateChange_001, TestSize.Level1)
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
    std::vector<DistributedKv::Entry> updateRecords;
    DistributedKv::Entry update;
    update.key = "update";
    update.value = jsonStr.c_str();
    updateRecords.push_back(update);
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->HandleMetaCapabilityUpdateChange(updateRecords));
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
}

HWTEST_F(MetaInfoMgrTest, HandleMetaCapabilityUpdateChange_002, TestSize.Level1)
{
    std::string uuid = "123456789";
    std::string deviceId = Sha256(uuid);
    DeviceIdEntry idEntry = {
        .networkId = "",
        .uuid = uuid,
        .deviceId = deviceId
    };
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
    std::vector<DistributedKv::Entry> updateRecords;
    DistributedKv::Entry update;
    update.key = "update";
    update.value = jsonStr.c_str();
    updateRecords.push_back(update);
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->HandleMetaCapabilityUpdateChange(updateRecords));
    DHContext::GetInstance().devIdEntrySet_.clear();
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
}

HWTEST_F(MetaInfoMgrTest, HandleMetaCapabilityUpdateChange_003, TestSize.Level1)
{
    std::string uuid = "123456789";
    std::string deviceId = Sha256(uuid);
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
    std::vector<DistributedKv::Entry> updateRecords;
    DHContext::GetInstance().AddOnlineDevice("111111", uuid, "222222");
    DistributedKv::Entry update;
    update.key = "update";
    update.value = jsonStr.c_str();
    updateRecords.push_back(update);
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->HandleMetaCapabilityUpdateChange(updateRecords));

    std::string enabledDeviceKey = deviceId + RESOURCE_SEPARATOR + "111111";
    TaskParam taskParam;
    TaskBoard::GetInstance().SaveEnabledDevice(enabledDeviceKey, taskParam);
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->HandleMetaCapabilityUpdateChange(updateRecords));

    DHContext::GetInstance().RemoveOnlineDeviceIdEntryByNetworkId("222222");
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
}

HWTEST_F(MetaInfoMgrTest, HandleMetaCapabilityDeleteChange_001, TestSize.Level1)
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
    std::vector<DistributedKv::Entry> deleteRecords;
    DistributedKv::Entry del;
    del.key = "update";
    del.value = jsonStr.c_str();
    deleteRecords.push_back(del);
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->HandleMetaCapabilityDeleteChange(deleteRecords));
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
}

HWTEST_F(MetaInfoMgrTest, GetEntriesByKeys_001, TestSize.Level1)
{
    std::vector<std::string> keys;
    auto ret = MetaInfoManager::GetInstance()->GetEntriesByKeys(keys);
    EXPECT_EQ(0, ret.size());

    keys.push_back("key_test");
    ret = MetaInfoManager::GetInstance()->GetEntriesByKeys(keys);
    EXPECT_EQ(0, ret.size());

    MetaInfoManager::GetInstance()->Init();
    ret = MetaInfoManager::GetInstance()->GetEntriesByKeys(keys);
    EXPECT_EQ(0, ret.size());
    MetaInfoManager::GetInstance()->UnInit();
}

HWTEST_F(MetaInfoMgrTest, GetDhSubtypeByUdidHash_001, TestSize.Level1)
{
    std::string udidHash = "";
    std::string dhId = "";
    auto ret = MetaInfoManager::GetInstance()->GetDhSubtypeByUdidHash(udidHash, dhId);
    EXPECT_EQ("", ret);

    udidHash = "udidHash_1";
    ret = MetaInfoManager::GetInstance()->GetDhSubtypeByUdidHash(udidHash, dhId);
    EXPECT_EQ("", ret);

    udidHash = "";
    dhId = "dhId_1";
    ret = MetaInfoManager::GetInstance()->GetDhSubtypeByUdidHash(udidHash, dhId);
    EXPECT_EQ("", ret);
}

HWTEST_F(MetaInfoMgrTest, GetDhSubtypeByUdidHash_002, TestSize.Level1)
{
    std::string dhid = "audio_123";
    std::string deviceId = "deviceId_123";
    std::string udidHash = "udidHash_123";

    std::shared_ptr<MetaCapabilityInfo> metaCapPtr = std::make_shared<MetaCapabilityInfo>(
        dhid, deviceId, "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs_test", "subtype", udidHash,
        CompVersion{ .sinkVersion = "1.0" });
    std::string key = udidHash + "###" + dhid;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[key] = metaCapPtr;
    auto ret = MetaInfoManager::GetInstance()->GetDhSubtypeByUdidHash("udidHash_456", dhid);
    EXPECT_EQ("", ret);

    ret = MetaInfoManager::GetInstance()->GetDhSubtypeByUdidHash(udidHash, dhid);
    EXPECT_EQ("subtype", ret);

    std::shared_ptr<MetaCapabilityInfo> metaCapPtr1 = nullptr;
    std::string udidHash1 = "udidHash_456";
    std::string key1 = udidHash1 + "###" + dhid;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[key1] = metaCapPtr1;
    ret = MetaInfoManager::GetInstance()->GetDhSubtypeByUdidHash(udidHash1, dhid);
    EXPECT_EQ("", ret);
}

HWTEST_F(MetaInfoMgrTest, ActiveCloudSyncData_001, TestSize.Level1)
{
    MetaInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    auto ret = MetaInfoManager::GetInstance()->ActiveCloudSyncData();
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

HWTEST_F(MetaInfoMgrTest, ActiveCloudSyncData_002, TestSize.Level1)
{
    MetaInfoManager::GetInstance()->Init();
    auto ret = MetaInfoManager::GetInstance()->ActiveCloudSyncData();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}
}
}