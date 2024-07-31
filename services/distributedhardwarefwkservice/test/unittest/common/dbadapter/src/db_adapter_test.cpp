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

#include "db_adapter_test.h"

#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#include "capability_info.h"
#include "capability_info_manager.h"
#include "constants.h"
#include "db_adapter.h"
#include "dh_context.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "mock_db_change_listener.h"
#include "version_info_manager.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "DbAdapterTest"

namespace {
const string DATABASE_DIR = "/data/service/el1/public/database/dtbhardware_manager_service/";
const std::string DEV_NETWORK_ID_1 = "nt36a637105409e904d4da83790a4a9";
const string TEST_DEV_ID_0 = "bb536a637105409e904d4da83791aa11";
const string TEST_DEV_ID_1 = "bb536a637105409e904d4da83791bb22";
const string TEST_DEV_ID_2 = "bb536a637105409e904d4da83791bb33";
const string TEST_DEV_NAME = "Dev1";
const string TEST_DH_ID_0 = "Camera_00";
const string TEST_DH_ID_1 = "Mic_01";
const string TEST_DH_ID_2 = "Gps_02";
const string TEST_DH_ID_3 = "Display_03";
const string TEST_DH_ID_4 = "Input_04";
const string TEST_DH_ATTR_0 = "db_test_attr0";
const string TEST_DH_ATTR_1 = "db_test_attr1";
const string TEST_DH_SUBTYPE_0 = "camera";
const string TEST_DH_SUBTYPE_1 = "mic";
const string TEST_DH_SUBTYPE_2 = "gps";
const string TEST_DH_SUBTYPE_3 = "screen";
const string TEST_DH_SUBTYPE_4 = "input";
constexpr uint16_t TEST_DEV_TYPE = 0x11;

const shared_ptr<CapabilityInfo> CAP_INFO_0 =
    make_shared<CapabilityInfo>(TEST_DH_ID_0, TEST_DEV_ID_0, DEV_NAME, TEST_DEV_TYPE, DHType::CAMERA, TEST_DH_ATTR_0,
    TEST_DH_SUBTYPE_0);
const shared_ptr<CapabilityInfo> CAP_INFO_1 =
    make_shared<CapabilityInfo>(TEST_DH_ID_1, TEST_DEV_ID_0, DEV_NAME, TEST_DEV_TYPE, DHType::AUDIO, TEST_DH_ATTR_0,
    TEST_DH_SUBTYPE_1);
const shared_ptr<CapabilityInfo> CAP_INFO_2 =
    make_shared<CapabilityInfo>(TEST_DH_ID_2, TEST_DEV_ID_0, DEV_NAME, TEST_DEV_TYPE, DHType::GPS, TEST_DH_ATTR_0,
    TEST_DH_SUBTYPE_2);
const shared_ptr<CapabilityInfo> CAP_INFO_3 =
    make_shared<CapabilityInfo>(TEST_DH_ID_3, TEST_DEV_ID_0, DEV_NAME, TEST_DEV_TYPE, DHType::HFP, TEST_DH_ATTR_0,
    TEST_DH_SUBTYPE_3);
const shared_ptr<CapabilityInfo> CAP_INFO_4 =
    make_shared<CapabilityInfo>(TEST_DH_ID_4, TEST_DEV_ID_0, DEV_NAME, TEST_DEV_TYPE, DHType::INPUT, TEST_DH_ATTR_0,
    TEST_DH_SUBTYPE_4);

const shared_ptr<CapabilityInfo> CAP_INFO_5 =
    make_shared<CapabilityInfo>(TEST_DH_ID_0, TEST_DEV_ID_1, DEV_NAME, TEST_DEV_TYPE, DHType::CAMERA, TEST_DH_ATTR_1,
    TEST_DH_SUBTYPE_0);
const shared_ptr<CapabilityInfo> CAP_INFO_6 =
    make_shared<CapabilityInfo>(TEST_DH_ID_1, TEST_DEV_ID_1, DEV_NAME, TEST_DEV_TYPE, DHType::AUDIO, TEST_DH_ATTR_1,
    TEST_DH_SUBTYPE_1);
const shared_ptr<CapabilityInfo> CAP_INFO_7 =
    make_shared<CapabilityInfo>(TEST_DH_ID_2, TEST_DEV_ID_1, DEV_NAME, TEST_DEV_TYPE, DHType::GPS, TEST_DH_ATTR_1,
    TEST_DH_SUBTYPE_2);
const shared_ptr<CapabilityInfo> CAP_INFO_8 =
    make_shared<CapabilityInfo>(TEST_DH_ID_3, TEST_DEV_ID_1, DEV_NAME, TEST_DEV_TYPE, DHType::HFP, TEST_DH_ATTR_1,
    TEST_DH_SUBTYPE_3);
const shared_ptr<CapabilityInfo> CAP_INFO_9 =
    make_shared<CapabilityInfo>(TEST_DH_ID_4, TEST_DEV_ID_1, DEV_NAME, TEST_DEV_TYPE, DHType::INPUT, TEST_DH_ATTR_1,
    TEST_DH_SUBTYPE_4);

std::shared_ptr<DBAdapter> g_dbAdapterPtr;
}

