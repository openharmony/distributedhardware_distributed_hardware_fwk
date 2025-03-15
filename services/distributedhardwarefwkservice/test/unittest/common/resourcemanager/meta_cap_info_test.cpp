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
#include "distributed_hardware_errno.h"
#include "meta_capability_info.h"
#include "meta_info_manager.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr uint16_t DEV_TYPE_TEST = 14;
    constexpr uint32_t DH_TYPE_TEST = 0x01;
    const std::string DHID_TEST = "123456789";
    const std::string DEVID_TEST = "111222333";
    const std::string DEVNAME_TEST = "camera";
    const std::string DHATTRS_TEST = "attrs";
    const std::string DHSUBTYPE_TEST = "subtype";
    const std::string UDIDHASH_TEST = "987654321";
    const std::string SINKVER_TEST = "1.0";

}

class MetaCapInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void MetaCapInfoTest::SetUp() {}

void MetaCapInfoTest::TearDown() {}

void MetaCapInfoTest::SetUpTestCase() {}

void MetaCapInfoTest::TearDownTestCase() {}

HWTEST_F(MetaCapInfoTest, FromJsonString_001, TestSize.Level1)
{
    uint16_t devType = 1;
    std::shared_ptr<MetaCapabilityInfo> metaCapInfo =
        std::make_shared<MetaCapabilityInfo>("", "", "", devType, DHType::CAMERA, "", "", "",
            CompVersion{ .sinkVersion = "" });
    std::string jsonStr = "";
    auto ret = metaCapInfo->FromJsonString(jsonStr);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(MetaCapInfoTest, FromJsonString_002, TestSize.Level1)
{
    uint16_t devType = 1;
    std::shared_ptr<MetaCapabilityInfo> metaCapInfo =
        std::make_shared<MetaCapabilityInfo>("", "", "", devType, DHType::CAMERA, "", "", "",
            CompVersion{ .sinkVersion = "" });
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, DH_ID.c_str(), DHID_TEST.c_str());
    char* cjson = cJSON_PrintUnformatted(jsonObj);
    if (cjson == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    std::string jsonStr(cjson);
    auto ret = metaCapInfo->FromJsonString(jsonStr);
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(MetaCapInfoTest, ToJson_001, TestSize.Level1)
{
    cJSON *jsonObject = nullptr;
    MetaCapabilityInfo metaCapInfo;
    EXPECT_NO_FATAL_FAILURE(ToJson(jsonObject, metaCapInfo));
}

HWTEST_F(MetaCapInfoTest, FromJson_001, TestSize.Level1)
{
    MetaCapabilityInfo metaCapInfo;
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddNumberToObject(jsonObj, DH_ID.c_str(), 1);
    EXPECT_NO_FATAL_FAILURE(FromJson(jsonObj, metaCapInfo));
    cJSON_Delete(jsonObj);
}

HWTEST_F(MetaCapInfoTest, FromJson_002, TestSize.Level1)
{
    MetaCapabilityInfo metaCapInfo;
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, DH_ID.c_str(), DHID_TEST.c_str());
    cJSON_AddNumberToObject(jsonObj, DEV_ID.c_str(), 1);
    EXPECT_NO_FATAL_FAILURE(FromJson(jsonObj, metaCapInfo));
    cJSON_Delete(jsonObj);
}

HWTEST_F(MetaCapInfoTest, FromJson_003, TestSize.Level1)
{
    MetaCapabilityInfo metaCapInfo;
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, DH_ID.c_str(), DHID_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DEV_ID.c_str(), DEVID_TEST.c_str());
    cJSON_AddNumberToObject(jsonObj, DEV_NAME.c_str(), 1);
    EXPECT_NO_FATAL_FAILURE(FromJson(jsonObj, metaCapInfo));
    cJSON_Delete(jsonObj);
}

HWTEST_F(MetaCapInfoTest, FromJson_004, TestSize.Level1)
{
    MetaCapabilityInfo metaCapInfo;
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    std::string devTypeTest = "devTypeTest";
    cJSON_AddStringToObject(jsonObj, DH_ID.c_str(), DHID_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DEV_ID.c_str(), DEVID_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DEV_NAME.c_str(), DEVNAME_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DEV_TYPE.c_str(), devTypeTest.c_str());
    EXPECT_NO_FATAL_FAILURE(FromJson(jsonObj, metaCapInfo));
    cJSON_Delete(jsonObj);
}

