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

#include "device_manager.h"
#include "device_manager_impl.h"

#include "dh_transport.h"
#include "dh_comm_tool.h"
#include "dh_transport_obj.h"
#include "dh_utils_tool.h"
#include "capability_info_manager.h"
#include "distributed_hardware_errno.h"
#include "mock_other_method.h"


using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace DistributedHardware {
using namespace std;

namespace {
    int32_t g_socketid = 1;
    std::string g_networkid = "networkId_test";
    constexpr int32_t SOCKETID = 10;
    constexpr int32_t INVALID_USER_ID = -100;
    constexpr int32_t INVALID_ACCOUNT_INFO_VALUE = -101;
    constexpr int32_t DH_COMM_REQ_FULL_CAPS = 1;
    constexpr int32_t DH_COMM_RSP_FULL_CAPS = 2;
}

static std::string g_mocklocalNetworkId = "123456789";
static bool g_mockDMValue = false;

std::string GetLocalNetworkId()
{
    return g_mocklocalNetworkId;
}

bool DeviceManagerImpl::CheckSinkAccessControl(const DmAccessCaller &caller, const DmAccessCallee &callee)
{
    return g_mockDMValue;
}

class DhTransportTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static inline shared_ptr<DeviceOtherMethodMock> otherMethodMock_ = nullptr;
private:
    std::shared_ptr<DHTransport> dhTransportTest_;
};

void DhTransportTest::SetUpTestCase()
{
    otherMethodMock_ = make_shared<DeviceOtherMethodMock>();
    DeviceOtherMethodMock::otherMethod = otherMethodMock_;
}

void DhTransportTest::TearDownTestCase()
{
    DeviceOtherMethodMock::otherMethod = nullptr;
    otherMethodMock_ = nullptr;
}

void DhTransportTest::SetUp()
{
    std::shared_ptr<DHCommTool> dhCommTool = std::make_shared<DHCommTool>();
    dhTransportTest_ = std::make_shared<DHTransport>(dhCommTool);
    g_mocklocalNetworkId = "123456789";
    g_mockDMValue = false;
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
    EXPECT_TRUE(commMsg.msg.empty());

    cJSON_AddStringToObject(jsonObject, COMM_MSG_CODE_KEY, "comm_msg_code_test");
    FromJson(jsonObject, commMsg);
    cJSON_Delete(jsonObject);
    EXPECT_TRUE(commMsg.msg.empty());
}

HWTEST_F(DhTransportTest, FromJson_CommMsg_002, TestSize.Level1)
{
    cJSON *jsonObject = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject != nullptr);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_CODE_KEY, 1);
    cJSON_AddStringToObject(jsonObject, COMM_MSG_USERID_KEY, "userId_test");
    CommMsg commMsg;
    FromJson(jsonObject, commMsg);
    cJSON_Delete(jsonObject);
    EXPECT_TRUE(commMsg.msg.empty());
}

HWTEST_F(DhTransportTest, FromJson_CommMsg_003, TestSize.Level1)
{
    cJSON *jsonObject = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject != nullptr);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_CODE_KEY, 1);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_USERID_KEY, 1);
    cJSON_AddStringToObject(jsonObject, COMM_MSG_TOKENID_KEY, "userId_test");
    CommMsg commMsg;
    FromJson(jsonObject, commMsg);
    cJSON_Delete(jsonObject);
    EXPECT_TRUE(commMsg.msg.empty());
}

HWTEST_F(DhTransportTest, FromJson_CommMsg_004, TestSize.Level1)
{
    cJSON *jsonObject = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject != nullptr);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_CODE_KEY, 1);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_USERID_KEY, 1);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_TOKENID_KEY, 1);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_MSG_KEY, 1);
    CommMsg commMsg;
    FromJson(jsonObject, commMsg);
    cJSON_Delete(jsonObject);
    EXPECT_TRUE(commMsg.msg.empty());
}

HWTEST_F(DhTransportTest, FromJson_CommMsg_005, TestSize.Level1)
{
    cJSON *jsonObject = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject != nullptr);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_CODE_KEY, 1);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_USERID_KEY, 1);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_TOKENID_KEY, 1);
    cJSON_AddStringToObject(jsonObject, COMM_MSG_MSG_KEY, "comm_msg_msg_test");
    CommMsg commMsg;
    FromJson(jsonObject, commMsg);
    cJSON_Delete(jsonObject);
    EXPECT_FALSE(commMsg.msg.empty());
}

