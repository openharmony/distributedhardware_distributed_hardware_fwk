/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "distributed_hardware_errno.h"
#include "meta_capability_info.h"
#include "meta_info_manager.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr uint16_t DEV_TYPE_TEST = 14;
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

HWTEST_F(MetaInfoMgrTest, Init_001, TestSize.Level0)
{
    auto ret = MetaInfoManager::GetInstance()->Init();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    ret = MetaInfoManager::GetInstance()->UnInit();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(MetaInfoMgrTest, UnInit_001, TestSize.Level0)
{
    MetaInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    auto ret = MetaInfoManager::GetInstance()->UnInit();
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_UNINIT_DB_FAILED, ret);
}

HWTEST_F(MetaInfoMgrTest, AddMetaCapInfos_001, TestSize.Level0)
{
    std::vector<std::shared_ptr<MetaCapabilityInfo>> metaCapInfos;
    auto ret = MetaInfoManager::GetInstance()->AddMetaCapInfos(metaCapInfos);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_RES_DB_DATA_INVALID, ret);
}

HWTEST_F(MetaInfoMgrTest, AddMetaCapInfos_002, TestSize.Level0)
{
    std::string deviceId = "deviceId_test";
    std::string udidHash = "udidHash_test";
    std::string dhId = "dhId_test";
    std::shared_ptr<MetaCapabilityInfo> metaCapInfo = std::make_shared<MetaCapabilityInfo>(
        dhId, deviceId, "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs_test", "subtype", udidHash, "1.0");
    std::vector<std::shared_ptr<MetaCapabilityInfo>> metaCapInfos;
    metaCapInfos.push_back(metaCapInfo);
    auto ret = MetaInfoManager::GetInstance()->AddMetaCapInfos(metaCapInfos);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);

    MetaInfoManager::GetInstance()->Init();
    ret = MetaInfoManager::GetInstance()->AddMetaCapInfos(metaCapInfos);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    MetaInfoManager::GetInstance()->UnInit();
}

HWTEST_F(MetaInfoMgrTest, SyncMetaInfoFromDB_001, TestSize.Level0)
{
    std::string udidHash = "";
    auto ret = MetaInfoManager::GetInstance()->SyncMetaInfoFromDB(udidHash);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(MetaInfoMgrTest, SyncMetaInfoFromDB_002, TestSize.Level0)
{
    std::string udidHash = "132456798";
    auto ret = MetaInfoManager::GetInstance()->SyncMetaInfoFromDB(udidHash);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

HWTEST_F(MetaInfoMgrTest, SyncMetaInfoFromDB_003, TestSize.Level0)
{
    std::string udidHash = "132456798";
    MetaInfoManager::GetInstance()->Init();
    auto ret = MetaInfoManager::GetInstance()->SyncMetaInfoFromDB(udidHash);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL, ret);
    MetaInfoManager::GetInstance()->UnInit();
}

HWTEST_F(MetaInfoMgrTest, SyncRemoteMetaInfos_001, TestSize.Level0)
{
    auto ret = MetaInfoManager::GetInstance()->SyncRemoteMetaInfos();
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);

    MetaInfoManager::GetInstance()->Init();
    ret = MetaInfoManager::GetInstance()->SyncRemoteMetaInfos();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    MetaInfoManager::GetInstance()->UnInit();
}

HWTEST_F(MetaInfoMgrTest, GetDataByKeyPrefix_001, TestSize.Level0)
{
    std::string keyPrefix = "";
    MetaCapInfoMap metaCapMap;
    auto ret = MetaInfoManager::GetInstance()->GetDataByKeyPrefix(keyPrefix, metaCapMap);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    keyPrefix = "keyPrefix_test";
    ret = MetaInfoManager::GetInstance()->GetDataByKeyPrefix(keyPrefix, metaCapMap);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

HWTEST_F(MetaInfoMgrTest, GetDataByKeyPrefix_002, TestSize.Level0)
{
    std::string keyPrefix = "keyPrefix_test";
    MetaCapInfoMap metaCapMap;
    MetaInfoManager::GetInstance()->Init();
    auto ret = MetaInfoManager::GetInstance()->GetDataByKeyPrefix(keyPrefix, metaCapMap);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL, ret);
    MetaInfoManager::GetInstance()->UnInit();
}

HWTEST_F(MetaInfoMgrTest, RemoveMetaInfoByKey_001, TestSize.Level0)
{
    std::string key = "";
    auto ret = MetaInfoManager::GetInstance()->RemoveMetaInfoByKey(key);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    key = "key_test";
    ret = MetaInfoManager::GetInstance()->RemoveMetaInfoByKey(key);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

HWTEST_F(MetaInfoMgrTest, RemoveMetaInfoByKey_002, TestSize.Level0)
{
    std::string key = "key_test";
    MetaInfoManager::GetInstance()->Init();
    auto ret = MetaInfoManager::GetInstance()->RemoveMetaInfoByKey(key);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    MetaInfoManager::GetInstance()->UnInit();
}

HWTEST_F(MetaInfoMgrTest, GetMetaCapInfo_001, TestSize.Level0)
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

HWTEST_F(MetaInfoMgrTest, GetMetaCapInfo_002, TestSize.Level0)
{
    std::string deviceId = "deviceId_test";
    std::string udidHash = "udidHash_test";
    std::string dhId = "dhId_test";
    std::shared_ptr<MetaCapabilityInfo> metaCapPtr;
    std::shared_ptr<MetaCapabilityInfo> dhMetaCapInfo = std::make_shared<MetaCapabilityInfo>(
        dhId, deviceId, "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs_test", "subtype", udidHash, "1.0");
    std::string key = udidHash + "###" + dhId;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[key] = dhMetaCapInfo;
    auto ret = MetaInfoManager::GetInstance()->GetMetaCapInfo(deviceId, dhId, metaCapPtr);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_CAPABILITY_MAP_NOT_FOUND, ret);

    ret = MetaInfoManager::GetInstance()->GetMetaCapInfo(udidHash, dhId, metaCapPtr);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    MetaInfoManager::GetInstance()->globalMetaInfoMap_.clear();
}

