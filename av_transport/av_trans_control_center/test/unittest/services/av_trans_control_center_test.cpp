/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "av_trans_control_center_test.h"
#include "av_trans_errno.h"

using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
void AVTransControlCenterTest::SetUpTestCase(void)
{
}

void AVTransControlCenterTest::TearDownTestCase(void)
{
}

void AVTransControlCenterTest::SetUp()
{
}

void AVTransControlCenterTest::TearDown()
{
}

/**
 * @tc.name: initialize_av_center_001
 * @tc.desc: initialize av center function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, initialize_av_center_001, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    TransRole transRole = TransRole::UNKNOWN;
    int32_t engineId = BASE_ENGINE_ID;
    int32_t ret = center_->InitializeAVCenter(transRole, engineId);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM_VALUE, ret);
}

/**
 * @tc.name: initialize_av_center_002
 * @tc.desc: initialize av center function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, initialize_av_center_002, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    TransRole transRole = TransRole::AV_SENDER;
    int32_t engineId = BASE_ENGINE_ID;
    center_->initialized_= true;
    int32_t ret = center_->InitializeAVCenter(transRole, engineId);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
    transRole = TransRole::AV_RECEIVER;
    ret = center_->InitializeAVCenter(transRole, engineId);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

/**
 * @tc.name: initialize_av_center_003
 * @tc.desc: initialize av center function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, initialize_av_center_003, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    TransRole transRole = TransRole::AV_SENDER;
    int32_t engineId = BASE_ENGINE_ID;
    center_->initialized_= false;
    int32_t ret = center_->InitializeAVCenter(transRole, engineId);
    EXPECT_EQ(ERR_DH_AVT_SESSION_ERROR, ret);
}

/**
 * @tc.name: release_av_center_001
 * @tc.desc: release av center function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, release_av_center_001, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    int32_t engineId = INVALID_ENGINE_ID;
    int32_t ret = center_->ReleaseAVCenter(engineId);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM_VALUE, ret);
}

/**
 * @tc.name: release_av_center_002
 * @tc.desc: release av center function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, release_av_center_002, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    int32_t engineId = BASE_ENGINE_ID;
    int32_t engineIdSecond = BASE_ENGINE_ID + 1 ;

    int32_t ret = center_->ReleaseAVCenter(engineId);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);

    center_->engine2DevIdMap_.insert(std::make_pair(engineId, "engineId"));
    center_->engine2DevIdMap_.insert(std::make_pair(engineIdSecond, "engineId"));
    ret = center_->ReleaseAVCenter(engineId);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

/**
 * @tc.name: release_av_center_003
 * @tc.desc: release av center function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, release_av_center_003, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    int32_t engineId = BASE_ENGINE_ID;
    center_->engine2DevIdMap_.insert(std::make_pair(engineId, "engineId"));
    int32_t ret = center_->ReleaseAVCenter(engineId);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

/**
 * @tc.name: release_av_center_004
 * @tc.desc: release av center function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, release_av_center_004, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    int32_t engineId = BASE_ENGINE_ID;
    center_->engine2DevIdMap_.insert(std::make_pair(engineId, "engineId"));
    center_->connectedDevIds_.push_back("engineId");
    int32_t ret = center_->ReleaseAVCenter(engineId);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

/**
 * @tc.name: create_control_channel_001
 * @tc.desc: create control channel function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, create_control_channel_001, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    int32_t engineId = INVALID_ENGINE_ID;
    std::string peerDevId = "peerDevId";
    int32_t ret = center_->CreateControlChannel(engineId, peerDevId);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM_VALUE, ret);
}

/**
 * @tc.name: create_control_channel_002
 * @tc.desc: create control channel function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, create_control_channel_002, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    int32_t engineId = BASE_ENGINE_ID;
    std::string peerDevId = "peerDevId";
    center_->initialized_ = false;
    int32_t ret = center_->CreateControlChannel(engineId, peerDevId);
    EXPECT_EQ(ERR_DH_AVT_CREATE_CHANNEL_FAILED, ret);
}

/**
 * @tc.name: create_control_channel_003
 * @tc.desc: create control channel function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, create_control_channel_003, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    int32_t engineId = BASE_ENGINE_ID;
    std::string peerDevId = "peerDevId";
    center_->initialized_ = true;
    center_->connectedDevIds_.push_back("peerDevId");
    int32_t ret = center_->CreateControlChannel(engineId, peerDevId);
    EXPECT_EQ(ERR_DH_AVT_CHANNEL_ALREADY_CREATED, ret);
}

/**
 * @tc.name: create_control_channel_004
 * @tc.desc: create control channel function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, create_control_channel_004, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    int32_t engineId = BASE_ENGINE_ID;
    std::string peerDevId = "peerDevId";
    center_->initialized_ = true;
    int32_t ret = center_->CreateControlChannel(engineId, peerDevId);
    EXPECT_EQ(ERR_DH_AVT_SESSION_ERROR, ret);
}

/**
 * @tc.name: notify_av_center_001
 * @tc.desc: notify av center function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, notify_av_center_001, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    int32_t engineId = BASE_ENGINE_ID;
    AVTransEvent event;
    event.type = EventType::EVENT_ADD_STREAM;

    int32_t ret = center_->NotifyAVCenter(engineId, event);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

/**
 * @tc.name: notify_av_center_002
 * @tc.desc: notify av center function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, notify_av_center_002, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    int32_t engineId = BASE_ENGINE_ID;
    AVTransEvent event;
    event.type = EventType::EVENT_REMOVE_STREAM;

    int32_t ret = center_->NotifyAVCenter(engineId, event);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

/**
 * @tc.name: notify_av_center_003
 * @tc.desc: notify av center function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, notify_av_center_003, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    int32_t engineId = BASE_ENGINE_ID;
    AVTransEvent event;
    event.type = EventType::EVENT_CHANNEL_OPENED;

    int32_t ret = center_->NotifyAVCenter(engineId, event);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

/**
 * @tc.name: register_ctl_center_callback_001
 * @tc.desc: register ctl center callback function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, register_ctl_center_callback_001, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    int32_t engineId = INVALID_ENGINE_ID;
    sptr<CenterCallback> callback = nullptr;
    int32_t ret = center_->RegisterCtlCenterCallback(engineId, callback);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM_VALUE, ret);
}

/**
 * @tc.name: register_ctl_center_callback_002
 * @tc.desc: register ctl center callback function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, register_ctl_center_callback_002, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    int32_t engineId = BASE_ENGINE_ID;
    sptr<CenterCallback> callback = nullptr;
    int32_t ret = center_->RegisterCtlCenterCallback(engineId, callback);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM_VALUE, ret);
}

/**
 * @tc.name: register_ctl_center_callback_003
 * @tc.desc: register ctl center callback function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, register_ctl_center_callback_003, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    int32_t engineId = BASE_ENGINE_ID;
    sptr<CenterCallback> callback = sptr<CenterCallback>(new CenterCallback());
    int32_t ret = center_->RegisterCtlCenterCallback(engineId, callback);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

/**
 * @tc.name: on_channel_event_001
 * @tc.desc: on channel event function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, on_channel_event_001, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    AVTransEvent event;
    event.type = EventType::EVENT_CHANNEL_CLOSED;
    center_->OnChannelEvent(event);
    int32_t engineId = BASE_ENGINE_ID;
    sptr<CenterCallback> callback = sptr<CenterCallback>(new CenterCallback());
    int32_t ret = center_->RegisterCtlCenterCallback(engineId, callback);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

/**
 * @tc.name: on_channel_event_002
 * @tc.desc: on channel event function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, on_channel_event_002, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    AVTransEvent event;
    event.type = EventType::EVENT_DATA_RECEIVED;
    center_->OnChannelEvent(event);
    int32_t engineId = BASE_ENGINE_ID;
    sptr<CenterCallback> callback = sptr<CenterCallback>(new CenterCallback());
    int32_t ret = center_->RegisterCtlCenterCallback(engineId, callback);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

/**
 * @tc.name: on_channel_event_003
 * @tc.desc: on channel event function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, on_channel_event_003, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    AVTransEvent event;
    event.type = EventType::EVENT_TIME_SYNC_RESULT;
    center_->sessionName_ = AV_SYNC_RECEIVER_CONTROL_SESSION_NAME;
    center_->OnChannelEvent(event);
    int32_t engineId = BASE_ENGINE_ID;
    sptr<CenterCallback> callback = sptr<CenterCallback>(new CenterCallback());
    int32_t ret = center_->RegisterCtlCenterCallback(engineId, callback);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

/**
 * @tc.name: on_channel_event_004
 * @tc.desc: on channel event function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, on_channel_event_004, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    AVTransEvent event;
    event.type = EventType::EVENT_REMOVE_STREAM;
    center_->sessionName_ = AV_SYNC_RECEIVER_CONTROL_SESSION_NAME;
    center_->OnChannelEvent(event);

    StreamData *data = nullptr;
    StreamData *ext = nullptr;
    center_->OnStreamReceived(data, ext);
    int32_t engineId = BASE_ENGINE_ID;
    sptr<CenterCallback> callback = sptr<CenterCallback>(new CenterCallback());
    int32_t ret = center_->RegisterCtlCenterCallback(engineId, callback);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

/**
 * @tc.name: send_message_001
 * @tc.desc: send message function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, send_message_001, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    std::shared_ptr<AVTransMessage> message = nullptr;
    int32_t ret = center_->SendMessage(message);;
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

/**
 * @tc.name: send_message_002
 * @tc.desc: send message function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, send_message_002, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    std::shared_ptr<AVTransMessage> message = std::make_shared<AVTransMessage>();
    int32_t ret = center_->SendMessage(message);;
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

/**
 * @tc.name: set_param_2_engines_001
 * @tc.desc: set param2engines function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, set_param_2_engines_001, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    AVTransTag tag = AVTransTag::START_AV_SYNC;
    std::string value = "value";
    int32_t engineId = BASE_ENGINE_ID;
    sptr<CenterCallback> callback = sptr<CenterCallback>(new CenterCallback());
    center_->callbackMap_.insert(std::make_pair(engineId, callback));
    center_->callbackMap_.insert(std::make_pair(engineId + 1, nullptr));
    center_->SetParam2Engines(tag, value);
    EXPECT_EQ(callback->value_, value);
}

/**
 * @tc.name: set_param_2_engines_002
 * @tc.desc: set param2engines function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, set_param_2_engines_002, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    AVTransSharedMemory memory;
    memory.name = "memory";
    sptr<CenterCallback> callback = sptr<CenterCallback>(new CenterCallback());
    center_->callbackMap_.insert(std::make_pair(0, callback));
    center_->callbackMap_.insert(std::make_pair(1, nullptr));
    center_->SetParam2Engines(memory);
    EXPECT_EQ(callback->memory_.name, memory.name);
}

/**
 * @tc.name: handle_channel_event_001
 * @tc.desc: handle channel event function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, handle_channel_event_001, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    AVTransEvent event;
    event.type = EventType::EVENT_CHANNEL_CLOSED;
    center_->HandleChannelEvent(event);
    event.type = EventType::EVENT_CHANNEL_OPEN_FAIL;
    center_->HandleChannelEvent(event);
    event.type = EventType::EVENT_CHANNEL_OPENED;
    event.content = AV_SYNC_RECEIVER_CONTROL_SESSION_NAME;
    center_->HandleChannelEvent(event);
    event.type = EventType::EVENT_CHANNEL_OPENED;
    event.content = AV_SYNC_SENDER_CONTROL_SESSION_NAME;
    center_->HandleChannelEvent(event);
    event.type = EventType::EVENT_START_SUCCESS;
    center_->HandleChannelEvent(event);
    EXPECT_NE(0, center_->connectedDevIds_.size());
}

/**
 * @tc.name: is_invalid_engineId_001
 * @tc.desc: is invalid engineId function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, is_invalid_engineId_001, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    int32_t engineId = BASE_ENGINE_ID - 1;
    bool ret = center_->IsInvalidEngineId(engineId);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: is_invalid_engineId_002
 * @tc.desc: is invalid engineId function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterTest, is_invalid_engineId_002, TestSize.Level0)
{
    center_ = std::make_shared<AVTransControlCenter>();
    int32_t engineId = BASE_ENGINE_ID + 1;
    bool ret = center_->IsInvalidEngineId(engineId);
    EXPECT_EQ(true, ret);
}
} // namespace DistributedHardware
} // namespace OHOS