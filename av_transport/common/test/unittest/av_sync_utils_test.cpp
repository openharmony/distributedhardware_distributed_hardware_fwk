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

#include "av_sync_utils.h"

#include "av_trans_constants.h"
#include "av_trans_errno.h"
#include "access_listener_service.h"
#include "cJSON.h"
#include "ashmem_mock.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
using namespace std;
class AvSyncUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
public:
    std::shared_ptr<MockAshmem> ashmemMock_;
};

void AvSyncUtilsTest::SetUpTestCase()
{
}

void AvSyncUtilsTest::TearDownTestCase()
{
}

void AvSyncUtilsTest::SetUp()
{
    auto ashmem_ = IAshmem::GetOrCreateInstance();
    ashmemMock_ = std::static_pointer_cast<MockAshmem>(ashmem_);
}

void AvSyncUtilsTest::TearDown()
{
    IAshmem::ReleaseInstance();
    ashmemMock_ = nullptr;
}

HWTEST_F(AvSyncUtilsTest, CreateAVTransSharedMemory_001, TestSize.Level0)
{
    std::string name = "";
    size_t size = 0;
    auto ret = CreateAVTransSharedMemory(name, size);
    EXPECT_EQ(0, ret.fd);
}

HWTEST_F(AvSyncUtilsTest, CloseAVTransSharedMemory_001, TestSize.Level0)
{
    AVTransSharedMemory memory = {
        .fd = -1,
        .size = 0,
        .name = "",
    };
    CloseAVTransSharedMemory(memory);
    EXPECT_EQ(0, memory.size);
}

HWTEST_F(AvSyncUtilsTest, CloseAVTransSharedMemory_002, TestSize.Level1)
{
    AVTransSharedMemory memory = {
        .fd = -1,
        .size = 0,
        .name = "name_test",
    };
    CloseAVTransSharedMemory(memory);
    EXPECT_EQ(0, memory.size);

    AVTransSharedMemory memory1 = {
        .fd = 1,
        .size = 0,
        .name = "name_test",
    };
    CloseAVTransSharedMemory(memory1);
    EXPECT_EQ(0, memory1.size);
}

HWTEST_F(AvSyncUtilsTest, MarshalSharedMemory_001, TestSize.Level0)
{
    AVTransSharedMemory memory = {
        .fd = 1,
        .size = 100,
        .name = "name_test",
    };
    auto ret = MarshalSharedMemory(memory);
    EXPECT_EQ(false, ret.empty());
}

HWTEST_F(AvSyncUtilsTest, UnmarshalSharedMemory_001, TestSize.Level0)
{
    std::string jsonStr = "jsonStr_test";
    auto ret = UnmarshalSharedMemory(jsonStr);
    EXPECT_EQ(0, ret.fd);
}

HWTEST_F(AvSyncUtilsTest, UnmarshalSharedMemory_002, TestSize.Level1)
{
    cJSON *cJsonObj = cJSON_CreateObject();
    cJSON_AddStringToObject(cJsonObj, KEY_SHARED_MEM_FD.c_str(), "mem_fd_test");
    char* cjson = cJSON_PrintUnformatted(cJsonObj);
    std::string jsonStr(cjson);
    auto ret = UnmarshalSharedMemory(jsonStr);
    EXPECT_EQ(0, ret.fd);
    cJSON_free(cjson);
    cJSON_Delete(cJsonObj);

    cJSON *cJsonObj1 = cJSON_CreateObject();
    cJSON_AddNumberToObject(cJsonObj1, KEY_SHARED_MEM_FD.c_str(), 1);
    cJSON_AddStringToObject(cJsonObj1, KEY_SHARED_MEM_SIZE.c_str(), "mem_size_test");
    char* cjson1 = cJSON_PrintUnformatted(cJsonObj1);
    std::string jsonStr1(cjson);
    ret = UnmarshalSharedMemory(jsonStr1);
    EXPECT_EQ(0, ret.fd);
    cJSON_free(cjson1);
    cJSON_Delete(cJsonObj1);

    cJSON *cJsonObj2 = cJSON_CreateObject();
    cJSON_AddNumberToObject(cJsonObj2, KEY_SHARED_MEM_FD.c_str(), 1);
    cJSON_AddNumberToObject(cJsonObj2, KEY_SHARED_MEM_SIZE.c_str(), 100);
    cJSON_AddNumberToObject(cJsonObj2, KEY_SHARED_MEM_NAME.c_str(), 1);
    char* cjson2 = cJSON_PrintUnformatted(cJsonObj2);
    std::string jsonStr2(cjson2);
    ret = UnmarshalSharedMemory(jsonStr2);
    EXPECT_EQ(0, ret.fd);
    cJSON_free(cjson2);
    cJSON_Delete(cJsonObj2);

    cJSON *cJsonObj3 = cJSON_CreateObject();
    cJSON_AddNumberToObject(cJsonObj3, KEY_SHARED_MEM_FD.c_str(), 1);
    cJSON_AddNumberToObject(cJsonObj3, KEY_SHARED_MEM_SIZE.c_str(), 100);
    cJSON_AddStringToObject(cJsonObj3, KEY_SHARED_MEM_NAME.c_str(), "mem_name_test");
    char* cjson3 = cJSON_PrintUnformatted(cJsonObj3);
    std::string jsonStr3(cjson3);
    ret = UnmarshalSharedMemory(jsonStr3);
    EXPECT_EQ("mem_name_test", ret.name);
    cJSON_free(cjson3);
    cJSON_Delete(cJsonObj3);
}