HWTEST_F(DhTransportTest, FromJson_CommMsg_006, TestSize.Level1)
{
    cJSON *jsonObject = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject != nullptr);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_CODE_KEY, 1);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_USERID_KEY, 1);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_TOKENID_KEY, 1);
    cJSON_AddStringToObject(jsonObject, COMM_MSG_MSG_KEY, "comm_msg_msg_test");
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_ACCOUNTID_KEY, 1);
    CommMsg commMsg;
    FromJson(jsonObject, commMsg);
    cJSON_Delete(jsonObject);
    EXPECT_FALSE(commMsg.msg.empty());
}

HWTEST_F(DhTransportTest, FromJson_CommMsg_007, TestSize.Level1)
{
    cJSON *jsonObject = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject != nullptr);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_CODE_KEY, 1);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_USERID_KEY, 1);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_TOKENID_KEY, 1);
    cJSON_AddStringToObject(jsonObject, COMM_MSG_MSG_KEY, "comm_msg_msg_test");
    cJSON_AddStringToObject(jsonObject, COMM_MSG_ACCOUNTID_KEY, "account_test");
    CommMsg commMsg;
    FromJson(jsonObject, commMsg);
    cJSON_Delete(jsonObject);
    EXPECT_FALSE(commMsg.msg.empty());
}

HWTEST_F(DhTransportTest, FromJson_CommMsg_008, TestSize.Level1)
{
    cJSON *jsonObject = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject != nullptr);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_CODE_KEY, 1);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_USERID_KEY, 1);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_TOKENID_KEY, 1);
    cJSON_AddStringToObject(jsonObject, COMM_MSG_MSG_KEY, "comm_msg_msg_test");
    cJSON_AddStringToObject(jsonObject, COMM_MSG_ACCOUNTID_KEY, "account_test");
    cJSON_AddStringToObject(jsonObject, COMM_MSG_SYNC_META_KEY, "sync_meta_test");
    CommMsg commMsg;
    FromJson(jsonObject, commMsg);
    cJSON_Delete(jsonObject);
    EXPECT_FALSE(commMsg.msg.empty());
}

HWTEST_F(DhTransportTest, FromJson_CommMsg_009, TestSize.Level1)
{
    cJSON *jsonObject = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject != nullptr);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_CODE_KEY, 1);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_USERID_KEY, 1);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_TOKENID_KEY, 1);
    cJSON_AddStringToObject(jsonObject, COMM_MSG_MSG_KEY, "comm_msg_msg_test");
    cJSON_AddStringToObject(jsonObject, COMM_MSG_ACCOUNTID_KEY, "account_test");
    cJSON_AddBoolToObject(jsonObject, COMM_MSG_SYNC_META_KEY, true);
    CommMsg commMsg;
    FromJson(jsonObject, commMsg);
    cJSON_Delete(jsonObject);
    EXPECT_FALSE(commMsg.msg.empty());
}

