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

#include "gtest/gtest.h"

#include "constants.h"
#include "dh_context.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {
const std::string TEST_NETWORKID = "111111";
const std::string TEST_UUID = "222222";
const std::string TEST_UDID = "333333";
constexpr int32_t OLD_HO_DEVICE_TYPE_TEST = -1;
constexpr int32_t NEW_HO_DEVICE_TYPE_TEST = 11;
constexpr int32_t INVALID_OSTYPE = 0;
}

class DhContextTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DhContextTest::SetUp() {}

void DhContextTest::TearDown() {}

void DhContextTest::SetUpTestCase() {}

void DhContextTest::TearDownTestCase() {}

HWTEST_F(DhContextTest, AddOnlineDevice_001, TestSize.Level1)
{
    DHContext::GetInstance().AddOnlineDevice("", "", "");
    EXPECT_EQ(true, DHContext::GetInstance().devIdEntrySet_.empty());

    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, "", "");
    EXPECT_EQ(true, DHContext::GetInstance().devIdEntrySet_.empty());

    DHContext::GetInstance().AddOnlineDevice("", TEST_UUID, "");
    EXPECT_EQ(true, DHContext::GetInstance().devIdEntrySet_.empty());

    DHContext::GetInstance().AddOnlineDevice("", "", TEST_NETWORKID);
    EXPECT_EQ(true, DHContext::GetInstance().devIdEntrySet_.empty());

    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, TEST_UUID, "");
    EXPECT_EQ(true, DHContext::GetInstance().devIdEntrySet_.empty());

    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, "", TEST_NETWORKID);
    EXPECT_EQ(true, DHContext::GetInstance().devIdEntrySet_.empty());

    DHContext::GetInstance().AddOnlineDevice("", TEST_UUID, TEST_NETWORKID);
    EXPECT_EQ(true, DHContext::GetInstance().devIdEntrySet_.empty());
}

HWTEST_F(DhContextTest, AddOnlineDevice_002, TestSize.Level1)
{
    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, TEST_UUID, TEST_NETWORKID);
    EXPECT_EQ(false, DHContext::GetInstance().devIdEntrySet_.empty());
}

HWTEST_F(DhContextTest, RemoveOnlineDeviceIdEntryByNetworkId_001, TestSize.Level1)
{
    std::string networkId = "";
    DHContext::GetInstance().RemoveOnlineDeviceIdEntryByNetworkId(networkId);
    EXPECT_EQ(false, DHContext::GetInstance().devIdEntrySet_.empty());

    DHContext::GetInstance().RemoveOnlineDeviceIdEntryByNetworkId("123");
    EXPECT_EQ(false, DHContext::GetInstance().devIdEntrySet_.empty());

    DHContext::GetInstance().RemoveOnlineDeviceIdEntryByNetworkId(TEST_NETWORKID);
    EXPECT_EQ(true, DHContext::GetInstance().devIdEntrySet_.empty());
}

HWTEST_F(DhContextTest, IsDeviceOnline_001, TestSize.Level1)
{
    std::string uuid = "";
    bool ret = DHContext::GetInstance().IsDeviceOnline(uuid);
    EXPECT_EQ(false, ret);

    ret = DHContext::GetInstance().IsDeviceOnline(TEST_UUID);
    EXPECT_EQ(false, ret);

    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, TEST_UUID, TEST_NETWORKID);
    ret = DHContext::GetInstance().IsDeviceOnline(TEST_UUID);
    EXPECT_EQ(true, ret);
}

HWTEST_F(DhContextTest, GetNetworkIdByUUID_001, TestSize.Level1)
{
    std::string uuid = "";
    auto ret = DHContext::GetInstance().GetNetworkIdByUUID(uuid);
    EXPECT_EQ("", ret);

    ret = DHContext::GetInstance().GetNetworkIdByUUID(TEST_UUID);
    EXPECT_EQ(TEST_NETWORKID, ret);

    DHContext::GetInstance().devIdEntrySet_.clear();
    ret = DHContext::GetInstance().GetNetworkIdByUUID(TEST_UUID);
    EXPECT_EQ("", ret);
}