void DbAdapterTest::SetUpTestCase(void)
{
    auto ret = mkdir(DATABASE_DIR.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (ret != 0) {
        DHLOGE("mkdir failed, path: %{public}s, errno : %{public}d", DATABASE_DIR.c_str(), errno);
    }
    std::shared_ptr<DistributedKv::KvStoreObserver> changeListener = std::make_shared<MockDBChangeListener>();
    g_dbAdapterPtr = std::make_shared<DBAdapter>(APP_ID, GLOBAL_CAPABILITY_ID, changeListener);
    if (g_dbAdapterPtr != nullptr) {
        g_dbAdapterPtr->Init(true, DistributedKv::DataType::TYPE_DYNAMICAL);
    }
}

void DbAdapterTest::TearDownTestCase(void)
{
    if (g_dbAdapterPtr != nullptr) {
        g_dbAdapterPtr->UnInit();
    }

    g_dbAdapterPtr = nullptr;

    auto ret = remove(DATABASE_DIR.c_str());
    if (ret != 0) {
        DHLOGE("remove dir failed, path: %{public}s, errno : %{public}d", DATABASE_DIR.c_str(), errno);
    }
}

void DbAdapterTest::SetUp()
{
}

void DbAdapterTest::TearDown()
{
}

/**
 * @tc.name: db_adapter_test_000
 * @tc.desc: Verify the Init function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(DbAdapterTest, db_adapter_test_000, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    EXPECT_EQ(DH_FWK_SUCCESS, g_dbAdapterPtr->Init(true, DistributedKv::DataType::TYPE_DYNAMICAL));
}

/**
 * @tc.name: db_adapter_test_001
 * @tc.desc: Verify the PutDataBatch function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJE
 */
HWTEST_F(DbAdapterTest, db_adapter_test_001, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    vector<shared_ptr<CapabilityInfo>> resInfos { CAP_INFO_0, CAP_INFO_1, CAP_INFO_2, CAP_INFO_3, CAP_INFO_4,
        CAP_INFO_5, CAP_INFO_6, CAP_INFO_7, CAP_INFO_8, CAP_INFO_9 };

    std::vector<std::string> keys;
    std::vector<std::string> values;
    std::string key;
    for (auto &resInfo : resInfos) {
        key = resInfo->GetKey();
        keys.push_back(key);
        values.push_back(resInfo->ToJsonString());
    }
    EXPECT_EQ(DH_FWK_SUCCESS, g_dbAdapterPtr->PutDataBatch(keys, values));
    for (auto &resInfo : resInfos) {
        g_dbAdapterPtr->RemoveDataByKey(resInfo->GetKey());
    }
}

/**
 * @tc.name: db_adapter_test_002
 * @tc.desc: Verify the PutDataBatch function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(DbAdapterTest, db_adapter_test_002, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    std::vector<std::string> keys { std::string(TEST_DEV_ID_2 + TEST_DH_ID_0) };
    std::vector<std::string> values { TEST_DH_ATTR_0 };

    EXPECT_EQ(DH_FWK_SUCCESS, g_dbAdapterPtr->PutDataBatch(keys, values));
    g_dbAdapterPtr->RemoveDataByKey(keys[0]);
}

/**
 * @tc.name: db_adapter_test_003
 * @tc.desc: Verify the PutDataBatch function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(DbAdapterTest, db_adapter_test_003, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    std::vector<std::string> keys { std::string(TEST_DEV_ID_2 + TEST_DH_ID_0) };
    std::vector<std::string> valuesEmpty;

    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, g_dbAdapterPtr->PutDataBatch(keys, valuesEmpty));
}

/**
 * @tc.name: db_adapter_test_004
 * @tc.desc: Verify the PutDataBatch function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(DbAdapterTest, db_adapter_test_004, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    std::vector<std::string> keysEmpty;
    std::vector<std::string> values { TEST_DH_ATTR_0 };

    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, g_dbAdapterPtr->PutDataBatch(keysEmpty, values));
}

/**
 * @tc.name: db_adapter_test_005
 * @tc.desc: Verify the PutDataBatch function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(DbAdapterTest, db_adapter_test_005, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    std::vector<std::string> keysEmpty;
    std::vector<std::string> valuesEmpty;

    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, g_dbAdapterPtr->PutDataBatch(keysEmpty, valuesEmpty));
}

/**
 * @tc.name: db_adapter_test_006
 * @tc.desc: Verify the PutDataBatch function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(DbAdapterTest, db_adapter_test_006, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    std::vector<std::string> keys { std::string(TEST_DEV_ID_2 + TEST_DH_ID_0) };
    std::vector<std::string> values { TEST_DH_ATTR_0, TEST_DH_ATTR_1 };

    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, g_dbAdapterPtr->PutDataBatch(keys, values));
}

/**
 * @tc.name: db_adapter_test_007
 * @tc.desc: Verify the PutDataBatch function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(DbAdapterTest, db_adapter_test_007, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    g_dbAdapterPtr->kvStoragePtr_ = nullptr;
    std::vector<std::string> keys { std::string(TEST_DEV_ID_2 + TEST_DH_ID_0) };
    std::vector<std::string> values { TEST_DH_ATTR_0 };

    EXPECT_EQ(ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL, g_dbAdapterPtr->PutDataBatch(keys, values));
}

/**
 * @tc.name: db_adapter_test_008
 * @tc.desc: Verify the ReInit function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(DbAdapterTest, db_adapter_test_008, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    g_dbAdapterPtr->kvStoragePtr_ = nullptr;
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL, g_dbAdapterPtr->ReInit(true));
}

/**
 * @tc.name: db_adapter_test_009
 * @tc.desc: Verify the RemoveDeviceData function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(DbAdapterTest, db_adapter_test_009, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    g_dbAdapterPtr->kvStoragePtr_ = nullptr;
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL, g_dbAdapterPtr->RemoveDeviceData(TEST_DEV_ID_0));
}

/**
 * @tc.name: db_adapter_test_010
 * @tc.desc: Verify the RemoveDataByKey function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(DbAdapterTest, db_adapter_test_010, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    g_dbAdapterPtr->kvStoragePtr_ = nullptr;
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL, g_dbAdapterPtr->RemoveDataByKey("key"));
}

/**
 * @tc.name: db_adapter_test_012
 * @tc.desc: Verify the UnRegisterChangeListener function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(DbAdapterTest, db_adapter_test_012, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    g_dbAdapterPtr->kvStoragePtr_ = nullptr;
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL, g_dbAdapterPtr->UnRegisterChangeListener());
}

/**
 * @tc.name: db_adapter_test_013
 * @tc.desc: Verify the PutData function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(DbAdapterTest, db_adapter_test_013, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    std::string key = std::string(TEST_DEV_ID_1 + TEST_DH_ID_1);
    std::string value = TEST_DH_ATTR_0;
    g_dbAdapterPtr->kvStoragePtr_ = nullptr;
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL, g_dbAdapterPtr->PutData(key, value));
}

/**
 * @tc.name: db_adapter_test_014
 * @tc.desc: Verify the RegisterChangeListener function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(DbAdapterTest, db_adapter_test_014, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    std::string networkId = DEV_NETWORK_ID_1;
    g_dbAdapterPtr->kvStoragePtr_ = nullptr;
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL, g_dbAdapterPtr->RegisterChangeListener());
}

/**
 * @tc.name: db_adapter_test_015
 * @tc.desc: Verify the UnRegisterChangeListener function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(DbAdapterTest, db_adapter_test_015, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    g_dbAdapterPtr->kvStoragePtr_ = nullptr;
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL, g_dbAdapterPtr->UnRegisterChangeListener());
}

/**
 * @tc.name: ReInit_001
 * @tc.desc: Verify the ReInit_001 function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(DbAdapterTest, ReInit_001, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    g_dbAdapterPtr->GetKvStorePtr(true, DistributedKv::DataType::TYPE_DYNAMICAL);
    EXPECT_EQ(DH_FWK_SUCCESS, g_dbAdapterPtr->ReInit(true));
}

/**
 * @tc.name: RemoveDeviceData_001
 * @tc.desc: Verify the RemoveDeviceData function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DbAdapterTest, RemoveDeviceData_001, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    g_dbAdapterPtr->GetKvStorePtr(true, DistributedKv::DataType::TYPE_DYNAMICAL);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_KV_STORAGE_OPERATION_FAIL, g_dbAdapterPtr->RemoveDeviceData(TEST_DEV_ID_0));
}

/**
 * @tc.name: PutData_01
 * @tc.desc: Verify the PutData function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(DbAdapterTest, PutData_01, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    std::string key;
    std::string value;
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, g_dbAdapterPtr->PutData(key, value));
}

/**
 * @tc.name: PutData_02
 * @tc.desc: Verify the PutData function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(DbAdapterTest, PutData_02, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    uint32_t MAX_MESSAGE_LEN = 40 * 1024 * 1024 +1 ;
    std::string key = "key";
    key.resize(MAX_MESSAGE_LEN);
    std::string value;
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, g_dbAdapterPtr->PutData(key, value));
}

/**
 * @tc.name: PutData_03
 * @tc.desc: Verify the PutData function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(DbAdapterTest, PutData_03, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    std::string key = "key";
    std::string value;
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, g_dbAdapterPtr->PutData(key, value));
}

/**
 * @tc.name: PutData_04
 * @tc.desc: Verify the PutData function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(DbAdapterTest, PutData_04, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    uint32_t MAX_MESSAGE_LEN = 40 * 1024 * 1024 + 1;
    std::string key = "key";
    std::string value;
    value.resize(MAX_MESSAGE_LEN);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, g_dbAdapterPtr->PutData(key, value));
}

/**
 * @tc.name: PutData_05
 * @tc.desc: Verify the PutData function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSCV
 */
HWTEST_F(DbAdapterTest, PutData_05, TestSize.Level0)
{
    if (g_dbAdapterPtr == nullptr) {
        return;
    }
    std::string key = "key";
    std::string value = "value";
    g_dbAdapterPtr->kvStoragePtr_ = nullptr;
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL, g_dbAdapterPtr->PutData(key, value));
}
} // namespace DistributedHardware
} // namespace OHOS