HWTEST_F(DhTransportTest, CreateClientSocket_001, TestSize.Level1)
{
    std::string remoteNetworkId = "";
    auto ret = dhTransportTest_->CreateClientSocket(remoteNetworkId);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(DhTransportTest, CheckCalleeAclRight_001, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    std::shared_ptr<CommMsg> commMsg = std::make_shared<CommMsg>();
    commMsg->userId = -1;
    auto ret = dhTransportTest_->CheckCalleeAclRight(commMsg);
    EXPECT_EQ(true, ret);
}

HWTEST_F(DhTransportTest, CheckCalleeAclRight_002, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    std::shared_ptr<CommMsg> commMsg = std::make_shared<CommMsg>();
    commMsg->userId = 1;
    g_mocklocalNetworkId = "";
    auto ret = dhTransportTest_->CheckCalleeAclRight(commMsg);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DhTransportTest, CheckCalleeAclRight_003, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    std::shared_ptr<CommMsg> commMsg = std::make_shared<CommMsg>();
    commMsg->userId = 1;
    std::vector<int32_t> userIds;
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(INVALID_USER_ID)));
    auto ret = dhTransportTest_->CheckCalleeAclRight(commMsg);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DhTransportTest, CheckCalleeAclRight_004, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    std::shared_ptr<CommMsg> commMsg = std::make_shared<CommMsg>();
    commMsg->userId = 1;
    std::vector<int32_t> userIds{100, 101};
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(INVALID_USER_ID)));
    auto ret = dhTransportTest_->CheckCalleeAclRight(commMsg);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DhTransportTest, CheckCalleeAclRight_005, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    std::shared_ptr<CommMsg> commMsg = std::make_shared<CommMsg>();
    commMsg->userId = 1;
    std::vector<int32_t> userIds;
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(DH_FWK_SUCCESS)));
    auto ret = dhTransportTest_->CheckCalleeAclRight(commMsg);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DhTransportTest, CheckCalleeAclRight_006, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    std::shared_ptr<CommMsg> commMsg = std::make_shared<CommMsg>();
    commMsg->userId = 1;
    std::vector<int32_t> userIds{100, 101};
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(DH_FWK_SUCCESS)));

    AccountSA::OhosAccountInfo osAccountInfo;
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(INVALID_ACCOUNT_INFO_VALUE)));
    auto ret = dhTransportTest_->CheckCalleeAclRight(commMsg);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DhTransportTest, CheckCalleeAclRight_007, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    std::shared_ptr<CommMsg> commMsg = std::make_shared<CommMsg>();
    commMsg->userId = 1;
    std::vector<int32_t> userIds{100, 101};
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(DH_FWK_SUCCESS)));

    AccountSA::OhosAccountInfo osAccountInfo;
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(DH_FWK_SUCCESS)));
    g_mockDMValue = true;
    auto ret = dhTransportTest_->CheckCalleeAclRight(commMsg);
    EXPECT_EQ(true, ret);
}

HWTEST_F(DhTransportTest, HandleReceiveMessage_001, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    int32_t userId = 1;
    uint64_t tokenId = 1;
    std::string networkId = "123456";
    std::string accountId = "111";
    CommMsg commMsg(DH_COMM_REQ_FULL_CAPS, userId, tokenId, networkId, accountId, true);
    std::string payload = GetCommMsgString(commMsg);
    std::string compressedPayLoad = Compress(payload);
    std::vector<int32_t> userIds;
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(INVALID_USER_ID)));
    ASSERT_NO_FATAL_FAILURE(dhTransportTest_->HandleReceiveMessage(compressedPayLoad));
}

HWTEST_F(DhTransportTest, HandleReceiveMessage_002, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    int32_t userId = -1;
    uint64_t tokenId = 1;
    std::string networkId = "123456";
    std::string accountId = "111";
    CommMsg commMsg(DH_COMM_REQ_FULL_CAPS, userId, tokenId, networkId, accountId, true);
    std::string payload = GetCommMsgString(commMsg);
    std::string compressedPayLoad = Compress(payload);
    std::shared_ptr<DHCommTool> dhCommTool = std::make_shared<DHCommTool>();
    dhTransportTest_->dhCommToolWPtr_ = dhCommTool;
    ASSERT_NO_FATAL_FAILURE(dhTransportTest_->HandleReceiveMessage(compressedPayLoad));
}

HWTEST_F(DhTransportTest, HandleReceiveMessage_003, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    int32_t userId = 1;
    uint64_t tokenId = 1;
    std::string networkId = "123456";
    std::string accountId = "111";
    CommMsg commMsg(DH_COMM_RSP_FULL_CAPS, userId, tokenId, networkId, accountId, true);
    std::string payload = GetCommMsgString(commMsg);
    std::string compressedPayLoad = Compress(payload);
    ASSERT_NO_FATAL_FAILURE(dhTransportTest_->HandleReceiveMessage(compressedPayLoad));
}

HWTEST_F(DhTransportTest, HandleReceiveMessage_004, TestSize.Level1)
{
    ASSERT_TRUE(dhTransportTest_ != nullptr);
    int32_t userId = 1;
    uint64_t tokenId = 1;
    std::string networkId = "123456";
    std::string accountId = "111";
    CommMsg commMsg(DH_COMM_RSP_FULL_CAPS, userId, tokenId, networkId, accountId, true);
    std::string payload = GetCommMsgString(commMsg);
    std::string compressedPayLoad = Compress(payload);
    std::shared_ptr<DHCommTool> dhCommTool = std::make_shared<DHCommTool>();
    dhTransportTest_->dhCommToolWPtr_ = dhCommTool;
    dhCommTool->Init();
    ASSERT_NO_FATAL_FAILURE(dhTransportTest_->HandleReceiveMessage(compressedPayLoad));
    dhCommTool->UnInit();
}
}
}