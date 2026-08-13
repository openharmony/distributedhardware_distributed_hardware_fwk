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
#include "capability_utils.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
class CapabilityInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CapabilityInfoTest::SetUp() {}

void CapabilityInfoTest::TearDown() {}

void CapabilityInfoTest::SetUpTestCase() {}

void CapabilityInfoTest::TearDownTestCase() {}

HWTEST_F(CapabilityInfoTest, ToJson_001, TestSize.Level1)
{
    cJSON *jsonObject = nullptr;
    CapabilityInfo capability;
    ToJson(jsonObject, capability);
    EXPECT_TRUE(capability.GetDHId().empty());
}

HWTEST_F(CapabilityInfoTest, FromJson_001, TestSize.Level1)
{
    cJSON *jsonObject = nullptr;
    CapabilityInfo capability;
    FromJson(jsonObject, capability);
    EXPECT_TRUE(capability.GetDHId().empty());
}

HWTEST_F(CapabilityInfoTest, FromJson_002, TestSize.Level1)
{
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddNumberToObject(jsonObj, DH_ID, 1);
    char* cjson = cJSON_PrintUnformatted(jsonObj);
    if (cjson == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    std::string jsonStr(cjson);
    CapabilityInfo capability;
    FromJson(jsonObj, capability);
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
    EXPECT_TRUE(capability.GetDeviceId().empty());
}

HWTEST_F(CapabilityInfoTest, FromJson_003, TestSize.Level1)
{
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, DH_ID, "dhid_test");
    cJSON_AddNumberToObject(jsonObj, DEV_ID, 1);
    char* cjson = cJSON_PrintUnformatted(jsonObj);
    if (cjson == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    std::string jsonStr(cjson);
    CapabilityInfo capability;
    FromJson(jsonObj, capability);
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
    EXPECT_FALSE(capability.GetDHId().empty());
}

HWTEST_F(CapabilityInfoTest, FromJson_004, TestSize.Level1)
{
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, DH_ID, "dhid_test");
    cJSON_AddStringToObject(jsonObj, DEV_ID, "devid_test");
    cJSON_AddNumberToObject(jsonObj, DEV_NAME, 1);
    char* cjson = cJSON_PrintUnformatted(jsonObj);
    if (cjson == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    std::string jsonStr(cjson);
    CapabilityInfo capability;
    FromJson(jsonObj, capability);
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
    EXPECT_FALSE(capability.GetDHId().empty());
}

HWTEST_F(CapabilityInfoTest, FromJson_005, TestSize.Level1)
{
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, DH_ID, "dhid_test");
    cJSON_AddStringToObject(jsonObj, DEV_ID, "devid_test");
    cJSON_AddStringToObject(jsonObj, DEV_NAME, "devname_test");
    cJSON_AddStringToObject(jsonObj, DEV_TYPE, "devtype_test");
    char* cjson = cJSON_PrintUnformatted(jsonObj);
    if (cjson == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    std::string jsonStr(cjson);
    CapabilityInfo capability;
    FromJson(jsonObj, capability);
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
    EXPECT_FALSE(capability.GetDHId().empty());
}

HWTEST_F(CapabilityInfoTest, FromJson_006, TestSize.Level1)
{
    const uint16_t devType = 1;
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, DH_ID, "dhid_test");
    cJSON_AddStringToObject(jsonObj, DEV_ID, "devid_test");
    cJSON_AddStringToObject(jsonObj, DEV_NAME, "devname_test");
    cJSON_AddNumberToObject(jsonObj, DEV_TYPE, devType);
    cJSON_AddStringToObject(jsonObj, DH_TYPE, "dhtype_test");
    char* cjson = cJSON_PrintUnformatted(jsonObj);
    if (cjson == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    std::string jsonStr(cjson);
    CapabilityInfo capability;
    FromJson(jsonObj, capability);
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
    EXPECT_FALSE(capability.GetDHId().empty());
}

HWTEST_F(CapabilityInfoTest, FromJson_007, TestSize.Level1)
{
    const uint16_t devType = 1;
    const uint32_t dhType = 1;
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, DH_ID, "dhid_test");
    cJSON_AddStringToObject(jsonObj, DEV_ID, "devid_test");
    cJSON_AddStringToObject(jsonObj, DEV_NAME, "devname_test");
    cJSON_AddNumberToObject(jsonObj, DEV_TYPE, devType);
    cJSON_AddNumberToObject(jsonObj, DH_TYPE, dhType);
    cJSON_AddNumberToObject(jsonObj, DH_ATTRS, 1);
    char* cjson = cJSON_PrintUnformatted(jsonObj);
    if (cjson == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    std::string jsonStr(cjson);
    CapabilityInfo capability;
    FromJson(jsonObj, capability);
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
    EXPECT_FALSE(capability.GetDHId().empty());
}

HWTEST_F(CapabilityInfoTest, FromJson_008, TestSize.Level1)
{
    const uint16_t devType = 1;
    const uint32_t dhType = 1;
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, DH_ID, "dhid_test");
    cJSON_AddStringToObject(jsonObj, DEV_ID, "devid_test");
    cJSON_AddStringToObject(jsonObj, DEV_NAME, "devname_test");
    cJSON_AddNumberToObject(jsonObj, DEV_TYPE, devType);
    cJSON_AddNumberToObject(jsonObj, DH_TYPE, dhType);
    cJSON_AddStringToObject(jsonObj, DH_ATTRS, "dhattrs_test");
    cJSON_AddNumberToObject(jsonObj, DH_SUBTYPE, 1);
    char* cjson = cJSON_PrintUnformatted(jsonObj);
    if (cjson == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    std::string jsonStr(cjson);
    CapabilityInfo capability;
    FromJson(jsonObj, capability);
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
    EXPECT_FALSE(capability.GetDHId().empty());
}

HWTEST_F(CapabilityInfoTest, FromJson_009, TestSize.Level1)
{
    const uint16_t devType = 1;
    const uint32_t dhType = 1;
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, DH_ID, "dhid_test");
    cJSON_AddStringToObject(jsonObj, DEV_ID, "devid_test");
    cJSON_AddStringToObject(jsonObj, DEV_NAME, "devname_test");
    cJSON_AddNumberToObject(jsonObj, DEV_TYPE, devType);
    cJSON_AddNumberToObject(jsonObj, DH_TYPE, dhType);
    cJSON_AddStringToObject(jsonObj, DH_ATTRS, "dhattrs_test");
    cJSON_AddStringToObject(jsonObj, DH_SUBTYPE, "dhsubtype_test");
    char* cjson = cJSON_PrintUnformatted(jsonObj);
    if (cjson == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    std::string jsonStr(cjson);
    CapabilityInfo capability;
    FromJson(jsonObj, capability);
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
    EXPECT_FALSE(capability.GetDHId().empty());
}

HWTEST_F(CapabilityInfoTest, IsCapKeyMatchDeviceId_001, TestSize.Level1)
{
    std::string key = "";
    std::string deviceId = "";
    auto ret = IsCapKeyMatchDeviceId(key, deviceId);
    EXPECT_EQ(false, ret);

    key = "key###123";
    ret = IsCapKeyMatchDeviceId(key, deviceId);
    EXPECT_EQ(false, ret);

    key = "";
    deviceId = "123456";
    ret = IsCapKeyMatchDeviceId(key, deviceId);
    EXPECT_EQ(false, ret);

    key = "key_test";
    ret = IsCapKeyMatchDeviceId(key, deviceId);
    EXPECT_EQ(false, ret);
}
}
}