HWTEST_F(DhContextTest, GetNetworkIdByUDID_001, TestSize.Level1)
{
    std::string udid = "";
    auto ret = DHContext::GetInstance().GetNetworkIdByUDID(udid);
    EXPECT_EQ("", ret);

    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, TEST_UUID, TEST_NETWORKID);
    ret = DHContext::GetInstance().GetNetworkIdByUDID(TEST_UDID);
    EXPECT_EQ(TEST_NETWORKID, ret);

    DHContext::GetInstance().devIdEntrySet_.clear();
    ret = DHContext::GetInstance().GetNetworkIdByUDID(udid);
    EXPECT_EQ("", ret);
}

HWTEST_F(DhContextTest, GetUdidHashIdByUUID_001, TestSize.Level1)
{
    auto ret = DHContext::GetInstance().GetUdidHashIdByUUID(TEST_UUID);
    EXPECT_EQ("", ret);

    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, TEST_UUID, TEST_NETWORKID);
    ret = DHContext::GetInstance().GetUdidHashIdByUUID(TEST_UUID);
    EXPECT_NE("", ret);
}

HWTEST_F(DhContextTest, GetUUIDByNetworkId_001, TestSize.Level1)
{
    std::string networkId = "";
    auto ret = DHContext::GetInstance().GetUUIDByNetworkId(networkId);
    EXPECT_EQ("", ret);

    ret = DHContext::GetInstance().GetUUIDByNetworkId(TEST_NETWORKID);
    EXPECT_EQ(TEST_UUID, ret);

    DHContext::GetInstance().devIdEntrySet_.clear();
    ret = DHContext::GetInstance().GetUUIDByNetworkId(TEST_NETWORKID);
    EXPECT_EQ("", ret);
}

HWTEST_F(DhContextTest, GetUDIDByNetworkId_001, TestSize.Level1)
{
    std::string networkId = "";
    auto ret = DHContext::GetInstance().GetUDIDByNetworkId(networkId);
    EXPECT_EQ("", ret);

    ret = DHContext::GetInstance().GetUDIDByNetworkId(TEST_NETWORKID);
    EXPECT_EQ("", ret);

    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, TEST_UUID, TEST_NETWORKID);
    ret = DHContext::GetInstance().GetUDIDByNetworkId(TEST_NETWORKID);
    EXPECT_EQ(TEST_UDID, ret);
}

HWTEST_F(DhContextTest, GetUUIDByDeviceId_001, TestSize.Level1)
{
    std::string deviceId = "";
    auto ret = DHContext::GetInstance().GetUUIDByDeviceId(deviceId);
    EXPECT_EQ("", ret);

    deviceId = "123456";
    ret = DHContext::GetInstance().GetUUIDByDeviceId(deviceId);
    EXPECT_EQ("", ret);
}

HWTEST_F(DhContextTest, GetUUIDByDeviceId_002, TestSize.Level1)
{
    std::string deviceId = Sha256(TEST_UUID);
    auto ret = DHContext::GetInstance().GetUUIDByDeviceId(deviceId);
    EXPECT_EQ(TEST_UUID, ret);

    deviceId = Sha256(TEST_UDID);
    ret = DHContext::GetInstance().GetUUIDByDeviceId(deviceId);
    EXPECT_EQ(TEST_UUID, ret);

    DHContext::GetInstance().devIdEntrySet_.clear();
    ret = DHContext::GetInstance().GetUUIDByDeviceId(deviceId);
    EXPECT_EQ("", ret);
}

HWTEST_F(DhContextTest, GetNetworkIdByDeviceId_001, TestSize.Level1)
{
    std::string deviceId = "";
    auto ret = DHContext::GetInstance().GetNetworkIdByDeviceId(deviceId);
    EXPECT_EQ("", ret);

    deviceId = Sha256(TEST_UUID);
    ret = DHContext::GetInstance().GetNetworkIdByDeviceId(deviceId);
    EXPECT_EQ("", ret);

    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, TEST_UUID, TEST_NETWORKID);
    ret = DHContext::GetInstance().GetNetworkIdByDeviceId(deviceId);
    EXPECT_EQ(TEST_NETWORKID, ret);
}

HWTEST_F(DhContextTest, GetDeviceIdByDBGetPrefix_001, TestSize.Level1)
{
    std::string prefix = "";
    auto ret = DHContext::GetInstance().GetDeviceIdByDBGetPrefix(prefix);
    EXPECT_EQ("", ret);

    prefix = "prefix_test";
    ret = DHContext::GetInstance().GetDeviceIdByDBGetPrefix(prefix);
    EXPECT_EQ(prefix, ret);

    prefix = "prefix" + RESOURCE_SEPARATOR;
    ret = DHContext::GetInstance().GetDeviceIdByDBGetPrefix(prefix);
    EXPECT_EQ("prefix", ret);
}

