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

HWTEST_F(VersionInfoTest, FromJsonString_001, TestSize.Level1)
{
    std::shared_ptr<VersionInfo> versionInfoPtr = std::make_shared<VersionInfo>();
    std::string jsonStr = "";
    auto ret = versionInfoPtr->FromJsonString(jsonStr);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(VersionInfoTest, ToJson_001, TestSize.Level1)
{
    cJSON *jsonObj = nullptr;
    VersionInfo verInfo;
    ToJson(jsonObj, verInfo);
    EXPECT_TRUE(verInfo.deviceId.empty());
}

HWTEST_F(VersionInfoTest, ToJson_002, TestSize.Level1)
{
    CompVersion compVerTo;
    CompVersion compVerFrom;
    compVerFrom.name = "name_test";
    compVerFrom.dhType = DHType::AUDIO;
    compVerFrom.handlerVersion = "handler_ver_1.0";
    compVerFrom.sourceVersion = "source_ver_1.0";
    compVerFrom.sinkVersion = "sink_ver_1.0";
    compVerFrom.haveFeature = true;
    compVerFrom.sourceFeatureFilters = { "feature0", "feature1"};
    compVerFrom.sinkSupportedFeatures = { "feature0", "feature1"};
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    ToJson(jsonObj, compVerFrom);
    FromJson(jsonObj, compVerTo);
    cJSON_Delete(jsonObj);
    EXPECT_EQ(compVerTo.name, compVerFrom.name);
    EXPECT_EQ(compVerTo.dhType, compVerFrom.dhType);
    EXPECT_EQ(compVerTo.haveFeature, compVerFrom.haveFeature);
    EXPECT_EQ(compVerTo.sourceFeatureFilters, compVerFrom.sourceFeatureFilters);
    EXPECT_EQ(compVerTo.sinkSupportedFeatures, compVerFrom.sinkSupportedFeatures);
}

HWTEST_F(VersionInfoTest, FromJson_CompVersion_001, TestSize.Level1)
{
    cJSON *jsonObj = nullptr;
    CompVersion compVer;
    FromJson(jsonObj, compVer);
    EXPECT_TRUE(compVer.name.empty());
}

HWTEST_F(VersionInfoTest, FromJson_CompVersion_002, TestSize.Level1)
{
    const uint32_t dhType = 1;
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, NAME, "name_test");
    cJSON_AddNumberToObject(jsonObj, TYPE, dhType);
    cJSON_AddStringToObject(jsonObj, HANDLER, "handler_ver_1.0");
    cJSON_AddStringToObject(jsonObj, SOURCE_VER, "source_ver_1.0");
    cJSON_AddStringToObject(jsonObj, SINK_VER, "sink_ver_1.0");
    CompVersion compVer;
    FromJson(jsonObj, compVer);
    cJSON_Delete(jsonObj);
    EXPECT_FALSE(compVer.name.empty());
}

HWTEST_F(VersionInfoTest, FromJson_CompVersion_003, TestSize.Level1)
{
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddNumberToObject(jsonObj, NAME, 1);
    cJSON_AddStringToObject(jsonObj, TYPE, "dhType_audio");
    cJSON_AddNumberToObject(jsonObj, HANDLER, 1);
    cJSON_AddNumberToObject(jsonObj, SOURCE_VER, 1);
    cJSON_AddNumberToObject(jsonObj, SINK_VER, 1);
    CompVersion compVer;
    FromJson(jsonObj, compVer);
    cJSON_Delete(jsonObj);
    EXPECT_TRUE(compVer.name.empty());
}

HWTEST_F(VersionInfoTest, FromJson_VersionInfo_001, TestSize.Level1)
{
    cJSON *jsonObj = nullptr;
    VersionInfo verInfo;
    FromJson(jsonObj, verInfo);
    EXPECT_TRUE(verInfo.deviceId.empty());
}

HWTEST_F(VersionInfoTest, FromJson_VersionInfo_002, TestSize.Level1)
{
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, DEV_ID, "devid_123");
    cJSON_AddStringToObject(jsonObj, DH_VER, "dhver_1.0");

    cJSON * jsonArr = cJSON_CreateArray();
    if (jsonArr == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    cJSON_AddItemToArray(jsonArr, cJSON_CreateString("com_ver_test"));
    cJSON_AddItemToObject(jsonObj, COMP_VER, jsonArr);
    VersionInfo verInfo;
    FromJson(jsonObj, verInfo);
    cJSON_Delete(jsonObj);
    EXPECT_FALSE(verInfo.deviceId.empty());
}