HWTEST_F(MetaCapInfoTest, FromJson_005, TestSize.Level1)
{
    MetaCapabilityInfo metaCapInfo;
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    std::string dhTypeTest = "dhTypeTest";
    cJSON_AddStringToObject(jsonObj, DH_ID.c_str(), DHID_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DEV_ID.c_str(), DEVID_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DEV_NAME.c_str(), DEVNAME_TEST.c_str());
    cJSON_AddNumberToObject(jsonObj, DEV_TYPE.c_str(), DEV_TYPE_TEST);
    cJSON_AddStringToObject(jsonObj, DH_TYPE.c_str(), dhTypeTest.c_str());
    EXPECT_NO_FATAL_FAILURE(FromJson(jsonObj, metaCapInfo));
    cJSON_Delete(jsonObj);
}

HWTEST_F(MetaCapInfoTest, FromJson_006, TestSize.Level1)
{
    MetaCapabilityInfo metaCapInfo;
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    std::string dhTypeTest = "dhTypeTest";
    cJSON_AddStringToObject(jsonObj, DH_ID.c_str(), DHID_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DEV_ID.c_str(), DEVID_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DEV_NAME.c_str(), DEVNAME_TEST.c_str());
    cJSON_AddNumberToObject(jsonObj, DEV_TYPE.c_str(), DEV_TYPE_TEST);
    cJSON_AddNumberToObject(jsonObj, DH_TYPE.c_str(), DH_TYPE_TEST);
    cJSON_AddNumberToObject(jsonObj, DH_ATTRS.c_str(), 1);
    EXPECT_NO_FATAL_FAILURE(FromJson(jsonObj, metaCapInfo));
    cJSON_Delete(jsonObj);
}

HWTEST_F(MetaCapInfoTest, FromJson_007, TestSize.Level1)
{
    MetaCapabilityInfo metaCapInfo;
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    std::string dhTypeTest = "dhTypeTest";
    cJSON_AddStringToObject(jsonObj, DH_ID.c_str(), DHID_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DEV_ID.c_str(), DEVID_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DEV_NAME.c_str(), DEVNAME_TEST.c_str());
    cJSON_AddNumberToObject(jsonObj, DEV_TYPE.c_str(), DEV_TYPE_TEST);
    cJSON_AddNumberToObject(jsonObj, DH_TYPE.c_str(), DH_TYPE_TEST);
    cJSON_AddStringToObject(jsonObj, DH_ATTRS.c_str(), DHATTRS_TEST.c_str());
    cJSON_AddNumberToObject(jsonObj, DH_SUBTYPE.c_str(), 1);
    EXPECT_NO_FATAL_FAILURE(FromJson(jsonObj, metaCapInfo));
    cJSON_Delete(jsonObj);
}

HWTEST_F(MetaCapInfoTest, FromJson_008, TestSize.Level1)
{
    MetaCapabilityInfo metaCapInfo;
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    std::string dhTypeTest = "dhTypeTest";
    cJSON_AddStringToObject(jsonObj, DH_ID.c_str(), DHID_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DEV_ID.c_str(), DEVID_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DEV_NAME.c_str(), DEVNAME_TEST.c_str());
    cJSON_AddNumberToObject(jsonObj, DEV_TYPE.c_str(), DEV_TYPE_TEST);
    cJSON_AddNumberToObject(jsonObj, DH_TYPE.c_str(), DH_TYPE_TEST);
    cJSON_AddStringToObject(jsonObj, DH_ATTRS.c_str(), DHATTRS_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DH_SUBTYPE.c_str(), DHSUBTYPE_TEST.c_str());
    cJSON_AddNumberToObject(jsonObj, DEV_UDID_HASH.c_str(), 1);
    EXPECT_NO_FATAL_FAILURE(FromJson(jsonObj, metaCapInfo));
    cJSON_Delete(jsonObj);
}

HWTEST_F(MetaCapInfoTest, FromJson_009, TestSize.Level1)
{
    MetaCapabilityInfo metaCapInfo;
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    std::string dhTypeTest = "dhTypeTest";
    cJSON_AddStringToObject(jsonObj, DH_ID.c_str(), DHID_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DEV_ID.c_str(), DEVID_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DEV_NAME.c_str(), DEVNAME_TEST.c_str());
    cJSON_AddNumberToObject(jsonObj, DEV_TYPE.c_str(), DEV_TYPE_TEST);
    cJSON_AddNumberToObject(jsonObj, DH_TYPE.c_str(), DH_TYPE_TEST);
    cJSON_AddStringToObject(jsonObj, DH_ATTRS.c_str(), DHATTRS_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DH_SUBTYPE.c_str(), DHSUBTYPE_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DEV_UDID_HASH.c_str(), UDIDHASH_TEST.c_str());
    cJSON_AddNumberToObject(jsonObj, SINK_VER.c_str(), 1);
    EXPECT_NO_FATAL_FAILURE(FromJson(jsonObj, metaCapInfo));
    cJSON_Delete(jsonObj);
}

