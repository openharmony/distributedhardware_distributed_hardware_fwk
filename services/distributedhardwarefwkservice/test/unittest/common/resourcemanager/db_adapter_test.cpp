/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "db_adapter.h"

#include <gtest/gtest.h>

#include "cJSON.h"

#include "distributed_kv_data_manager.h"
#include "kvstore_observer.h"

#include "constants.h"
#include "capability_info_manager.h"
#include "distributed_hardware_errno.h"
#include "dh_utils_tool.h"
#include "dh_context.h"
#include "meta_info_manager.h"
#include "version_info_manager.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
namespace {
const string DEV_ID_TEST = "123456789";
const string DH_ID_TEST = "Camera_0";
const string UUID_TEST = "111111";
const string UDID_TEST = "222222";
const string NETWORKID_TEST = "333333";
}

class DBAdapterTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DBAdapterTest::SetUp()
{
}

void DBAdapterTest::TearDown() {}

void DBAdapterTest::SetUpTestCase()
{
    MetaInfoManager::GetInstance()->Init();
}

void DBAdapterTest::TearDownTestCase()
{
    MetaInfoManager::GetInstance()->UnInit();
}

HWTEST_F(DBAdapterTest, GetNetworkIdByKey_001, TestSize.Level1)
{
    ASSERT_TRUE(MetaInfoManager::GetInstance()->dbAdapterPtr_ != nullptr);
    std::string key = "";
    auto ret = MetaInfoManager::GetInstance()->dbAdapterPtr_->GetNetworkIdByKey(key);
    EXPECT_TRUE(ret.empty());

    key = DEV_ID_TEST + RESOURCE_SEPARATOR + DH_ID_TEST;
    ret = MetaInfoManager::GetInstance()->dbAdapterPtr_->GetNetworkIdByKey(key);
    EXPECT_TRUE(ret.empty());
}

HWTEST_F(DBAdapterTest, SyncByNotFound_001, TestSize.Level1)
{
    ASSERT_TRUE(MetaInfoManager::GetInstance()->dbAdapterPtr_ != nullptr);
    std::string key = "";
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->dbAdapterPtr_->SyncByNotFound(key));
}

HWTEST_F(DBAdapterTest, SyncByNotFound_002, TestSize.Level1)
{
    ASSERT_TRUE(MetaInfoManager::GetInstance()->dbAdapterPtr_ != nullptr);
    DHContext::GetInstance().AddOnlineDevice(UDID_TEST, UUID_TEST, NETWORKID_TEST);
    std::string devId = Sha256(UUID_TEST);
    std::string key = devId + RESOURCE_SEPARATOR + DH_ID_TEST;
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->dbAdapterPtr_->SyncByNotFound(key));
    DHContext::GetInstance().RemoveOnlineDeviceIdEntryByNetworkId(NETWORKID_TEST);
}

HWTEST_F(DBAdapterTest, SyncDBForRecover_001, TestSize.Level1)
{
    ASSERT_TRUE(MetaInfoManager::GetInstance()->dbAdapterPtr_ != nullptr);
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->dbAdapterPtr_->SyncDBForRecover());

    CapabilityInfoManager::GetInstance()->Init();
    ASSERT_TRUE(CapabilityInfoManager::GetInstance()->dbAdapterPtr_ != nullptr);
    ASSERT_NO_FATAL_FAILURE(CapabilityInfoManager::GetInstance()->dbAdapterPtr_->SyncDBForRecover());
    CapabilityInfoManager::GetInstance()->UnInit();

    VersionInfoManager::GetInstance()->Init();
    ASSERT_TRUE(VersionInfoManager::GetInstance()->dbAdapterPtr_ != nullptr);
    ASSERT_NO_FATAL_FAILURE(VersionInfoManager::GetInstance()->dbAdapterPtr_->SyncDBForRecover());
    VersionInfoManager::GetInstance()->UnInit();
}