HWTEST_F(AvSyncUtilsTest, WriteClockUnitToMemory_001, TestSize.Level1)
{
    AVTransSharedMemory memory = {
        .fd = 1,
        .size = 100,
        .name = "name_test",
    };
    AVSyncClockUnit unit = {
        .index = 1,
        .frameNum = 1,
        .pts = 1,
    };
    auto ret = WriteClockUnitToMemory(memory, unit);
    EXPECT_EQ(false, ret == 0);
}

HWTEST_F(AvSyncUtilsTest, SetAccessConfig_001, TestSize.Level1)
{
    DAudioAccessConfigManager::GetInstance().ClearAccessConfig();
    sptr<IAccessListener> listenerNull = nullptr;
    int32_t timeOut = 0;
    std::string pkgName = "pkgName";
    int32_t ret = DAudioAccessConfigManager::GetInstance().SetAccessConfig(listenerNull, timeOut, pkgName);
    EXPECT_EQ(ret, ERR_DH_AVT_INVALID_PARAM);
    sptr<IAccessListener> listener(new AccessListenerService());
    std::string pkgNameNull = "";
    ret = DAudioAccessConfigManager::GetInstance().SetAccessConfig(listener, timeOut, pkgNameNull);
    EXPECT_EQ(ret, ERR_DH_AVT_INVALID_PARAM);
    ret = DAudioAccessConfigManager::GetInstance().SetAccessConfig(listener, timeOut, pkgName);
    EXPECT_EQ(ret, DH_AVT_SUCCESS);
}

HWTEST_F(AvSyncUtilsTest, IsAuthorizationGranted_001, TestSize.Level1)
{
    DAudioAccessConfigManager::GetInstance().ClearAccessConfig();
    std::string networkId = "0";
    std::string networkIdDiff = "1";
    bool ret = DAudioAccessConfigManager::GetInstance().IsAuthorizationGranted(networkId);
    EXPECT_EQ(ret, false);
    DAudioAccessConfigManager::GetInstance().SetAuthorizationGranted(networkIdDiff, true);
    ret = DAudioAccessConfigManager::GetInstance().IsAuthorizationGranted(networkId);
    EXPECT_EQ(ret, false);
    DAudioAccessConfigManager::GetInstance().SetAuthorizationGranted(networkId, false);
    ret = DAudioAccessConfigManager::GetInstance().IsAuthorizationGranted(networkId);
    EXPECT_EQ(ret, false);
    DAudioAccessConfigManager::GetInstance().SetAuthorizationGranted(networkId, true);
    ret = DAudioAccessConfigManager::GetInstance().IsAuthorizationGranted(networkId);
    EXPECT_EQ(ret, true);
}

HWTEST_F(AvSyncUtilsTest, ClearAuthorizationResult_001, TestSize.Level1)
{
    DAudioAccessConfigManager::GetInstance().ClearAccessConfig();
    std::string networkId = "0";
    DAudioAccessConfigManager::GetInstance().ClearAuthorizationResult(networkId);
    DAudioAccessConfigManager::GetInstance().SetAuthorizationGranted(networkId, true);
    DAudioAccessConfigManager::GetInstance().ClearAuthorizationResult(networkId);
    bool ret = DAudioAccessConfigManager::GetInstance().HasAuthorizationDecision(networkId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(AvSyncUtilsTest, WaitForAuthorizationResult_001, TestSize.Level1)
{
    DAudioAccessConfigManager::GetInstance().ClearAccessConfig();
    std::string networkId = "0";
    bool ret = DAudioAccessConfigManager::GetInstance().WaitForAuthorizationResult(networkId);
    DAudioAccessConfigManager::GetInstance().SetAuthorizationGranted(networkId, true);
    ret = DAudioAccessConfigManager::GetInstance().WaitForAuthorizationResult(networkId);
    EXPECT_EQ(ret, true);
}

HWTEST_F(AvSyncUtilsTest, ClearAccessConfigByPkgName_001, TestSize.Level1)
{
    DAudioAccessConfigManager::GetInstance().ClearAccessConfig();
    std::string pkgNameNull = "";
    std::string networkId = "0";
    DAudioAccessConfigManager::GetInstance().ClearAccessConfigByPkgName(pkgNameNull);
    std::string pkgName = "pkgName";
    DAudioAccessConfigManager::GetInstance().ClearAccessConfigByPkgName(pkgName);
    DAudioAccessConfigManager::GetInstance().pkgName_ = pkgName;
    DAudioAccessConfigManager::GetInstance().SetAuthorizationGranted(networkId, true);
    DAudioAccessConfigManager::GetInstance().ClearAccessConfigByPkgName(pkgName);
    bool ret = DAudioAccessConfigManager::GetInstance().HasAuthorizationDecision(networkId);
    EXPECT_EQ(ret, false);
}
}
}