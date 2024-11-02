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
#include "distributed_hardware_errno.h"
#include "version_info.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
class VersionInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void VersionInfoTest::SetUp() {}

void VersionInfoTest::TearDown() {}

void VersionInfoTest::SetUpTestCase() {}

void VersionInfoTest::TearDownTestCase() {}

HWTEST_F(VersionInfoTest, FromJsonString_001, TestSize.Level0)
{
    std::shared_ptr<VersionInfo> versionInfoPtr = std::make_shared<VersionInfo>();
    std::string jsonStr = "";
    auto ret = versionInfoPtr->FromJsonString(jsonStr);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(VersionInfoTest, ToJson_001, TestSize.Level0)
{
    cJSON *jsonObj = nullptr;
    VersionInfo verInfo;
    ToJson(jsonObj, verInfo);
    EXPECT_TRUE(verInfo.deviceId.empty());
}

HWTEST_F(VersionInfoTest, FromJson_CompVersion_001, TestSize.Level0)
{
    cJSON *jsonObj = nullptr;
    CompVersion compVer;
    FromJson(jsonObj, compVer);
    EXPECT_TRUE(compVer.name.empty());
}

HWTEST_F(VersionInfoTest, FromJson_CompVersion_002, TestSize.Level0)
{
    const uint32_t dhType = 1;
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, NAME.c_str(), "name_test");
    cJSON_AddNumberToObject(jsonObj, TYPE.c_str(), dhType);
    cJSON_AddStringToObject(jsonObj, HANDLER.c_str(), "handler_ver_1.0");
    cJSON_AddStringToObject(jsonObj, SOURCE_VER.c_str(), "source_ver_1.0");
    cJSON_AddStringToObject(jsonObj, SINK_VER.c_str(), "sink_ver_1.0");
    CompVersion compVer;
    FromJson(jsonObj, compVer);
    cJSON_Delete(jsonObj);
    EXPECT_FALSE(compVer.name.empty());
}

HWTEST_F(VersionInfoTest, FromJson_CompVersion_003, TestSize.Level0)
{
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddNumberToObject(jsonObj, NAME.c_str(), 1);
    cJSON_AddStringToObject(jsonObj, TYPE.c_str(), "dhType_audio");
    cJSON_AddNumberToObject(jsonObj, HANDLER.c_str(), 1);
    cJSON_AddNumberToObject(jsonObj, SOURCE_VER.c_str(), 1);
    cJSON_AddNumberToObject(jsonObj, SINK_VER.c_str(), 1);
    CompVersion compVer;
    FromJson(jsonObj, compVer);
    cJSON_Delete(jsonObj);
    EXPECT_TRUE(compVer.name.empty());
}

HWTEST_F(VersionInfoTest, FromJson_VersionInfo_001, TestSize.Level0)
{
    cJSON *jsonObj = nullptr;
    VersionInfo verInfo;
    FromJson(jsonObj, verInfo);
    EXPECT_TRUE(verInfo.deviceId.empty());
}

HWTEST_F(VersionInfoTest, FromJson_VersionInfo_002, TestSize.Level0)
{
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, DEV_ID.c_str(), "devid_123");
    cJSON_AddStringToObject(jsonObj, DH_VER.c_str(), "dhver_1.0");

    cJSON * jsonArr = cJSON_CreateArray();
    if (jsonArr == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    cJSON_AddItemToArray(jsonArr, cJSON_CreateString("com_ver_test"));
    cJSON_AddItemToObject(jsonObj, COMP_VER.c_str(), jsonArr);
    VersionInfo verInfo;
    FromJson(jsonObj, verInfo);
    cJSON_Delete(jsonObj);
    EXPECT_FALSE(verInfo.deviceId.empty());
}

HWTEST_F(VersionInfoTest, FromJson_VersionInfo_003, TestSize.Level0)
{
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddNumberToObject(jsonObj, DEV_ID.c_str(), 1);
    cJSON_AddNumberToObject(jsonObj, DH_VER.c_str(), 1);
    VersionInfo verInfo;
    FromJson(jsonObj, verInfo);
    cJSON_Delete(jsonObj);
    EXPECT_TRUE(verInfo.deviceId.empty());
}
}
}