HWTEST_F(DBAdapterTest, RemoveDeviceData_001, TestSize.Level1)
{
    ASSERT_TRUE(MetaInfoManager::GetInstance()->dbAdapterPtr_ != nullptr);
    std::string deviceId = "";
    auto ret = MetaInfoManager::GetInstance()->dbAdapterPtr_->RemoveDeviceData(deviceId);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(DBAdapterTest, RemoveDataByKey_001, TestSize.Level1)
{
    ASSERT_TRUE(MetaInfoManager::GetInstance()->dbAdapterPtr_ != nullptr);
    std::string key = "";
    auto ret = MetaInfoManager::GetInstance()->dbAdapterPtr_->RemoveDataByKey(key);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(DBAdapterTest, GetEntriesByKeys_001, TestSize.Level1)
{
    ASSERT_TRUE(MetaInfoManager::GetInstance()->dbAdapterPtr_ != nullptr);
    std::vector<std::string> keys;
    auto ret = MetaInfoManager::GetInstance()->dbAdapterPtr_->GetEntriesByKeys(keys);
    EXPECT_TRUE(ret.empty());
}

HWTEST_F(DBAdapterTest, GetEntriesByKeys_002, TestSize.Level1)
{
    ASSERT_TRUE(MetaInfoManager::GetInstance()->dbAdapterPtr_ != nullptr);
    std::vector<std::string> keys;
    std::string key = "123456";
    keys.push_back(key);
    MetaInfoManager::GetInstance()->dbAdapterPtr_->kvStoragePtr_ = nullptr;
    auto ret = MetaInfoManager::GetInstance()->dbAdapterPtr_->GetEntriesByKeys(keys);
    EXPECT_TRUE(ret.empty());
}

HWTEST_F(DBAdapterTest, SyncDataByNetworkId_001, TestSize.Level1)
{
    ASSERT_TRUE(MetaInfoManager::GetInstance()->dbAdapterPtr_ != nullptr);
    std::string networkId = "123456789";
    MetaInfoManager::GetInstance()->dbAdapterPtr_->kvStoragePtr_ = nullptr;
    auto ret = MetaInfoManager::GetInstance()->dbAdapterPtr_->SyncDataByNetworkId(networkId);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DBAdapterTest, ClearDataByPrefix_001, TestSize.Level1)
{
    ASSERT_TRUE(MetaInfoManager::GetInstance()->dbAdapterPtr_ != nullptr);
    std::string prefix = "123###456";
    MetaInfoManager::GetInstance()->dbAdapterPtr_->kvStoragePtr_ = nullptr;
    auto ret = MetaInfoManager::GetInstance()->dbAdapterPtr_->ClearDataByPrefix(prefix);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DBAdapterTest, CloudSyncData_001, TestSize.Level1)
{
    ASSERT_TRUE(MetaInfoManager::GetInstance()->dbAdapterPtr_ != nullptr);
    MetaInfoManager::GetInstance()->dbAdapterPtr_->kvStoragePtr_ = nullptr;
    auto ret = MetaInfoManager::GetInstance()->dbAdapterPtr_->CloudSyncData();
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL, ret);
}

HWTEST_F(DBAdapterTest, CloudSyncCallback_001, TestSize.Level1)
{
    ASSERT_TRUE(MetaInfoManager::GetInstance()->dbAdapterPtr_ != nullptr);
    DistributedKv::ProgressDetail detail;
    detail.code = DistributedKv::Status::SUCCESS;
    detail.progress = DistributedKv::Progress::SYNC_FINISH;
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->dbAdapterPtr_->CloudSyncCallback(std::move(detail)));
}

HWTEST_F(DBAdapterTest, CloudSyncCallback_002, TestSize.Level1)
{
    ASSERT_TRUE(MetaInfoManager::GetInstance()->dbAdapterPtr_ != nullptr);
    DistributedKv::ProgressDetail detail;
    detail.code = DistributedKv::Status::SUCCESS;
    detail.progress = DistributedKv::Progress::SYNC_IN_PROGRESS;
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->dbAdapterPtr_->CloudSyncCallback(std::move(detail)));
}

HWTEST_F(DBAdapterTest, CloudSyncCallback_003, TestSize.Level1)
{
    ASSERT_TRUE(MetaInfoManager::GetInstance()->dbAdapterPtr_ != nullptr);
    DistributedKv::ProgressDetail detail;
    detail.code = DistributedKv::Status::ERROR;
    detail.progress = DistributedKv::Progress::SYNC_FINISH;
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->dbAdapterPtr_->CloudSyncCallback(std::move(detail)));
}

HWTEST_F(DBAdapterTest, CloudSyncCallback_004, TestSize.Level1)
{
    ASSERT_TRUE(MetaInfoManager::GetInstance()->dbAdapterPtr_ != nullptr);
    DistributedKv::ProgressDetail detail;
    detail.code = DistributedKv::Status::ERROR;
    detail.progress = DistributedKv::Progress::SYNC_IN_PROGRESS;
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->dbAdapterPtr_->CloudSyncCallback(std::move(detail)));
}

HWTEST_F(DBAdapterTest, CloudSyncCallback_005, TestSize.Level1)
{
    ASSERT_TRUE(MetaInfoManager::GetInstance()->dbAdapterPtr_ != nullptr);
    DistributedKv::ProgressDetail detail;
    detail.code = DistributedKv::Status::TIME_OUT;
    detail.progress = DistributedKv::Progress::SYNC_IN_PROGRESS;
    ASSERT_NO_FATAL_FAILURE(MetaInfoManager::GetInstance()->dbAdapterPtr_->CloudSyncCallback(std::move(detail)));
}
}
}