HWTEST_F(MetaCapInfoTest, FromJson_010, TestSize.Level1)
{
    MetaCapabilityInfo metaCapInfo;
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, DH_ID.c_str(), DHID_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DEV_ID.c_str(), DEVID_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DEV_NAME.c_str(), DEVNAME_TEST.c_str());
    cJSON_AddNumberToObject(jsonObj, DEV_TYPE.c_str(), DEV_TYPE_TEST);
    cJSON_AddNumberToObject(jsonObj, DH_TYPE.c_str(), DH_TYPE_TEST);
    cJSON_AddStringToObject(jsonObj, DH_ATTRS.c_str(), DHATTRS_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DH_SUBTYPE.c_str(), DHSUBTYPE_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, DEV_UDID_HASH.c_str(), UDIDHASH_TEST.c_str());
    cJSON_AddStringToObject(jsonObj, SINK_VER.c_str(), SINKVER_TEST.c_str());
    EXPECT_NO_FATAL_FAILURE(FromJson(jsonObj, metaCapInfo));
    cJSON_Delete(jsonObj);
}

HWTEST_F(MetaCapInfoTest, Compare_001, TestSize.Level1)
{
    uint16_t devType = 1;
    MetaCapabilityInfo metaCapInfo("", "", "", devType, DHType::CAMERA, "", "", "", CompVersion{ .sinkVersion = "" });
    std::shared_ptr<MetaCapabilityInfo> metaCapInfoPtr =
        std::make_shared<MetaCapabilityInfo>("devid_123", "", "", devType, DHType::CAMERA, "", "", "",
            CompVersion{ .sinkVersion = "" });
    auto ret = metaCapInfoPtr->Compare(metaCapInfo);
    EXPECT_EQ(false, ret);
}

HWTEST_F(MetaCapInfoTest, Compare_002, TestSize.Level1)
{
    uint16_t devType = 1;
    MetaCapabilityInfo metaCapInfo("devid_123", "", "", devType, DHType::CAMERA, "", "", "",
        CompVersion{ .sinkVersion = "" });
    std::shared_ptr<MetaCapabilityInfo> metaCapInfoPtr =
        std::make_shared<MetaCapabilityInfo>("devid_123", "dhid_123", "", devType, DHType::CAMERA, "", "", "",
            CompVersion{ .sinkVersion = "" });
    auto ret = metaCapInfoPtr->Compare(metaCapInfo);
    EXPECT_EQ(false, ret);
}

HWTEST_F(MetaCapInfoTest, Compare_003, TestSize.Level1)
{
    uint16_t devType = 1;
    MetaCapabilityInfo metaCapInfo("devid_123", "dhid_123", "", devType, DHType::CAMERA, "", "", "",
        CompVersion{ .sinkVersion = "" });
    std::shared_ptr<MetaCapabilityInfo> metaCapInfoPtr = std::make_shared<MetaCapabilityInfo>(
        "devid_123", "dhid_123", "devname", devType, DHType::CAMERA, "", "", "", CompVersion{ .sinkVersion = "" });
    auto ret = metaCapInfoPtr->Compare(metaCapInfo);
    EXPECT_EQ(false, ret);
}

HWTEST_F(MetaCapInfoTest, Compare_004, TestSize.Level1)
{
    uint16_t devType = 1;
    MetaCapabilityInfo metaCapInfo("devid_123", "dhid_123", "devname", devType, DHType::CAMERA, "", "", "",
        CompVersion{ .sinkVersion = "" });
    std::shared_ptr<MetaCapabilityInfo> metaCapInfoPtr =
        std::make_shared<MetaCapabilityInfo>("devid_123", "dhid_123", "devname", 2, DHType::CAMERA, "", "", "",
            CompVersion{ .sinkVersion = "" });
    auto ret = metaCapInfoPtr->Compare(metaCapInfo);
    EXPECT_EQ(false, ret);
}

