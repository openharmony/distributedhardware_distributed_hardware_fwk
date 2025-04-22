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

#include "dh_transport.h"
#include <string>

#include "dh_comm_tool.h"
#include "dh_transport_obj.h"
#include "capability_info_manager.h"
#include "distributed_hardware_errno.h"


using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
using namespace std;

namespace {
    int32_t g_socketid = 1;
    std::string g_networkid = "networkId_test";
    constexpr int32_t SOCKETID = 10;
}
class DhTransportTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
private:
    std::shared_ptr<DHTransport> dhTransportTest_;
};

void DhTransportTest::SetUpTestCase()
{
}

void DhTransportTest::TearDownTestCase()
{
}

void DhTransportTest::SetUp()
{
    std::shared_ptr<DHCommTool> dhCommTool = std::make_shared<DHCommTool>();
    dhTransportTest_ = std::make_shared<DHTransport>(dhCommTool);
}

void DhTransportTest::TearDown()
{
}

HWTEST_F(DhTransportTest, OnSocketClosed_001, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    ShutdownReason reason = ShutdownReason::SHUTDOWN_REASON_UNKNOWN;
    dhTransportTest_->remoteDevSocketIds_[g_networkid] = g_socketid;
    dhTransportTest_->OnSocketClosed(2, reason);

    dhTransportTest_->OnSocketClosed(g_socketid, reason);
    EXPECT_EQ(0, dhTransportTest_->remoteDevSocketIds_.size());
}

HWTEST_F(DhTransportTest, OnBytesReceived_001, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    int32_t sessionId = -1;
    char *data = nullptr;
    uint32_t dataLen = 0;
    dhTransportTest_->OnBytesReceived(sessionId, data, dataLen);

    dhTransportTest_->OnBytesReceived(g_socketid, data, dataLen);

    char dataMsg[10] = "dataMsg";
    dhTransportTest_->OnBytesReceived(sessionId, dataMsg, dataLen);

    dhTransportTest_->OnBytesReceived(g_socketid, dataMsg, dataLen);

    dataLen = 1;
    dhTransportTest_->OnBytesReceived(g_socketid, data, dataLen);

    dhTransportTest_->OnBytesReceived(sessionId, data, dataLen);
    dhTransportTest_->OnBytesReceived(sessionId, dataMsg, dataLen);
    dataLen = 5 * 1024 * 1024;
    dhTransportTest_->OnBytesReceived(sessionId, data, dataLen);
    dhTransportTest_->OnBytesReceived(sessionId, dataMsg, dataLen);
    dhTransportTest_->OnBytesReceived(g_socketid, dataMsg, dataLen);

    dataLen = 1;
    dhTransportTest_->OnBytesReceived(g_socketid, dataMsg, dataLen);
    dhTransportTest_->remoteDevSocketIds_[g_networkid] = g_socketid;
    dhTransportTest_->OnBytesReceived(g_socketid, dataMsg, dataLen);
    EXPECT_EQ(1, dhTransportTest_->remoteDevSocketIds_.size());
}

HWTEST_F(DhTransportTest, Init_001, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    dhTransportTest_->isSocketSvrCreateFlag_ = true;
    auto ret = dhTransportTest_->Init();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    dhTransportTest_->isSocketSvrCreateFlag_ = false;
    dhTransportTest_->Init();
    EXPECT_NE(ERR_DH_FWK_COMPONENT_TRANSPORT_OPT_FAILED, ret);
}

HWTEST_F(DhTransportTest, UnInit_001, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    auto ret = dhTransportTest_->UnInit();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    dhTransportTest_->isSocketSvrCreateFlag_ = true;
    dhTransportTest_->UnInit();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(DhTransportTest, IsDeviceSessionOpened_001, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    dhTransportTest_->remoteDevSocketIds_.clear();
    auto ret = dhTransportTest_->IsDeviceSessionOpened(g_networkid, g_socketid);
    EXPECT_EQ(false, ret);

    dhTransportTest_->remoteDevSocketIds_[g_networkid] = g_socketid;
    ret = dhTransportTest_->IsDeviceSessionOpened(g_networkid, g_socketid);
    EXPECT_EQ(true, ret);
}

HWTEST_F(DhTransportTest, IsDeviceSessionOpened_002, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    std::string remoteNetworkId = "";
    auto ret = dhTransportTest_->IsDeviceSessionOpened(remoteNetworkId, g_socketid);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DhTransportTest, StartSocket_001, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    dhTransportTest_->remoteDevSocketIds_[g_networkid] = g_socketid;
    auto ret = dhTransportTest_->StartSocket(g_networkid);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    std::string remoteNetworkId = "remoteNetworkId_test";
    ret = dhTransportTest_->StartSocket(remoteNetworkId);
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_TRANSPORT_OPT_FAILED, ret);
}

HWTEST_F(DhTransportTest, StartSocket_002, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    std::string remoteNetworkId = "";
    auto ret = dhTransportTest_->StartSocket(remoteNetworkId);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(DhTransportTest, StopSocket_001, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    dhTransportTest_->remoteDevSocketIds_.clear();
    auto ret = dhTransportTest_->StopSocket(g_networkid);
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_TRANSPORT_OPT_FAILED, ret);

    dhTransportTest_->remoteDevSocketIds_[g_networkid] = g_socketid;
    ret = dhTransportTest_->StopSocket(g_networkid);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(DhTransportTest, StopSocket_002, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    std::string remoteNetworkId = "";
    auto ret = dhTransportTest_->StopSocket(remoteNetworkId);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(DhTransportTest, Send_001, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    std::string payload = "payload_test";
    dhTransportTest_->remoteDevSocketIds_.clear();
    auto ret = dhTransportTest_->Send(g_networkid, payload);
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_TRANSPORT_OPT_FAILED, ret);
}

HWTEST_F(DhTransportTest, Send_002, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    std::string payload = "payload_test";
    dhTransportTest_->remoteDevSocketIds_.clear();
    dhTransportTest_->remoteDevSocketIds_[g_networkid] = g_socketid;
    auto ret = dhTransportTest_->Send(g_networkid, payload);
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_TRANSPORT_OPT_FAILED, ret);
}

