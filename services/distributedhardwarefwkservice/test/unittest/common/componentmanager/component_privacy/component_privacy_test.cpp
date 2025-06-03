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
#include <string>

#include "ability_manager_errors.h"
#include "component_privacy.h"
#include "component_loader.h"
#include "distributed_hardware_errno.h"
#include "idistributed_hardware_sink.h"

using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
using namespace std;
namespace {
    const std::string SUBTYPE_MIC = "mic";
    const std::string SUBTYPE_CAMERA = "camera";
    const std::string NETWORK_ID = "123456789";
    constexpr const char *PRIVACY_SUBTYPE = "subtype";
    constexpr const char *PRIVACY_NETWORKID = "networkId";
    constexpr uint32_t COMP_START_PAGE = 1;
    constexpr uint32_t COMP_STOP_PAGE = 2;
    constexpr uint16_t DEVICE_TYPE_WIFI_CAMERA = 0x08;
    constexpr uint16_t DEVICE_TYPE_AUDIO = 0x0A;
    constexpr uint16_t DEVICE_TYPE_PC = 0x0C;
    constexpr uint16_t DEVICE_TYPE_PHONE = 0x0E;
    constexpr uint16_t DEVICE_TYPE_PAD = 0x11;
    constexpr uint16_t DEVICE_TYPE_WATCH = 0x6D;
    constexpr uint16_t DEVICE_TYPE_CAR = 0x83;
    constexpr uint16_t DEVICE_TYPE_TV = 0x9C;
    constexpr uint16_t DEVICE_TYPE_SMART_DISPLAY = 0xA02;
    constexpr uint16_t DEVICE_TYPE_2IN1 = 0xA2F;
    constexpr uint16_t DEVICE_TYPE_UNKNOWN = 0x00;
}
class ComponentPrivacyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<ComponentPrivacy> compPrivacy_ = nullptr;
};

void ComponentPrivacyTest::SetUpTestCase()
{
}

void ComponentPrivacyTest::TearDownTestCase()
{
}

void ComponentPrivacyTest::SetUp()
{
    compPrivacy_ = std::make_shared<ComponentPrivacy>();
}

void ComponentPrivacyTest::TearDown()
{
    compPrivacy_ = nullptr;
}

HWTEST_F(ComponentPrivacyTest, OnPrivaceResourceMessage_001, TestSize.Level1)
{
    ASSERT_TRUE(compPrivacy_ != nullptr);
    ResourceEventType type = ResourceEventType::EVENT_TYPE_QUERY_RESOURCE;
    bool isSensitive = true;
    bool isSameAccout = true;
    int32_t ret = compPrivacy_->OnPrivaceResourceMessage(type, SUBTYPE_MIC, NETWORK_ID, isSensitive, isSameAccout);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_KEY_IS_EMPTY, ret);
}