HWTEST_F(VersionInfoTest, FromJson_VersionInfo_003, TestSize.Level1)
{
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddNumberToObject(jsonObj, DEV_ID, 1);
    cJSON_AddNumberToObject(jsonObj, DH_VER, 1);
    VersionInfo verInfo;
    FromJson(jsonObj, verInfo);
    cJSON_Delete(jsonObj);
    EXPECT_TRUE(verInfo.deviceId.empty());
}

HWTEST_F(VersionInfoTest, FromJsoncompVerContinue_001, TestSize.Level1)
{
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddItemToObject(jsonObj, "test", cJSON_CreateArray());
    CompVersion compVer;
    FromJsoncompVerContinue(jsonObj, compVer);
    EXPECT_EQ(false, compVer.haveFeature);
    cJSON_Delete(jsonObj);
}

HWTEST_F(VersionInfoTest, FromJsoncompVerContinue_002, TestSize.Level1)
{
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON *srcJsonArr = cJSON_CreateArray();
    if (srcJsonArr == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    cJSON_AddItemToArray(srcJsonArr, cJSON_CreateString("src_feature"));
    cJSON_AddItemToArray(srcJsonArr, cJSON_CreateNumber(1));
    cJSON_AddItemToObject(jsonObj, SOURCE_FEATURE_FILTER, srcJsonArr);
    cJSON_AddStringToObject(jsonObj, SINK_SUPPORTED_FEATURE, "sink_filter");
    CompVersion compVer;
    FromJsoncompVerContinue(jsonObj, compVer);
    EXPECT_EQ(true, compVer.haveFeature);
    cJSON_Delete(jsonObj);
}

HWTEST_F(VersionInfoTest, FromJsoncompVerContinue_003, TestSize.Level1)
{
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON *sinkJsonArr = cJSON_CreateArray();
    if (sinkJsonArr == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    cJSON_AddItemToArray(sinkJsonArr, cJSON_CreateString("sink_feature"));
    cJSON_AddItemToArray(sinkJsonArr, cJSON_CreateNumber(1));
    cJSON_AddItemToObject(jsonObj, SOURCE_FEATURE_FILTER, sinkJsonArr);
    cJSON_AddStringToObject(jsonObj, SINK_SUPPORTED_FEATURE, "src_filter");
    CompVersion compVer;
    FromJsoncompVerContinue(jsonObj, compVer);
    EXPECT_EQ(true, compVer.haveFeature);
    cJSON_Delete(jsonObj);
}

HWTEST_F(VersionInfoTest, FromJsoncompVerContinue_004, TestSize.Level1)
{
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON *srcJsonArr = cJSON_CreateArray();
    if (srcJsonArr == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    cJSON_AddItemToArray(srcJsonArr, cJSON_CreateString("src_feature"));
    cJSON_AddItemToArray(srcJsonArr, cJSON_CreateNumber(1));
    cJSON_AddItemToObject(jsonObj, SOURCE_FEATURE_FILTER, srcJsonArr);

    cJSON *sinkJsonArr = cJSON_CreateArray();
    if (sinkJsonArr == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    cJSON_AddItemToArray(sinkJsonArr, cJSON_CreateString("sink_feature"));
    cJSON_AddItemToArray(sinkJsonArr, cJSON_CreateNumber(1));
    cJSON_AddItemToObject(jsonObj, SINK_SUPPORTED_FEATURE, sinkJsonArr);
    CompVersion compVer;
    FromJsoncompVerContinue(jsonObj, compVer);
    EXPECT_EQ(true, compVer.haveFeature);
    cJSON_Delete(jsonObj);
}

HWTEST_F(VersionInfoTest, FromJsoncompVerContinue_005, TestSize.Level1)
{
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON *srcJsonArr = cJSON_CreateArray();
    if (srcJsonArr == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    cJSON_AddItemToObject(jsonObj, SOURCE_FEATURE_FILTER, srcJsonArr);

    cJSON *sinkJsonArr = cJSON_CreateArray();
    if (sinkJsonArr == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    cJSON_AddItemToObject(jsonObj, SINK_SUPPORTED_FEATURE, sinkJsonArr);
    CompVersion compVer;
    FromJsoncompVerContinue(jsonObj, compVer);
    EXPECT_EQ(true, compVer.haveFeature);
    cJSON_Delete(jsonObj);
}
}
}