HWTEST_F(DhTransportTest, GetRemoteNetworkIdBySocketId_001, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    std::string networkid = "123456";
    dhTransportTest_->remoteDevSocketIds_[networkid] = SOCKETID;
    auto ret = dhTransportTest_->GetRemoteNetworkIdBySocketId(SOCKETID);
    EXPECT_EQ(networkid, ret);

    dhTransportTest_->remoteDevSocketIds_.clear();
    ret = dhTransportTest_->GetRemoteNetworkIdBySocketId(SOCKETID);
    EXPECT_EQ("", ret);

    std::string remoteDevId = "";
    dhTransportTest_->ClearDeviceSocketOpened(remoteDevId);

    std::string payload = "";
    dhTransportTest_->HandleReceiveMessage(payload);

    payload = "payload_test";
    cJSON *jsonObj = cJSON_CreateObject();
    std::string networkIdKey = "networkId";
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, networkIdKey.c_str(), "111111");
    char* cjson = cJSON_PrintUnformatted(jsonObj);
    if (cjson == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    std::string jsonStr(cjson);
    ASSERT_NO_FATAL_FAILURE(dhTransportTest_->HandleReceiveMessage(jsonStr));
}

HWTEST_F(DhTransportTest, ToJson_FullCapsRsp_001, TestSize.Level1)
{
    cJSON *json = nullptr;
    FullCapsRsp capsRsp;
    ToJson(json, capsRsp);

    cJSON *jsonObject = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject != nullptr);
    std::vector<std::shared_ptr<CapabilityInfo>> resInfos;
    std::shared_ptr<CapabilityInfo> capInfo = std::make_shared<CapabilityInfo>(
        "dhId_test", "deviceId_test", "devName_test", 14, DHType::CAMERA, "attrs_test", "subtype");
    resInfos.emplace_back(capInfo);
    capsRsp.networkId = "123456";
    capsRsp.caps = resInfos;
    ToJson(jsonObject, capsRsp);
    cJSON_Delete(jsonObject);
    EXPECT_FALSE(capsRsp.networkId.empty());
}

HWTEST_F(DhTransportTest, FromJson_FullCapsRsp_001, TestSize.Level1)
{
    cJSON *json = nullptr;
    FullCapsRsp capsRsp;
    FromJson(json, capsRsp);

    cJSON *jsonObject = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject != nullptr);
    cJSON_AddStringToObject(jsonObject, CAPS_RSP_NETWORKID_KEY, "caps_rsp_networkIid_test");
    cJSON_AddStringToObject(jsonObject, CAPS_RSP_CAPS_KEY, "caps_rsp_caps_test");
    FromJson(jsonObject, capsRsp);
    cJSON_Delete(jsonObject);
    EXPECT_FALSE(capsRsp.networkId.empty());
}

HWTEST_F(DhTransportTest, FromJson_FullCapsRsp_002, TestSize.Level1)
{
    cJSON *jsonObject = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject != nullptr);
    cJSON_AddNumberToObject(jsonObject, CAPS_RSP_NETWORKID_KEY, 1);

    cJSON *jsonArr = cJSON_CreateArray();
    if (jsonArr == nullptr) {
        cJSON_Delete(jsonObject);
        return;
    }
    cJSON_AddItemToArray(jsonArr, cJSON_CreateNumber(1));
    cJSON_AddItemToObject(jsonObject, CAPS_RSP_CAPS_KEY, jsonArr);
    FullCapsRsp capsRsp;
    FromJson(jsonObject, capsRsp);
    cJSON_Delete(jsonObject);
    EXPECT_TRUE(capsRsp.networkId.empty());
}

HWTEST_F(DhTransportTest, ToJson_CommMsg_001, TestSize.Level1)
{
    cJSON *json = nullptr;
    CommMsg commMsg;
    ToJson(json, commMsg);

    cJSON *jsonObject = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject != nullptr);
    ToJson(jsonObject, commMsg);
    cJSON_Delete(jsonObject);
}

HWTEST_F(DhTransportTest, FromJson_CommMsg_001, TestSize.Level1)
{
    cJSON *json = nullptr;
    CommMsg commMsg;
    FromJson(json, commMsg);

    cJSON *jsonObject = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject != nullptr);
    cJSON_AddStringToObject(jsonObject, COMM_MSG_CODE_KEY, "comm_msg_code_test");
    cJSON_AddStringToObject(jsonObject, COMM_MSG_MSG_KEY, "comm_msg_msg_test");
    FromJson(jsonObject, commMsg);
    cJSON_Delete(jsonObject);
    EXPECT_FALSE(commMsg.msg.empty());
}

HWTEST_F(DhTransportTest, FromJson_CommMsg_002, TestSize.Level1)
{
    cJSON *jsonObject = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject != nullptr);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_CODE_KEY, 1);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_MSG_KEY, 1);
    CommMsg commMsg;
    FromJson(jsonObject, commMsg);
    cJSON_Delete(jsonObject);
    EXPECT_TRUE(commMsg.msg.empty());
}

HWTEST_F(DhTransportTest, CreateClientSocket_001, TestSize.Level1)
{
    std::string remoteNetworkId = "";
    auto ret = dhTransportTest_->CreateClientSocket(remoteNetworkId);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}
}
}