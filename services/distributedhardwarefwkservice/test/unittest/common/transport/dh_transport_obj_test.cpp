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

#include <gtest/gtest.h>

#include <string>

#include "dh_transport_obj.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_log.h"


using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace DistributedHardware {
using namespace std;
constexpr uint16_t TEST_DEV_TYPE = 0x14;

class DhTransportObjTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DhTransportObjTest::SetUpTestCase()
{
}

void DhTransportObjTest::TearDownTestCase()
{
}

void DhTransportObjTest::SetUp()
{
}

void DhTransportObjTest::TearDown()
{
}

HWTEST_F(DhTransportObjTest, ToJson_FullCapsRsp_001, TestSize.Level1)
{
    cJSON *json = nullptr;
    bool isSyncMeta = false;
    FullCapsRsp capsRsp;
    ASSERT_NO_FATAL_FAILURE(ToJson(json, capsRsp, isSyncMeta));


    cJSON *jsonObject = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject != nullptr);
    std::vector<std::shared_ptr<MetaCapabilityInfo>> metaCapInfos;
    std::vector<std::shared_ptr<CapabilityInfo>> capInfos;
    std::shared_ptr<CapabilityInfo> capInfo = std::make_shared<CapabilityInfo>(
        "dhId_test", "deviceId_test", "devName_test", TEST_DEV_TYPE, DHType::CAMERA, "attrs_test", "subtype");
    capInfos.push_back(capInfo);
    FullCapsRsp capsRsp1("networkId_123456", capInfos, metaCapInfos);
    ASSERT_NO_FATAL_FAILURE(ToJson(jsonObject, capsRsp1, isSyncMeta));
    cJSON_Delete(jsonObject);
}

HWTEST_F(DhTransportObjTest, ToJson_FullCapsRsp_002, TestSize.Level1)
{
    bool isSyncMeta = true;
    FullCapsRsp capsRsp;
    cJSON *jsonObject = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject != nullptr);
    std::vector<std::shared_ptr<MetaCapabilityInfo>> metaCapInfos;
    std::vector<std::shared_ptr<CapabilityInfo>> capInfos;
    CompVersion compVersion;
    compVersion.sinkVersion = "1.0";
    std::string udidHash = "udidHash_123";
    std::shared_ptr<MetaCapabilityInfo> metaCapInfo = std::make_shared<MetaCapabilityInfo>("dhId_1", "deviceId_1",
        "devName_camera", TEST_DEV_TYPE, DHType::CAMERA, "attrs_1", "subtype", udidHash, compVersion);
    metaCapInfos.push_back(metaCapInfo);
    FullCapsRsp capsRsp1("networkId_123456", capInfos, metaCapInfos);
    ASSERT_NO_FATAL_FAILURE(ToJson(jsonObject, capsRsp1, isSyncMeta));
    cJSON_Delete(jsonObject);
}

HWTEST_F(DhTransportObjTest, FromJson_FullCapsRsp_001, TestSize.Level1)
{
    cJSON *json = nullptr;
    bool isSyncMeta = false;
    FullCapsRsp capsRsp;
    ASSERT_NO_FATAL_FAILURE(FromJson(json, capsRsp, isSyncMeta));

    cJSON *jsonObject = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject != nullptr);
    cJSON_AddNumberToObject(jsonObject, CAPS_RSP_NETWORKID_KEY, 1);
    ASSERT_NO_FATAL_FAILURE(FromJson(jsonObject, capsRsp, isSyncMeta));
    cJSON_Delete(jsonObject);

    cJSON *jsonObject1 = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject1 != nullptr);
    cJSON_AddStringToObject(jsonObject1, CAPS_RSP_NETWORKID_KEY, "networkId_123456");
    cJSON_AddStringToObject(jsonObject1, CAPS_RSP_CAPS_KEY, "rsp_caps_ket_test");
    ASSERT_NO_FATAL_FAILURE(FromJson(jsonObject1, capsRsp, isSyncMeta));
    cJSON_Delete(jsonObject1);
}

HWTEST_F(DhTransportObjTest, FromJson_FullCapsRsp_002, TestSize.Level1)
{
    bool isSyncMeta = false;
    FullCapsRsp capsRsp;
    cJSON *jsonObject = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject != nullptr);
    cJSON_AddStringToObject(jsonObject, CAPS_RSP_NETWORKID_KEY, "networkId_123456");

    cJSON *jsonArr = cJSON_CreateArray();
    if (jsonArr == nullptr) {
        cJSON_Delete(jsonObject);
        return;
    }
    cJSON_AddItemToObject(jsonObject, CAPS_RSP_CAPS_KEY, jsonArr);
    ASSERT_NO_FATAL_FAILURE(FromJson(jsonObject, capsRsp, isSyncMeta));
    cJSON_Delete(jsonObject);
}
}
}