HWTEST_F(DhContextTest, AddIsomerismConnectDev_001, TestSize.Level1)
{
    std::string IsomerismDeviceId = "";
    DHContext::GetInstance().AddIsomerismConnectDev(IsomerismDeviceId);
    EXPECT_TRUE(DHContext::GetInstance().connectedDevIds_.empty());

    IsomerismDeviceId = "IsomerismDeviceId_test";
    DHContext::GetInstance().AddIsomerismConnectDev(IsomerismDeviceId);
    EXPECT_FALSE(DHContext::GetInstance().connectedDevIds_.empty());
}

HWTEST_F(DhContextTest, DelIsomerismConnectDev_001, TestSize.Level1)
{
    std::string IsomerismDeviceId = "";
    DHContext::GetInstance().DelIsomerismConnectDev(IsomerismDeviceId);
    EXPECT_FALSE(DHContext::GetInstance().connectedDevIds_.empty());

    IsomerismDeviceId = "123456789";
    DHContext::GetInstance().DelIsomerismConnectDev(IsomerismDeviceId);
    EXPECT_FALSE(DHContext::GetInstance().connectedDevIds_.empty());

    IsomerismDeviceId = "IsomerismDeviceId_test";
    DHContext::GetInstance().DelIsomerismConnectDev(IsomerismDeviceId);
    EXPECT_TRUE(DHContext::GetInstance().connectedDevIds_.empty());
}

HWTEST_F(DhContextTest, OnMessage_001, TestSize.Level1)
{
    sptr<IPublisherListener> dhFwkIsomerismListener(new DHContext::DHFWKIsomerismListener());
    ASSERT_TRUE(dhFwkIsomerismListener != nullptr);
    DHTopic topic = DHTopic::TOPIC_LOW_LATENCY;
    std::string message = "";
    dhFwkIsomerismListener->OnMessage(topic, message);
    EXPECT_TRUE(DHContext::GetInstance().connectedDevIds_.empty());

    message = "message_test";
    dhFwkIsomerismListener->OnMessage(topic, message);
    EXPECT_TRUE(DHContext::GetInstance().connectedDevIds_.empty());

    topic = DHTopic::TOPIC_ISOMERISM;
    dhFwkIsomerismListener->OnMessage(topic, message);
    EXPECT_TRUE(DHContext::GetInstance().connectedDevIds_.empty());
}

HWTEST_F(DhContextTest, OnMessage_002, TestSize.Level1)
{
    sptr<IPublisherListener> dhFwkIsomerismListener(new DHContext::DHFWKIsomerismListener());
    ASSERT_TRUE(dhFwkIsomerismListener != nullptr);
    DHTopic topic = DHTopic::TOPIC_ISOMERISM;
    const std::string eventKey = "Isomerism_event";
    cJSON *json = cJSON_CreateObject();
    ASSERT_TRUE(json != nullptr);
    cJSON_AddStringToObject(json, eventKey.c_str(), "Isomerism_event_test");
    char* cjson = cJSON_PrintUnformatted(json);
    if (cjson == nullptr) {
        cJSON_Delete(json);
        return;
    }
    std::string message(cjson);
    dhFwkIsomerismListener->OnMessage(topic, message);
    cJSON_free(cjson);
    cJSON_Delete(json);
    EXPECT_TRUE(DHContext::GetInstance().connectedDevIds_.empty());
}

HWTEST_F(DhContextTest, OnMessage_003, TestSize.Level1)
{
    sptr<IPublisherListener> dhFwkIsomerismListener(new DHContext::DHFWKIsomerismListener());
    ASSERT_TRUE(dhFwkIsomerismListener != nullptr);
    DHTopic topic = DHTopic::TOPIC_ISOMERISM;
    const std::string eventKey = "isomerism_event";
    cJSON *json = cJSON_CreateObject();
    ASSERT_TRUE(json != nullptr);
    cJSON_AddNumberToObject(json, eventKey.c_str(), 1);
    char* cjson = cJSON_PrintUnformatted(json);
    if (cjson == nullptr) {
        cJSON_Delete(json);
        return;
    }
    std::string message(cjson);
    dhFwkIsomerismListener->OnMessage(topic, message);
    cJSON_free(cjson);
    cJSON_Delete(json);
    EXPECT_TRUE(DHContext::GetInstance().connectedDevIds_.empty());
}

