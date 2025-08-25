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
#include "capability_info.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "local_capability_info_manager.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr uint16_t DEV_TYPE_TEST = 14;
    constexpr uint32_t MAX_DB_RECORD_LENGTH = 10005;
}
class LocalCapInfoMgrTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void LocalCapInfoMgrTest::SetUp() {}

void LocalCapInfoMgrTest::TearDown() {}

void LocalCapInfoMgrTest::SetUpTestCase() {}

void LocalCapInfoMgrTest::TearDownTestCase() {}

HWTEST_F(LocalCapInfoMgrTest, Init_001, TestSize.Level1)
{
    auto ret = LocalCapabilityInfoManager::GetInstance()->Init();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    ret = LocalCapabilityInfoManager::GetInstance()->UnInit();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(LocalCapInfoMgrTest, UnInit_001, TestSize.Level1)
{
    LocalCapabilityInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    auto ret = LocalCapabilityInfoManager::GetInstance()->UnInit();
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_UNINIT_DB_FAILED, ret);
}

HWTEST_F(LocalCapInfoMgrTest, SyncDeviceInfoFromDB_001, TestSize.Level1)
{
    std::string deviceId = "deviceId_test";
    LocalCapabilityInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    auto ret = LocalCapabilityInfoManager::GetInstance()->SyncDeviceInfoFromDB(deviceId);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);

    LocalCapabilityInfoManager::GetInstance()->Init();
    ret = LocalCapabilityInfoManager::GetInstance()->SyncDeviceInfoFromDB(deviceId);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL, ret);

    ret = LocalCapabilityInfoManager::GetInstance()->SyncDeviceInfoFromDB("");
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(LocalCapInfoMgrTest, SyncDeviceInfoFromDB_002, TestSize.Level1)
{
    std::string deviceId(266, 'A');
    LocalCapabilityInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    auto ret = LocalCapabilityInfoManager::GetInstance()->SyncDeviceInfoFromDB(deviceId);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(LocalCapInfoMgrTest, AddCapability_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<CapabilityInfo>> resInfos;
    auto ret = LocalCapabilityInfoManager::GetInstance()->AddCapability(resInfos);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_RES_DB_DATA_INVALID, ret);

    for (int32_t i = 1; i < MAX_DB_RECORD_LENGTH; i++) {
        std::shared_ptr<CapabilityInfo> capInfoTest = make_shared<CapabilityInfo>(std::to_string(i), std::to_string(i),
            "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs", "subtype");
        resInfos.push_back(capInfoTest);
    }
    ret = LocalCapabilityInfoManager::GetInstance()->AddCapability(resInfos);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_RES_DB_DATA_INVALID, ret);
}