HWTEST_F(ComponentPrivacyTest, OnPrivaceResourceMessage_002, TestSize.Level1)
{
    ASSERT_TRUE(compPrivacy_ != nullptr);
    ResourceEventType type = ResourceEventType::EVENT_TYPE_QUERY_RESOURCE;
    bool isSensitive = true;
    bool isSameAccout = true;
    int32_t ret = compPrivacy_->OnPrivaceResourceMessage(type, SUBTYPE_MIC, "", isSensitive, isSameAccout);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(ComponentPrivacyTest, OnPrivaceResourceMessage_003, TestSize.Level1)
{
    ASSERT_TRUE(compPrivacy_ != nullptr);
    ResourceEventType type = ResourceEventType::EVENT_TYPE_PULL_UP_PAGE;
    bool isSensitive = true;
    bool isSameAccout = true;
    int32_t ret = compPrivacy_->OnPrivaceResourceMessage(type, SUBTYPE_MIC, "", isSensitive, isSameAccout);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(ComponentPrivacyTest, OnPrivaceResourceMessage_004, TestSize.Level1)
{
    ASSERT_TRUE(compPrivacy_ != nullptr);
    ResourceEventType type = ResourceEventType::EVENT_TYPE_PULL_UP_PAGE;
    bool isSensitive = true;
    bool isSameAccout = true;
    compPrivacy_->eventHandler_ = nullptr;
    int32_t ret = compPrivacy_->OnPrivaceResourceMessage(type, SUBTYPE_MIC, NETWORK_ID, isSensitive, isSameAccout);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(ComponentPrivacyTest, OnPrivaceResourceMessage_005, TestSize.Level1)
{
    ASSERT_TRUE(compPrivacy_ != nullptr);
    ResourceEventType type = ResourceEventType::EVENT_TYPE_CLOSE_PAGE;
    bool isSensitive = true;
    bool isSameAccout = true;
    int32_t ret = compPrivacy_->OnPrivaceResourceMessage(type, SUBTYPE_MIC, NETWORK_ID, isSensitive, isSameAccout);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(ComponentPrivacyTest, OnPrivaceResourceMessage_006, TestSize.Level1)
{
    ASSERT_TRUE(compPrivacy_ != nullptr);
    ResourceEventType type = ResourceEventType::EVENT_TYPE_CLOSE_PAGE;
    bool isSensitive = true;
    bool isSameAccout = true;
    compPrivacy_->eventHandler_ = nullptr;
    int32_t ret = compPrivacy_->OnPrivaceResourceMessage(type, SUBTYPE_MIC, NETWORK_ID, isSensitive, isSameAccout);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(ComponentPrivacyTest, HandlePullUpPage_002, TestSize.Level1)
{
    ASSERT_TRUE(compPrivacy_ != nullptr);
    compPrivacy_->eventHandler_ = nullptr;
    EXPECT_NO_FATAL_FAILURE(compPrivacy_->HandlePullUpPage(SUBTYPE_MIC, NETWORK_ID));
}

HWTEST_F(ComponentPrivacyTest, HandleClosePage_001, TestSize.Level1)
{
    ASSERT_TRUE(compPrivacy_ != nullptr);
    compPrivacy_->eventHandler_ = nullptr;
    EXPECT_NO_FATAL_FAILURE(compPrivacy_->HandleClosePage(SUBTYPE_MIC));
}

HWTEST_F(ComponentPrivacyTest, StartPrivacePage_001, TestSize.Level1)
{
    ASSERT_TRUE(compPrivacy_ != nullptr);
    auto ret = compPrivacy_->StartPrivacePage(SUBTYPE_MIC, "");
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(ComponentPrivacyTest, ProcessEvent_001, TestSize.Level1)
{
    ASSERT_TRUE(compPrivacy_ != nullptr);
    cJSON *jsonArrayMsg = cJSON_CreateArray();
    ASSERT_TRUE(jsonArrayMsg != nullptr);

    cJSON *tmpJson = cJSON_CreateObject();
    if (tmpJson == NULL) {
        cJSON_Delete(jsonArrayMsg);
        return;
    }
    cJSON_AddNumberToObject(tmpJson, PRIVACY_SUBTYPE, 1);
    cJSON_AddStringToObject(tmpJson, PRIVACY_NETWORKID, NETWORK_ID.c_str());
    cJSON_AddItemToArray(jsonArrayMsg, tmpJson);
    AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(COMP_START_PAGE,
        std::shared_ptr<cJSON>(jsonArrayMsg, cJSON_Delete), 0);
    EXPECT_NO_FATAL_FAILURE(compPrivacy_->eventHandler_->ProcessEvent(msgEvent));
}

HWTEST_F(ComponentPrivacyTest, ProcessEvent_002, TestSize.Level1)
{
    ASSERT_TRUE(compPrivacy_ != nullptr);
    cJSON *jsonArrayMsg = cJSON_CreateArray();
    ASSERT_TRUE(jsonArrayMsg != nullptr);

    cJSON *tmpJson = cJSON_CreateObject();
    if (tmpJson == NULL) {
        cJSON_Delete(jsonArrayMsg);
        return;
    }
    cJSON_AddStringToObject(tmpJson, PRIVACY_SUBTYPE, SUBTYPE_MIC.c_str());
    cJSON_AddNumberToObject(tmpJson, PRIVACY_NETWORKID, 1);
    cJSON_AddItemToArray(jsonArrayMsg, tmpJson);
    AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(COMP_START_PAGE,
        std::shared_ptr<cJSON>(jsonArrayMsg, cJSON_Delete), 0);
    EXPECT_NO_FATAL_FAILURE(compPrivacy_->eventHandler_->ProcessEvent(msgEvent));
}

HWTEST_F(ComponentPrivacyTest, ProcessEvent_003, TestSize.Level1)
{
    ASSERT_TRUE(compPrivacy_ != nullptr);
    cJSON *jsonArrayMsg = cJSON_CreateArray();
    ASSERT_TRUE(jsonArrayMsg != nullptr);

    cJSON *tmpJson = cJSON_CreateObject();
    if (tmpJson == NULL) {
        cJSON_Delete(jsonArrayMsg);
        return;
    }
    cJSON_AddStringToObject(tmpJson, PRIVACY_SUBTYPE, SUBTYPE_MIC.c_str());
    cJSON_AddStringToObject(tmpJson, PRIVACY_NETWORKID, NETWORK_ID.c_str());
    cJSON_AddItemToArray(jsonArrayMsg, tmpJson);
    AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(COMP_START_PAGE,
        std::shared_ptr<cJSON>(jsonArrayMsg, cJSON_Delete), 0);
    EXPECT_NO_FATAL_FAILURE(compPrivacy_->eventHandler_->ProcessEvent(msgEvent));
}

HWTEST_F(ComponentPrivacyTest, ProcessEvent_004, TestSize.Level1)
{
    ASSERT_TRUE(compPrivacy_ != nullptr);
    cJSON *jsonArrayMsg = cJSON_CreateArray();
    ASSERT_TRUE(jsonArrayMsg != nullptr);

    cJSON *tmpJson = cJSON_CreateObject();
    if (tmpJson == NULL) {
        cJSON_Delete(jsonArrayMsg);
        return;
    }
    cJSON_AddStringToObject(tmpJson, PRIVACY_SUBTYPE, SUBTYPE_CAMERA.c_str());
    cJSON_AddStringToObject(tmpJson, PRIVACY_NETWORKID, NETWORK_ID.c_str());
    cJSON_AddItemToArray(jsonArrayMsg, tmpJson);
    AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(COMP_START_PAGE,
        std::shared_ptr<cJSON>(jsonArrayMsg, cJSON_Delete), 0);
    EXPECT_NO_FATAL_FAILURE(compPrivacy_->eventHandler_->ProcessEvent(msgEvent));
}

HWTEST_F(ComponentPrivacyTest, ProcessEvent_005, TestSize.Level1)
{
    ASSERT_TRUE(compPrivacy_ != nullptr);
    cJSON *jsonArrayMsg = cJSON_CreateArray();
    ASSERT_TRUE(jsonArrayMsg != nullptr);

    cJSON *tmpJson = cJSON_CreateObject();
    if (tmpJson == NULL) {
        cJSON_Delete(jsonArrayMsg);
        return;
    }
    cJSON_AddNumberToObject(tmpJson, PRIVACY_SUBTYPE, 1);
    cJSON_AddItemToArray(jsonArrayMsg, tmpJson);
    AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(COMP_STOP_PAGE,
        std::shared_ptr<cJSON>(jsonArrayMsg, cJSON_Delete), 0);
    EXPECT_NO_FATAL_FAILURE(compPrivacy_->eventHandler_->ProcessEvent(msgEvent));
}

HWTEST_F(ComponentPrivacyTest, ProcessEvent_006, TestSize.Level1)
{
    ASSERT_TRUE(compPrivacy_ != nullptr);
    cJSON *jsonArrayMsg = cJSON_CreateArray();
    ASSERT_TRUE(jsonArrayMsg != nullptr);

    cJSON *tmpJson = cJSON_CreateObject();
    if (tmpJson == NULL) {
        cJSON_Delete(jsonArrayMsg);
        return;
    }
    cJSON_AddStringToObject(tmpJson, PRIVACY_SUBTYPE, SUBTYPE_CAMERA.c_str());
    cJSON_AddItemToArray(jsonArrayMsg, tmpJson);
    AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(COMP_STOP_PAGE,
        std::shared_ptr<cJSON>(jsonArrayMsg, cJSON_Delete), 0);
    EXPECT_NO_FATAL_FAILURE(compPrivacy_->eventHandler_->ProcessEvent(msgEvent));
}

HWTEST_F(ComponentPrivacyTest, ProcessEvent_007, TestSize.Level1)
{
    ASSERT_TRUE(compPrivacy_ != nullptr);
    cJSON *jsonArrayMsg = cJSON_CreateArray();
    ASSERT_TRUE(jsonArrayMsg != nullptr);

    cJSON *tmpJson = cJSON_CreateObject();
    if (tmpJson == NULL) {
        cJSON_Delete(jsonArrayMsg);
        return;
    }
    cJSON_AddStringToObject(tmpJson, PRIVACY_SUBTYPE, SUBTYPE_MIC.c_str());
    cJSON_AddItemToArray(jsonArrayMsg, tmpJson);
    AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(COMP_STOP_PAGE,
        std::shared_ptr<cJSON>(jsonArrayMsg, cJSON_Delete), 0);
    EXPECT_NO_FATAL_FAILURE(compPrivacy_->eventHandler_->ProcessEvent(msgEvent));
}

HWTEST_F(ComponentPrivacyTest, DeviceTypeToString_001, TestSize.Level1)
{
    ASSERT_TRUE(compPrivacy_ != nullptr);
    std::string ret = compPrivacy_->DeviceTypeToString(DEVICE_TYPE_WIFI_CAMERA);
    EXPECT_EQ("camera", ret);

    ret = compPrivacy_->DeviceTypeToString(DEVICE_TYPE_AUDIO);
    EXPECT_EQ("audio", ret);

    ret = compPrivacy_->DeviceTypeToString(DEVICE_TYPE_PC);
    EXPECT_EQ("pc", ret);

    ret = compPrivacy_->DeviceTypeToString(DEVICE_TYPE_PHONE);
    EXPECT_EQ("phone", ret);

    ret = compPrivacy_->DeviceTypeToString(DEVICE_TYPE_PAD);
    EXPECT_EQ("pad", ret);

    ret = compPrivacy_->DeviceTypeToString(DEVICE_TYPE_WATCH);
    EXPECT_EQ("watch", ret);

    ret = compPrivacy_->DeviceTypeToString(DEVICE_TYPE_CAR);
    EXPECT_EQ("car", ret);

    ret = compPrivacy_->DeviceTypeToString(DEVICE_TYPE_TV);
    EXPECT_EQ("tv", ret);

    ret = compPrivacy_->DeviceTypeToString(DEVICE_TYPE_SMART_DISPLAY);
    EXPECT_EQ("display", ret);

    ret = compPrivacy_->DeviceTypeToString(DEVICE_TYPE_2IN1);
    EXPECT_EQ("2in1", ret);

    ret = compPrivacy_->DeviceTypeToString(DEVICE_TYPE_UNKNOWN);
    EXPECT_EQ("unknown", ret);
}
}
}