HWTEST_F(DhContextTest, OnMessage_004, TestSize.Level1)
{
    sptr<IPublisherListener> dhFwkIsomerismListener(new DHContext::DHFWKIsomerismListener());
    ASSERT_TRUE(dhFwkIsomerismListener != nullptr);
    DHTopic topic = DHTopic::TOPIC_ISOMERISM;
    const std::string eventKey = "isomerism_event";
    const std::string devId = "Dev_Id";
    cJSON *json = cJSON_CreateObject();
    ASSERT_TRUE(json != nullptr);
    cJSON_AddStringToObject(json, eventKey.c_str(), "isomerism_event_test");
    cJSON_AddStringToObject(json, DEV_ID, "Dev_Id_test");

    char* cjson = cJSON_PrintUnformatted(json);
    if (cjson == nullptr) {
        cJSON_Delete(json);
        return;
    }
    std::string message(cjson);
    dhFwkIsomerismListener->OnMessage(topic, message);
    cJSON_free(cjson);
    cJSON_Delete(json);
    EXPECT_TRUE(DHContext::GetInstance().connectedDevIds_.empty());
}

HWTEST_F(DhContextTest, OnMessage_005, TestSize.Level1)
{
    sptr<IPublisherListener> dhFwkIsomerismListener(new DHContext::DHFWKIsomerismListener());
    ASSERT_TRUE(dhFwkIsomerismListener != nullptr);
    DHTopic topic = DHTopic::TOPIC_ISOMERISM;
    const std::string eventKey = "isomerism_event";
    const std::string devId = "Dev_Id";
    cJSON *json = cJSON_CreateObject();
    ASSERT_TRUE(json != nullptr);
    cJSON_AddStringToObject(json, eventKey.c_str(), "isomerism_event_test");
    cJSON_AddNumberToObject(json, DEV_ID, 1);

    char* cjson = cJSON_PrintUnformatted(json);
    if (cjson == nullptr) {
        cJSON_Delete(json);
        return;
    }
    std::string message(cjson);
    dhFwkIsomerismListener->OnMessage(topic, message);
    cJSON_free(cjson);
    cJSON_Delete(json);
    EXPECT_TRUE(DHContext::GetInstance().connectedDevIds_.empty());
}

HWTEST_F(DhContextTest, OnMessage_006, TestSize.Level1)
{
    sptr<IPublisherListener> dhFwkIsomerismListener(new DHContext::DHFWKIsomerismListener());
    ASSERT_TRUE(dhFwkIsomerismListener != nullptr);
    DHTopic topic = DHTopic::TOPIC_ISOMERISM;
    const std::string eventKey = "isomerism_event";
    const std::string devId = "dev_id";
    cJSON *json = cJSON_CreateObject();
    ASSERT_TRUE(json != nullptr);
    cJSON_AddStringToObject(json, eventKey.c_str(), "isomerism_event_test");
    cJSON_AddStringToObject(json, DEV_ID, "dev_id_test");

    char* cjson = cJSON_PrintUnformatted(json);
    if (cjson == nullptr) {
        cJSON_Delete(json);
        return;
    }
    std::string message(cjson);
    dhFwkIsomerismListener->OnMessage(topic, message);
    cJSON_free(cjson);
    cJSON_Delete(json);
    EXPECT_TRUE(DHContext::GetInstance().connectedDevIds_.empty());
}

HWTEST_F(DhContextTest, OnMessage_007, TestSize.Level1)
{
    sptr<IPublisherListener> dhFwkIsomerismListener(new DHContext::DHFWKIsomerismListener());
    ASSERT_TRUE(dhFwkIsomerismListener != nullptr);
    DHTopic topic = DHTopic::TOPIC_ISOMERISM;
    const std::string eventKey = "isomerism_event";
    const std::string devId = "dev_id";
    cJSON *json = cJSON_CreateObject();
    ASSERT_TRUE(json != nullptr);
    cJSON_AddStringToObject(json, eventKey.c_str(), "isomerism_connect");
    cJSON_AddStringToObject(json, DEV_ID, "dev_id_test");

    char* cjson = cJSON_PrintUnformatted(json);
    if (cjson == nullptr) {
        cJSON_Delete(json);
        return;
    }
    std::string message(cjson);
    dhFwkIsomerismListener->OnMessage(topic, message);
    cJSON_free(cjson);
    cJSON_Delete(json);
    EXPECT_FALSE(DHContext::GetInstance().connectedDevIds_.empty());
}