HWTEST_F(LocalCapInfoMgrTest, AddCapability_002, TestSize.Level1)
{
    std::shared_ptr<CapabilityInfo> capInfo = std::make_shared<CapabilityInfo>("", "", "", 0, DHType::UNKNOWN, "", "");
    std::vector<std::shared_ptr<CapabilityInfo>> resInfos;
    resInfos.push_back(capInfo);
    LocalCapabilityInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    auto ret = LocalCapabilityInfoManager::GetInstance()->AddCapability(resInfos);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

HWTEST_F(LocalCapInfoMgrTest, AddCapability_003, TestSize.Level1)
{
    std::shared_ptr<CapabilityInfo> capInfo = std::make_shared<CapabilityInfo>("", "", "", 0, DHType::UNKNOWN, "", "");
    std::shared_ptr<CapabilityInfo> capInfo1 = nullptr;
    std::vector<std::shared_ptr<CapabilityInfo>> resInfos;
    resInfos.push_back(capInfo);
    resInfos.push_back(capInfo1);
    LocalCapabilityInfoManager::GetInstance()->Init();
    auto ret = LocalCapabilityInfoManager::GetInstance()->AddCapability(resInfos);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(LocalCapInfoMgrTest, RemoveCapabilityInfoByKey_001, TestSize.Level1)
{
    std::string key = "";
    auto ret = LocalCapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(key);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(LocalCapInfoMgrTest, RemoveCapabilityInfoByKey_002, TestSize.Level1)
{
    std::string key = "deviceId_test";
    LocalCapabilityInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    auto ret = LocalCapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(key);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);

    LocalCapabilityInfoManager::GetInstance()->Init();
    ret = LocalCapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(key);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(LocalCapInfoMgrTest, GetCapabilitiesByDeviceId_001, TestSize.Level1)
{
    std::string deviceId = "";
    std::vector<std::shared_ptr<CapabilityInfo>> resInfos;
    ASSERT_NO_FATAL_FAILURE(LocalCapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(deviceId, resInfos));
}

HWTEST_F(LocalCapInfoMgrTest, GetCapabilitiesByDeviceId_002, TestSize.Level1)
{
    std::string deviceId = "test_device_id";
    auto localCapInfoMgr = LocalCapabilityInfoManager::GetInstance();
    std::shared_ptr<CapabilityInfo> capInfo1 = std::make_shared<CapabilityInfo>(
        "dhId_test1", deviceId, "devName_test1", DEV_TYPE_TEST, DHType::AUDIO, "attrs", "subtype");
    std::shared_ptr<CapabilityInfo> capInfo2 = std::make_shared<CapabilityInfo>(
        "dhId_test2", "other_device_id", "devName_test2", DEV_TYPE_TEST, DHType::AUDIO, "attrs", "subtype");

    std::string key1 = GetCapabilityKey(deviceId, "dhId_test1");
    std::string key2 = GetCapabilityKey("other_device_id", "dhId_test2");

    localCapInfoMgr->globalCapInfoMap_[key1] = capInfo1;
    localCapInfoMgr->globalCapInfoMap_[key2] = capInfo2;

    std::vector<std::shared_ptr<CapabilityInfo>> resInfos;
    localCapInfoMgr->GetCapabilitiesByDeviceId(deviceId, resInfos);
    ASSERT_EQ(resInfos.size(), 1);
}

HWTEST_F(LocalCapInfoMgrTest, GetCapability_001, TestSize.Level1)
{
    std::string deviceId = "";
    std::string dhId = "";
    std::shared_ptr<CapabilityInfo> capPtr;
    auto ret = LocalCapabilityInfoManager::GetInstance()->GetCapability(deviceId, dhId, capPtr);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    deviceId = "deviceId_test";
    ret = LocalCapabilityInfoManager::GetInstance()->GetCapability(deviceId, dhId, capPtr);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    dhId = "dhId_test";
    deviceId = "";
    ret = LocalCapabilityInfoManager::GetInstance()->GetCapability(deviceId, dhId, capPtr);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(LocalCapInfoMgrTest, GetCapability_002, TestSize.Level1)
{
    std::string deviceId = "deviceId_test";
    std::string dhId = "dhId_test";
    std::shared_ptr<CapabilityInfo> capPtr;
    auto ret = LocalCapabilityInfoManager::GetInstance()->GetCapability(deviceId, dhId, capPtr);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_CAPABILITY_MAP_NOT_FOUND, ret);

    std::shared_ptr<CapabilityInfo> capbilityInfo = std::make_shared<CapabilityInfo>(
        dhId, deviceId, "devName_test", DEV_TYPE_TEST, DHType::AUDIO, "attrs", "subtype");
    std::string key = deviceId + "###" + dhId;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capbilityInfo;
    ret = LocalCapabilityInfoManager::GetInstance()->GetCapability(deviceId, dhId, capPtr);
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_.clear();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(LocalCapInfoMgrTest, GetDataByKey_001, TestSize.Level1)
{
    std::string key = "";
    std::shared_ptr<CapabilityInfo> capPtr;
    auto ret = LocalCapabilityInfoManager::GetInstance()->GetDataByKey(key, capPtr);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(LocalCapInfoMgrTest, GetDataByKey_002, TestSize.Level1)
{
    std::string key = "key_test";
    std::shared_ptr<CapabilityInfo> capPtr;
    LocalCapabilityInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    auto ret = LocalCapabilityInfoManager::GetInstance()->GetDataByKey(key, capPtr);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);

    LocalCapabilityInfoManager::GetInstance()->Init();
    ret = LocalCapabilityInfoManager::GetInstance()->GetDataByKey(key, capPtr);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL, ret);
}

HWTEST_F(LocalCapInfoMgrTest, GetDataByDHType_001, TestSize.Level1)
{
    CapabilityInfoMap capabilityMap;
    std::string deviceId = "deviceId_test";
    std::shared_ptr<CapabilityInfo> capbilityInfo = std::make_shared<CapabilityInfo>(
        "dhId_test", deviceId, "devName_test", 14, DHType::AUDIO, "attrs", "subtype");
    std::string key = deviceId + "###" + "dhId_test";
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capbilityInfo;
    auto ret = LocalCapabilityInfoManager::GetInstance()->GetDataByDHType(DHType::CAMERA, capabilityMap);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    ret = LocalCapabilityInfoManager::GetInstance()->GetDataByDHType(DHType::AUDIO, capabilityMap);
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_.clear();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(LocalCapInfoMgrTest, GetDataByKeyPrefix_001, TestSize.Level1)
{
    std::string keyPrefix = "";
    CapabilityInfoMap capabilityMap;
    auto ret = LocalCapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(keyPrefix, capabilityMap);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(LocalCapInfoMgrTest, GetDataByKeyPrefix_002, TestSize.Level1)
{
    std::string keyPrefix = "keyPrefix_test";
    CapabilityInfoMap capabilityMap;
    LocalCapabilityInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    auto ret = LocalCapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(keyPrefix, capabilityMap);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);

    LocalCapabilityInfoManager::GetInstance()->Init();
    ret = LocalCapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(keyPrefix, capabilityMap);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL, ret);
}

HWTEST_F(LocalCapInfoMgrTest, ClearRemoteDeviceLocalInfoData_001, TestSize.Level1)
{
    std::string peeruuid = "";
    LocalCapabilityInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    auto ret = LocalCapabilityInfoManager::GetInstance()->ClearRemoteDeviceLocalInfoData(peeruuid);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);

    LocalCapabilityInfoManager::GetInstance()->Init();
    ret = LocalCapabilityInfoManager::GetInstance()->ClearRemoteDeviceLocalInfoData(peeruuid);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(LocalCapInfoMgrTest, RemoveLocalInfoInMemByUuid_001, TestSize.Level1)
{
    std::string peeruuid = "123456789";
    std::string dhid = "audio_132";
    std::string deviceId = Sha256(peeruuid);

    std::shared_ptr<CapabilityInfo> capInfo = std::make_shared<CapabilityInfo>(
        dhid, deviceId, "devName_test", DEV_TYPE_TEST, DHType::AUDIO, "attrs", "subtype");
    std::string key = deviceId + "###" + dhid;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capInfo;
    auto ret = LocalCapabilityInfoManager::GetInstance()->RemoveLocalInfoInMemByUuid("111222333");
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    ret = LocalCapabilityInfoManager::GetInstance()->RemoveLocalInfoInMemByUuid(peeruuid);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(LocalCapInfoMgrTest, GetDhSubtype_001, TestSize.Level1)
{
    std::string deviceId = "";
    std::string dhId = "";
    auto ret = LocalCapabilityInfoManager::GetInstance()->GetDhSubtype(deviceId, dhId);
    EXPECT_EQ("", ret);

    deviceId = "deviceId_1";
    ret = LocalCapabilityInfoManager::GetInstance()->GetDhSubtype(deviceId, dhId);
    EXPECT_EQ("", ret);

    deviceId = "";
    dhId = "dhId_1";
    ret = LocalCapabilityInfoManager::GetInstance()->GetDhSubtype(deviceId, dhId);
    EXPECT_EQ("", ret);
}

HWTEST_F(LocalCapInfoMgrTest, GetDhSubtype_002, TestSize.Level1)
{
    std::string peeruuid = "123456789";
    std::string dhid = "audio_132";
    std::string deviceId = Sha256(peeruuid);

    std::shared_ptr<CapabilityInfo> capInfo = std::make_shared<CapabilityInfo>(
        dhid, deviceId, "devName_test", DEV_TYPE_TEST, DHType::AUDIO, "attrs", "subtype");
    std::string key = deviceId + "###" + dhid;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capInfo;
    auto ret = LocalCapabilityInfoManager::GetInstance()->GetDhSubtype("deviceId_1", dhid);
    EXPECT_EQ("", ret);

    ret = LocalCapabilityInfoManager::GetInstance()->GetDhSubtype(deviceId, dhid);
    EXPECT_EQ("subtype", ret);

    std::shared_ptr<CapabilityInfo> capInfo1 = nullptr;
    std::string deviceId1 = "deviceId_2";
    std::string key1 = deviceId1 + "###" + dhid;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key1] = capInfo1;
    ret = LocalCapabilityInfoManager::GetInstance()->GetDhSubtype(deviceId1, dhid);
    EXPECT_EQ("", ret);
}
}
}