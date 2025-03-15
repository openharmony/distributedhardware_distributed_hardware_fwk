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

#include "utils_tool_test.h"

#include <chrono>
#include <cstdint>
#include <gtest/gtest.h>
#include <set>
#include <string>
#include <thread>

#include "anonymous_string.h"
#include "dh_utils_tool.h"
#include "dh_utils_hitrace.h"
#include "distributed_hardware_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr int32_t UUID_LENGTH = 257;
    constexpr uint32_t JSON_SIZE = 40 * 1024 * 1024 + 1;
    constexpr uint32_t KEY_SIZE = 257;
    constexpr int32_t OS_TYPE = 10;
}
void UtilsToolTest::SetUpTestCase(void)
{
}

void UtilsToolTest::TearDownTestCase(void)
{
}

void UtilsToolTest::SetUp()
{
}

void UtilsToolTest::TearDown()
{
}

/**
 * @tc.name: GetAnonyString_001
 * @tc.desc: Verify the GetAnonyString function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(UtilsToolTest, GetAnonyString_001, TestSize.Level1)
{
    std::string value = "";
    auto ret = GetAnonyString(value);
    EXPECT_EQ("", ret);

    value = "11";
    ret = GetAnonyString(value);
    EXPECT_EQ("******", ret);

    value = "123456789";
    ret = GetAnonyString(value);
    EXPECT_EQ("1******9", ret);

    value = "111222333444555666777888999";
    ret = GetAnonyString(value);
    EXPECT_EQ("1112******8999", ret);
}

/**
 * @tc.name: GetAnonyInt32_001
 * @tc.desc: Verify the GetAnnoyInt32 function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(UtilsToolTest, GetAnonyInt32_001, TestSize.Level1)
{
    int32_t value = 123456;
    auto ret = GetAnonyInt32(value);
    EXPECT_EQ("1*****", ret);
}

/**
 * @tc.name: GetUUIDByDm_001
 * @tc.desc: Verify the GetUUIDBySoftBus function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(UtilsToolTest, GetUUIDByDm_001, TestSize.Level1)
{
    std::string networkId = "";
    std::string ret = GetUUIDByDm(networkId);
    EXPECT_EQ(0, ret.size());
}

HWTEST_F(UtilsToolTest, GetUDIDByDm_001, TestSize.Level1)
{
    std::string networkId = "";
    std::string ret = GetUDIDByDm(networkId);
    EXPECT_EQ(0, ret.size());
}

/**
 * @tc.name: GetDeviceIdByUUID_001
 * @tc.desc: Verify the GetDeviceIdByUUID function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(UtilsToolTest, GetDeviceIdByUUID_001, TestSize.Level1)
{
    std::string uuidEmpty = "";
    std::string ret = GetDeviceIdByUUID(uuidEmpty);
    EXPECT_EQ(0, ret.size());

    std::string uuid(UUID_LENGTH, '1');
    ret = GetDeviceIdByUUID(uuid);
    EXPECT_EQ(0, ret.size());
}

/**
 * @tc.name: GetDeviceIdByUUID_002
 * @tc.desc: Verify the GetDeviceIdByUUID function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(UtilsToolTest, GetDeviceIdByUUID_002, TestSize.Level1)
{
    DHType dhType = DHType::CAMERA;
    DHQueryTraceStart(dhType);

    dhType = DHType::MAX_DH;
    DHQueryTraceStart(dhType);

    std::string uuid = "bb536a637105409e904d4da78290ab1";
    std::string ret = GetDeviceIdByUUID(uuid);
    EXPECT_NE(0, ret.size());
}

HWTEST_F(UtilsToolTest, IsUInt8_001, TestSize.Level1)
{
    cJSON* jsonObj1 = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj1 != nullptr);
    const std::string key = "int8_key";
    cJSON_AddStringToObject(jsonObj1, key.c_str(), "int8_key_test");
    cJSON *keyJson1 = cJSON_GetObjectItem(jsonObj1, key.c_str());
    auto ret = IsUInt8(keyJson1);
    EXPECT_EQ(false, ret);
    cJSON_Delete(jsonObj1);

    cJSON* jsonObj2 = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj2 != nullptr);
    cJSON_AddNumberToObject(jsonObj2, key.c_str(), 1);
    cJSON *keyJson2 = cJSON_GetObjectItem(jsonObj2, key.c_str());
    ret = IsUInt8(keyJson2);
    EXPECT_EQ(true, ret);
    cJSON_Delete(jsonObj2);
}

HWTEST_F(UtilsToolTest, IsUInt16_001, TestSize.Level1)
{
    cJSON* jsonObj1 = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj1 != nullptr);
    const std::string key = "uint16_key";
    cJSON_AddStringToObject(jsonObj1, key.c_str(), "uint16_key_test");
    cJSON *keyJson1 = cJSON_GetObjectItem(jsonObj1, key.c_str());
    auto ret = IsUInt16(keyJson1);
    cJSON_Delete(jsonObj1);
    EXPECT_EQ(false, ret);

    cJSON* jsonObj2 = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj2 != nullptr);
    cJSON_AddNumberToObject(jsonObj2, key.c_str(), 1);
    cJSON *keyJson2 = cJSON_GetObjectItem(jsonObj2, key.c_str());
    ret = IsUInt16(keyJson2);
    EXPECT_EQ(true, ret);
    cJSON_Delete(jsonObj2);
}

HWTEST_F(UtilsToolTest, IsUInt32_001, TestSize.Level1)
{
    cJSON* jsonObj1 = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj1 != nullptr);
    const std::string key = "uint32_key";
    cJSON_AddStringToObject(jsonObj1, key.c_str(), "uint32_key_test");
    cJSON *keyJson1 = cJSON_GetObjectItem(jsonObj1, key.c_str());
    auto ret = IsUInt32(keyJson1);
    EXPECT_EQ(false, ret);
    cJSON_Delete(jsonObj1);

    cJSON* jsonObj2 = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj2 != nullptr);
    cJSON_AddNumberToObject(jsonObj2, key.c_str(), 1);
    cJSON *keyJson2 = cJSON_GetObjectItem(jsonObj2, key.c_str());
    ret = IsUInt32(keyJson2);
    EXPECT_EQ(true, ret);
    cJSON_Delete(jsonObj2);
}

HWTEST_F(UtilsToolTest, GetSysPara_001, TestSize.Level1)
{
    char *key = nullptr;
    bool value = false;
    auto ret = GetSysPara(key, value);
    EXPECT_EQ(false, ret);
}

HWTEST_F(UtilsToolTest, IsJsonLengthValid_001, TestSize.Level1)
{
    std::string inputJsonStr = "";
    auto ret = IsJsonLengthValid(inputJsonStr);
    EXPECT_EQ(false, ret);

    std::string jsonStr(JSON_SIZE, 'a');
    ret = IsJsonLengthValid(jsonStr);
    EXPECT_EQ(false, ret);
}

HWTEST_F(UtilsToolTest, IsKeySizeValid_001, TestSize.Level1)
{
    std::string inputKey = "";
    auto ret = IsKeySizeValid(inputKey);
    EXPECT_EQ(false, ret);

    std::string key(KEY_SIZE, 'a');
    ret = IsKeySizeValid(key);
    EXPECT_EQ(false, ret);
}

HWTEST_F(UtilsToolTest, GetDeviceSystemType_001, TestSize.Level1)
{
    std::string extraData = "";
    auto ret = GetDeviceSystemType(extraData);
    EXPECT_EQ(ERR_DH_FWK_INVALID_OSTYPE, ret);
}

HWTEST_F(UtilsToolTest, GetDeviceSystemType_002, TestSize.Level1)
{
    cJSON* jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    std::string key = "key";
    cJSON_AddStringToObject(jsonObj, key.c_str(), "key_test");
    char* cjson = cJSON_PrintUnformatted(jsonObj);
    if (cjson == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    std::string extraData(cjson);
    auto ret = GetDeviceSystemType(extraData);
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
    EXPECT_EQ(ERR_DH_FWK_INVALID_OSTYPE, ret);
}

HWTEST_F(UtilsToolTest, GetDeviceSystemType_003, TestSize.Level1)
{
    cJSON* jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    const std::string key = "OS_TYPE";
    cJSON_AddStringToObject(jsonObj, key.c_str(), "key_test");
    char* cjson = cJSON_PrintUnformatted(jsonObj);
    if (cjson == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    std::string extraData(cjson);
    auto ret = GetDeviceSystemType(extraData);
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
    EXPECT_EQ(ERR_DH_FWK_INVALID_OSTYPE, ret);
}

HWTEST_F(UtilsToolTest, GetDeviceSystemType_004, TestSize.Level1)
{
    cJSON* jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    const std::string key = "OS_TYPE";
    cJSON_AddNumberToObject(jsonObj, key.c_str(), OS_TYPE);
    char* cjson = cJSON_PrintUnformatted(jsonObj);
    if (cjson == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    std::string extraData(cjson);
    auto ret = GetDeviceSystemType(extraData);
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
    EXPECT_EQ(OS_TYPE, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