HWTEST_F(MetaInfoMgrTest, GetMetaCapInfosByUdidHash_001, TestSize.Level0)
{
    std::string udidHash = "";
    std::vector<std::shared_ptr<MetaCapabilityInfo>> metaCapInfos;
    MetaInfoManager::GetInstance()->GetMetaCapInfosByUdidHash(udidHash, metaCapInfos);

    udidHash = "udidHash_test";
    std::string deviceId = "deviceId_test";
    std::string dhId = "dhId_test";
    std::shared_ptr<MetaCapabilityInfo> metaCapInfo = std::make_shared<MetaCapabilityInfo>(
        dhId, deviceId, "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs_test", "subtype", udidHash, "1.0");
    std::string key = udidHash + "###" + dhId;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[key] = metaCapInfo;
    MetaInfoManager::GetInstance()->GetMetaCapInfosByUdidHash(deviceId, metaCapInfos);
    EXPECT_TRUE(metaCapInfos.empty());

    MetaInfoManager::GetInstance()->GetMetaCapInfosByUdidHash(udidHash, metaCapInfos);
    EXPECT_FALSE(metaCapInfos.empty());
    MetaInfoManager::GetInstance()->globalMetaInfoMap_.clear();
}

HWTEST_F(MetaInfoMgrTest, GetMetaCapByValue_001, TestSize.Level0)
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
        dhId, deviceId, "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs_test", "subtype", udidHash, "1.0");
    ret = MetaInfoManager::GetInstance()->GetMetaCapByValue(value, metaCapPtr);
    EXPECT_EQ(ERR_DH_FWK_JSON_PARSE_FAILED, ret);
}

HWTEST_F(MetaInfoMgrTest, GetMetaDataByDHType_001, TestSize.Level0)
{
    std::string deviceId = "deviceId_test";
    std::string udidHash = "udidHash_test";
    std::string dhId = "dhId_test";
    MetaCapInfoMap metaInfoMap;
    std::shared_ptr<MetaCapabilityInfo> dhMetaCapInfo = std::make_shared<MetaCapabilityInfo>(
        dhId, deviceId, "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs_test", "subtype", udidHash, "1.0");
    std::string key = udidHash + "###" + dhId;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[key] = dhMetaCapInfo;
    auto ret = MetaInfoManager::GetInstance()->GetMetaDataByDHType(DHType::AUDIO, metaInfoMap);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    ret = MetaInfoManager::GetInstance()->GetMetaDataByDHType(DHType::CAMERA, metaInfoMap);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    MetaInfoManager::GetInstance()->globalMetaInfoMap_.clear();
}

HWTEST_F(MetaInfoMgrTest, SyncDataByNetworkId_001, TestSize.Level0)
{
    std::string networkId = "";
    auto ret = MetaInfoManager::GetInstance()->SyncDataByNetworkId(networkId);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    networkId = "networkId_test";
    ret = MetaInfoManager::GetInstance()->SyncDataByNetworkId(networkId);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

HWTEST_F(MetaInfoMgrTest, SyncDataByNetworkId_002, TestSize.Level0)
{
    std::string networkId = "networkId_test";
    MetaInfoManager::GetInstance()->Init();
    auto ret = MetaInfoManager::GetInstance()->SyncDataByNetworkId(networkId);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    MetaInfoManager::GetInstance()->UnInit();
}

HWTEST_F(MetaInfoMgrTest, RemoveMetaInfoInMemByUdid_001, TestSize.Level0)
{
    std::string peerudid = "123456789";
    std::string peerudIdHash = Sha256(peerudid);
    std::string deviceId = "deviceId_test";
    std::string udidHash = "udidHash_test";
    std::string dhId = "dhId_test";
    std::shared_ptr<MetaCapabilityInfo> metaCapPtr = std::make_shared<MetaCapabilityInfo>(
        dhId, deviceId, "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs_test", "subtype", udidHash, "1.0");
    std::string key = peerudIdHash + "###" + dhId;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[key] = metaCapPtr;
    auto ret = MetaInfoManager::GetInstance()->RemoveMetaInfoInMemByUdid(deviceId);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    ret = MetaInfoManager::GetInstance()->RemoveMetaInfoInMemByUdid(peerudid);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(MetaInfoMgrTest, ClearRemoteDeviceMetaInfoData_001, TestSize.Level0)
{
    MetaInfoManager::GetInstance()->Init();
    std::string peerudid = "peerudid_test";
    std::string peeruuid = "peeruuid_test";
    auto ret = MetaInfoManager::GetInstance()->ClearRemoteDeviceMetaInfoData(peerudid, peeruuid);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    MetaInfoManager::GetInstance()->UnInit();
    ret = MetaInfoManager::GetInstance()->ClearRemoteDeviceMetaInfoData(peerudid, peeruuid);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

HWTEST_F(MetaInfoMgrTest, GetEntriesByKeys_001, TestSize.Level0)
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
}
}