HWTEST_F(DhContextTest, OnMessage_008, TestSize.Level1)
{
    sptr<IPublisherListener> dhFwkIsomerismListener(new DHContext::DHFWKIsomerismListener());
    ASSERT_TRUE(dhFwkIsomerismListener != nullptr);
    DHTopic topic = DHTopic::TOPIC_ISOMERISM;
    const std::string eventKey = "isomerism_event";
    const std::string devId = "dev_id";
    cJSON *json = cJSON_CreateObject();
    ASSERT_TRUE(json != nullptr);
    cJSON_AddStringToObject(json, eventKey.c_str(), "isomerism_disconnect");
    cJSON_AddStringToObject(json, DEV_ID, "dev_id_test");

    char* cjson = cJSON_PrintUnformatted(json);
    if (cjson == nullptr) {
        cJSON_Delete(json);
        return;
    }
    std::string message(cjson);
    dhFwkIsomerismListener->OnMessage(topic, message);
    cJSON_free(cjson);
    cJSON_Delete(json);
    EXPECT_TRUE(DHContext::GetInstance().connectedDevIds_.empty());
}

HWTEST_F(DhContextTest, GetDeviceIdByNetworkId_001, TestSize.Level1)
{
    std::string networkId = "";
    auto ret = DHContext::GetInstance().GetDeviceIdByNetworkId(networkId);
    EXPECT_EQ("", ret);

    networkId = "networkId_1";
    DHContext::GetInstance().AddOnlineDevice(TEST_UDID, TEST_UUID, TEST_NETWORKID);
    ret = DHContext::GetInstance().GetDeviceIdByNetworkId(networkId);
    EXPECT_EQ("", ret);

    ret = DHContext::GetInstance().GetDeviceIdByNetworkId(TEST_NETWORKID);
    EXPECT_EQ(Sha256(TEST_UUID), ret);
}

HWTEST_F(DhContextTest, CheckAndDeleteOnlineDeviceOSType_001, TestSize.Level1)
{
    std::string networkId = "networkId_123";
    auto ret = DHContext::GetInstance().IsDoubleFwkDevice(networkId);
    DHContext::GetInstance().DeleteOnlineDeviceOSType(networkId);
    EXPECT_EQ(false, ret);

    DHContext::GetInstance().AddOnlineDeviceOSType(networkId, INVALID_OSTYPE);
    ret = DHContext::GetInstance().IsDoubleFwkDevice(networkId);
    DHContext::GetInstance().DeleteOnlineDeviceOSType(networkId);
    EXPECT_EQ(false, ret);

    DHContext::GetInstance().AddOnlineDeviceOSType(networkId, OLD_HO_DEVICE_TYPE_TEST);
    ret = DHContext::GetInstance().IsDoubleFwkDevice(networkId);
    DHContext::GetInstance().DeleteOnlineDeviceOSType(networkId);
    EXPECT_EQ(true, ret);

    DHContext::GetInstance().AddOnlineDeviceOSType(networkId, NEW_HO_DEVICE_TYPE_TEST);
    ret = DHContext::GetInstance().IsDoubleFwkDevice(networkId);
    DHContext::GetInstance().DeleteOnlineDeviceOSType(networkId);
    EXPECT_EQ(true, ret);
}

HWTEST_F(DhContextTest, IsRealTimeOnlineDevice_001, TestSize.Level1)
{
    std::string networkId = "networkId_1";
    DHContext::GetInstance().AddRealTimeOnlineDeviceNetworkId(networkId);
    auto ret = DHContext::GetInstance().IsRealTimeOnlineDevice(networkId);
    EXPECT_EQ(true, ret);

    DHContext::GetInstance().DeleteRealTimeOnlineDeviceNetworkId(networkId);
    ret = DHContext::GetInstance().IsRealTimeOnlineDevice(networkId);
    EXPECT_EQ(false, ret);
}
}
}