HWTEST_F(MetaCapInfoTest, Compare_005, TestSize.Level1)
{
    uint16_t devType = 1;
    MetaCapabilityInfo metaCapInfo("devid_123", "dhid_123", "devname", devType, DHType::CAMERA, "", "", "",
        CompVersion{ .sinkVersion = "" });
    std::shared_ptr<MetaCapabilityInfo> metaCapInfoPtr = std::make_shared<MetaCapabilityInfo>(
        "devid_123", "dhid_123", "devname", devType, DHType::AUDIO, "", "", "", CompVersion{ .sinkVersion = "" });
    auto ret = metaCapInfoPtr->Compare(metaCapInfo);
    EXPECT_EQ(false, ret);
}

HWTEST_F(MetaCapInfoTest, Compare_006, TestSize.Level1)
{
    uint16_t devType = 1;
    MetaCapabilityInfo metaCapInfo("devid_123", "dhid_123", "devname", devType, DHType::CAMERA, "", "", "",
        CompVersion{ .sinkVersion = "" });
    std::shared_ptr<MetaCapabilityInfo> metaCapInfoPtr = std::make_shared<MetaCapabilityInfo>(
        "devid_123", "dhid_123", "devname", devType, DHType::CAMERA, "attrs", "", "",
        CompVersion{ .sinkVersion = "" });
    auto ret = metaCapInfoPtr->Compare(metaCapInfo);
    EXPECT_EQ(false, ret);
}

HWTEST_F(MetaCapInfoTest, Compare_007, TestSize.Level1)
{
    uint16_t devType = 1;
    MetaCapabilityInfo metaCapInfo("devid_123", "dhid_123", "devname", devType, DHType::CAMERA, "attrs", "", "",
        CompVersion{ .sinkVersion = "" });
    std::shared_ptr<MetaCapabilityInfo> metaCapInfoPtr = std::make_shared<MetaCapabilityInfo>(
        "devid_123", "dhid_123", "devname", devType, DHType::CAMERA, "attrs", "subtype", "",
        CompVersion{ .sinkVersion = "" });
    auto ret = metaCapInfoPtr->Compare(metaCapInfo);
    EXPECT_EQ(false, ret);
}

HWTEST_F(MetaCapInfoTest, Compare_008, TestSize.Level1)
{
    uint16_t devType = 1;
    MetaCapabilityInfo metaCapInfo("devid_123", "dhid_123", "devname",
        devType, DHType::CAMERA, "attrs", "subtype", "", CompVersion{ .sinkVersion = "" });
    std::shared_ptr<MetaCapabilityInfo> metaCapInfoPtr = std::make_shared<MetaCapabilityInfo>(
        "devid_123", "dhid_123", "devname", devType, DHType::CAMERA, "attrs", "subtype", "udidhash_123",
        CompVersion{ .sinkVersion = "" });
    auto ret = metaCapInfoPtr->Compare(metaCapInfo);
    EXPECT_EQ(false, ret);
}

HWTEST_F(MetaCapInfoTest, Compare_009, TestSize.Level1)
{
    uint16_t devType = 1;
    MetaCapabilityInfo metaCapInfo("devid_123", "dhid_123", "devname",
        devType, DHType::CAMERA, "attrs", "subtype", "udidhash_123", CompVersion{ .sinkVersion = "" });
    std::shared_ptr<MetaCapabilityInfo> metaCapInfoPtr = std::make_shared<MetaCapabilityInfo>(
        "devid_123", "dhid_123", "devname", devType, DHType::CAMERA, "attrs", "subtype", "udidhash_123",
        CompVersion{ .sinkVersion = "1.0" });
    auto ret = metaCapInfoPtr->Compare(metaCapInfo);
    EXPECT_EQ(false, ret);
}

HWTEST_F(MetaCapInfoTest, Compare_010, TestSize.Level1)
{
    uint16_t devType = 1;
    MetaCapabilityInfo metaCapInfo("devid_123", "dhid_123", "devname",
        devType, DHType::CAMERA, "attrs", "subtype", "udidhash_123", CompVersion{ .sinkVersion = "1.0" });
    std::shared_ptr<MetaCapabilityInfo> metaCapInfoPtr = std::make_shared<MetaCapabilityInfo>(
        "devid_123", "dhid_123", "devname", devType, DHType::CAMERA, "attrs", "subtype", "udidhash_123",
        CompVersion{ .sinkVersion = "1.0" });
    auto ret = metaCapInfoPtr->Compare(metaCapInfo);
    EXPECT_EQ(true, ret);
}
}
}