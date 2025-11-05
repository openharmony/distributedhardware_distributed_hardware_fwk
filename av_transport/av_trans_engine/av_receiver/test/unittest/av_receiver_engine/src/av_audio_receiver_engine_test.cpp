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

#include <iostream>
#include "av_audio_receiver_engine_test.h"
#include "distributed_hardware_fwk_kit.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
const std::string FILTERNAME = "avreceivererengine";

void AvAudioReceiverEngineTest::SetUp()
{
}

void AvAudioReceiverEngineTest::TearDown()
{
}

void AvAudioReceiverEngineTest::SetUpTestCase()
{
}

void AvAudioReceiverEngineTest::TearDownTestCase()
{
}

HWTEST_F(AvAudioReceiverEngineTest, Initialize_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->isInitialized_ = true;
    std::string value264 = MIME_VIDEO_H264;
    receiver->SetVideoCodecType(value264);
    std::string value = MIME_VIDEO_H265;
    receiver->SetVideoCodecType(value);
    int32_t ret = receiver->Initialize();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, Initialize_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = nullptr;
    int32_t ret = receiver->Initialize();
    EXPECT_EQ(ERR_DH_AVT_INIT_FAILED, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, Initialize_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->sessionName_ = "";
    int32_t ret = receiver->Initialize();
    EXPECT_EQ(ERR_DH_AVT_INIT_FAILED, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, InitPipeline_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->pipeline_ = nullptr;
    receiver->SetEngineReady(ownerName);
    receiver->pipeline_ = std::make_shared<Pipeline::Pipeline>();
    receiver->SetEngineReady(ownerName);
    int32_t ret = receiver->InitPipeline();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, InitPipeline_002, testing::ext::TestSize.Level1)
{
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(OWNER_NAME_D_MIC, peerDevId);
    int32_t ret = receiver->InitPipeline();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, InitPipeline_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_SCREEN;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    int32_t ret = receiver->InitPipeline();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, InitPipeline_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);

    receiver->SetCurrentState(StateId::IDLE);
    int32_t result = receiver->PreparePipeline("configParam");

    receiver->SetCurrentState(StateId::STARTED);
    result = receiver->PreparePipeline("configParam");

    receiver->SetCurrentState(StateId::INITIALIZED);
    result = receiver->PreparePipeline("");

    receiver->meta_ = nullptr;
    result = receiver->PreparePipeline("configParam");
    EXPECT_EQ(result, ERR_DH_AVT_NULL_POINTER);
}

HWTEST_F(AvAudioReceiverEngineTest, CreateControlChannel_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    std::vector<std::string> dstDevIds;
    int32_t ret = receiver->CreateControlChannel(dstDevIds, ChannelAttribute{TransStrategy::LOW_LATANCY_STRATEGY});
    EXPECT_EQ(ERR_DH_AVT_NULL_POINTER, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, CreateControlChannel_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    std::vector<std::string> dstDevIds = {peerDevId};
    int32_t ret = receiver->CreateControlChannel(dstDevIds, ChannelAttribute{TransStrategy::LOW_LATANCY_STRATEGY});
    EXPECT_EQ(ERR_DH_AVT_CREATE_CHANNEL_FAILED, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, CreateControlChannel_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    std::vector<std::string> dstDevIds = {peerDevId};
    int32_t ret = receiver->CreateControlChannel(dstDevIds, ChannelAttribute{TransStrategy::LOW_LATANCY_STRATEGY});
    EXPECT_EQ(ERR_DH_AVT_CREATE_CHANNEL_FAILED, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, Start_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::IDLE;
    receiver->pipeline_ = std::make_shared<Pipeline::Pipeline>();
    int32_t ret = receiver->Start();
    EXPECT_EQ(ERR_DH_AVT_START_FAILED, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, Start_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::CH_CREATED;
    receiver->pipeline_ = std::make_shared<Pipeline::Pipeline>();
    int32_t ret = receiver->Start();
    receiver->Stop();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, Start_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::CH_CREATED;
    receiver->pipeline_ = nullptr;
    int32_t ret = receiver->Start();
    receiver->Stop();
    EXPECT_EQ(ERR_DH_AVT_START_FAILED, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, Release_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->isInitialized_ = true;
    receiver->pipeline_ = std::make_shared<Pipeline::Pipeline>();
    receiver->dhFwkKit_ = std::make_shared<DistributedHardwareFwkKit>();
    receiver->SetCurrentState(StateId::IDLE);
    int32_t result = receiver->Release();
    EXPECT_EQ(result, DH_AVT_SUCCESS);
}

HWTEST_F(AvAudioReceiverEngineTest, Release_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->isInitialized_ = true;
    receiver->pipeline_ = nullptr;
    receiver->dhFwkKit_ = std::make_shared<DistributedHardwareFwkKit>();
    receiver->SetCurrentState(StateId::IDLE);
    int32_t result = receiver->Release();
    EXPECT_EQ(result, DH_AVT_SUCCESS);
}

HWTEST_F(AvAudioReceiverEngineTest, Release_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->isInitialized_ = true;
    receiver->pipeline_ = nullptr;
    receiver->dhFwkKit_ = std::make_shared<DistributedHardwareFwkKit>();
    receiver->SetCurrentState(StateId::IDLE);
    int32_t result = receiver->Release();
    EXPECT_EQ(result, DH_AVT_SUCCESS);
}

HWTEST_F(AvAudioReceiverEngineTest, Release_005, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->isInitialized_ = true;
    receiver->pipeline_ = nullptr;
    receiver->dhFwkKit_ = std::make_shared<DistributedHardwareFwkKit>();
    receiver->SetCurrentState(StateId::INITIALIZED);
    int32_t result = receiver->Release();
    EXPECT_EQ(result, DH_AVT_SUCCESS);
}

HWTEST_F(AvAudioReceiverEngineTest, Stop_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::CH_CREATED;
    receiver->pipeline_ = std::make_shared<Pipeline::Pipeline>();
    receiver->Start();
    int32_t ret = receiver->Stop();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, SetParameterInner_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->meta_ = std::make_shared<Media::Meta>();
    receiver->SetParameterInner(AVTransTag::VIDEO_CODEC_TYPE, "H264");
    receiver->SetParameterInner(AVTransTag::AUDIO_CODEC_TYPE, "1");
    receiver->SetParameterInner(AVTransTag::AUDIO_CHANNEL_MASK, "2");
    receiver->SetParameterInner(AVTransTag::AUDIO_SAMPLE_RATE, "3");
    receiver->SetParameterInner(AVTransTag::AUDIO_CHANNEL_LAYOUT, "4");
    receiver->SetParameterInner(AVTransTag::AUDIO_SAMPLE_FORMAT, "5");
    receiver->SetParameterInner(AVTransTag::AUDIO_FRAME_SIZE, "6");
    receiver->SetParameterInner(AVTransTag::TIME_SYNC_RESULT, "7");
    receiver->SetParameterInner(AVTransTag::START_AV_SYNC, "8");
    receiver->SetParameterInner(AVTransTag::STOP_AV_SYNC, "0");
    receiver->SetParameterInner(AVTransTag::SHARED_MEMORY_FD, "1");
    receiver->SetParameterInner(AVTransTag::ENGINE_READY, "2");
    EXPECT_NE(receiver->meta_, nullptr);
}

HWTEST_F(AvAudioReceiverEngineTest, SetParameterInner_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->meta_ = nullptr;
    receiver->SetParameterInner(AVTransTag::VIDEO_CODEC_TYPE, "H264");
    receiver->SetParameterInner(AVTransTag::AUDIO_CODEC_TYPE, "1");
    receiver->SetParameterInner(AVTransTag::AUDIO_CHANNEL_MASK, "2");
    receiver->SetParameterInner(AVTransTag::AUDIO_SAMPLE_RATE, "3");
    receiver->SetParameterInner(AVTransTag::AUDIO_CHANNEL_LAYOUT, "4");
    receiver->SetParameterInner(AVTransTag::AUDIO_SAMPLE_FORMAT, "5");
    receiver->SetParameterInner(AVTransTag::AUDIO_FRAME_SIZE, "6");
    receiver->SetParameterInner(AVTransTag::TIME_SYNC_RESULT, "7");
    receiver->SetParameterInner(AVTransTag::START_AV_SYNC, "8");
    receiver->SetParameterInner(AVTransTag::STOP_AV_SYNC, "0");
    receiver->SetParameterInner(AVTransTag::SHARED_MEMORY_FD, "1");
    EXPECT_NO_FATAL_FAILURE(receiver->SetParameterInner(AVTransTag::ENGINE_READY, "2"));
}

HWTEST_F(AvAudioReceiverEngineTest, SetParameterInner_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->meta_ = nullptr;
    EXPECT_NO_FATAL_FAILURE(receiver->SetParameterInner(AVTransTag::INVALID, "1"));
}

HWTEST_F(AvAudioReceiverEngineTest, Stop_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->pipeline_ = nullptr;

    int32_t result = receiver->Stop();
    EXPECT_EQ(result, ERR_DH_AVT_START_FAILED);
}

HWTEST_F(AvAudioReceiverEngineTest, SetParameter_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    int32_t ret = receiver->SetParameter(AVTransTag::INVALID, value);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, SetParameter_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    int32_t ret = receiver->SetParameter(AVTransTag::INVALID, value);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, SetParameter_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    int32_t ret = receiver->SetParameter(AVTransTag::INVALID, value);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, SetParameter_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "123";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    std::shared_ptr<Pipeline::Pipeline> pipeline_ = nullptr;
    receiver->pipeline_ = std::make_shared<Pipeline::Pipeline>();
    int32_t ret = receiver->SetParameter(AVTransTag::VIDEO_WIDTH, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, SetParameter_005, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->meta_ = nullptr;
    receiver->SetParameter(AVTransTag::VIDEO_WIDTH, "H264");
    receiver->SetParameter(AVTransTag::VIDEO_HEIGHT, "1");
    receiver->SetParameter(AVTransTag::VIDEO_FRAME_RATE, "2");
    receiver->SetParameter(AVTransTag::AUDIO_BIT_RATE, "3");
    receiver->SetParameter(AVTransTag::VIDEO_FRAME_RATE, "4");
    receiver->SetParameter(AVTransTag::VIDEO_CODEC_TYPE, "5");
    receiver->SetParameter(AVTransTag::AUDIO_CODEC_TYPE, "6");
    receiver->SetParameter(AVTransTag::AUDIO_CHANNEL_MASK, "7");
    receiver->SetParameter(AVTransTag::AUDIO_SAMPLE_RATE, "8");
    receiver->SetParameter(AVTransTag::AUDIO_CHANNEL_LAYOUT, "9");
    receiver->SetParameter(AVTransTag::AUDIO_SAMPLE_FORMAT, "10");
    receiver->SetParameter(AVTransTag::AUDIO_FRAME_SIZE, "11");
    receiver->SetParameter(AVTransTag::TIME_SYNC_RESULT, "12");
    receiver->SetParameter(AVTransTag::START_AV_SYNC, "13");
    receiver->SetParameter(AVTransTag::STOP_AV_SYNC, "14");
    receiver->SetParameter(AVTransTag::SHARED_MEMORY_FD, "15");
    EXPECT_NO_FATAL_FAILURE(receiver->SetParameter(AVTransTag::ENGINE_READY, "16"));
}

HWTEST_F(AvAudioReceiverEngineTest, SetParameter_006, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->meta_ = std::make_shared<Media::Meta>();
    receiver->SetParameter(AVTransTag::VIDEO_WIDTH, "0");
    receiver->SetParameter(AVTransTag::VIDEO_HEIGHT, "1");
    receiver->SetParameter(AVTransTag::VIDEO_FRAME_RATE, "2");
    receiver->SetParameter(AVTransTag::AUDIO_BIT_RATE, "3");
    receiver->SetParameter(AVTransTag::VIDEO_FRAME_RATE, "4");
    receiver->SetParameter(AVTransTag::VIDEO_CODEC_TYPE, "5");
    receiver->SetParameter(AVTransTag::AUDIO_CODEC_TYPE, "6");
    receiver->SetParameter(AVTransTag::AUDIO_CHANNEL_MASK, "7");
    receiver->SetParameter(AVTransTag::AUDIO_SAMPLE_RATE, "8");
    receiver->SetParameter(AVTransTag::AUDIO_CHANNEL_LAYOUT, "9");
    receiver->SetParameter(AVTransTag::AUDIO_SAMPLE_FORMAT, "10");
    receiver->SetParameter(AVTransTag::AUDIO_FRAME_SIZE, "11");
    receiver->SetParameter(AVTransTag::TIME_SYNC_RESULT, "12");
    receiver->SetParameter(AVTransTag::START_AV_SYNC, "13");
    receiver->SetParameter(AVTransTag::STOP_AV_SYNC, "14");
    receiver->SetParameter(AVTransTag::SHARED_MEMORY_FD, "15");
    receiver->SetParameter(AVTransTag::ENGINE_READY, "16");
    receiver->SetParameter(AVTransTag::VIDEO_BIT_RATE, "17");
    EXPECT_NE(receiver->meta_, nullptr);
}

HWTEST_F(AvAudioReceiverEngineTest, PreparePipeline_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::STARTED;
    int32_t ret = receiver->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_PREPARE_FAILED, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, PreparePipeline_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::INITIALIZED;
    int32_t ret = receiver->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_NULL_POINTER, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, PreparePipeline_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::INITIALIZED;
    int32_t ret = receiver->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_NULL_POINTER, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, PreparePipeline_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::INITIALIZED;
    int32_t ret = receiver->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_NULL_POINTER, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, PreparePipeline_005, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_MIC;
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->pipeline_ = std::make_shared<Pipeline::Pipeline>();
    int32_t ret = receiver->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_PREPARE_FAILED, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, SendMessage_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    int32_t ret = receiver->SendMessage(nullptr);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, SendMessage_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    auto avMessage = std::make_shared<AVTransMessage>((uint32_t)AVTransTag::START_AV_SYNC,
        ownerName, peerDevId);

    int32_t ret = receiver->SendMessage(avMessage);
    EXPECT_EQ(ERR_DH_AVT_SEND_DATA_FAILED, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, RegisterReceiverCallback_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    std::shared_ptr<ReceiverEngineCallback> callback = nullptr;
    int32_t ret = receiver->RegisterReceiverCallback(callback);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, RegisterReceiverCallback_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    std::shared_ptr<ReceiverEngineCallback> callback = std::make_shared<ReceiverEngineCallback>();
    int32_t ret = receiver->RegisterReceiverCallback(callback);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, OnChannelEvent_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->receiverCallback_ = nullptr;
    AVTransEvent transEvent;
    transEvent.content = "content";
    transEvent.type = EventType::EVENT_CHANNEL_OPENED;
    receiver->OnChannelEvent(transEvent);
    receiver->receiverCallback_ = std::make_shared<ReceiverEngineCallback>();
    receiver->OnChannelEvent(transEvent);

    EXPECT_EQ(StateId::CH_CREATED, receiver->currentState_);
}

HWTEST_F(AvAudioReceiverEngineTest, OnChannelEvent_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    AVTransEvent transEvent;
    transEvent.content = "content";
    transEvent.type = EventType::EVENT_CHANNEL_OPEN_FAIL;
    receiver->receiverCallback_ = std::make_shared<ReceiverEngineCallback>();
    receiver->OnChannelEvent(transEvent);

    EXPECT_EQ(StateId::INITIALIZED, receiver->currentState_);
}

HWTEST_F(AvAudioReceiverEngineTest, OnChannelEvent_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    AVTransEvent event;
    event.content = "content";
    event.type = EventType::EVENT_CHANNEL_CLOSED;
    receiver->receiverCallback_ = std::make_shared<ReceiverEngineCallback>();
    receiver->currentState_ = StateId::CH_CREATED;
    receiver->OnChannelEvent(event);
    receiver->currentState_ = StateId::IDLE;
    receiver->OnChannelEvent(event);
    receiver->currentState_ = StateId::INITIALIZED;
    receiver->OnChannelEvent(event);

    event.type = EventType::EVENT_DATA_RECEIVED;
    EXPECT_NO_FATAL_FAILURE(receiver->OnChannelEvent(event));
}

HWTEST_F(AvAudioReceiverEngineTest, OnChannelEvent_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    AVTransEvent transEvent;
    transEvent.content = "content";
    transEvent.type = EventType::EVENT_REMOVE_STREAM;
    receiver->receiverCallback_ = std::make_shared<ReceiverEngineCallback>();
    receiver->OnChannelEvent(transEvent);

    EXPECT_NE(StateId::INITIALIZED, receiver->currentState_);
}

HWTEST_F(AvAudioReceiverEngineTest, Release_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->RegRespFunMap();
    std::string value = "50";
    receiver->SetVideoWidth(value);
    receiver->SetVideoHeight(value);
    receiver->SetVideoFrameRate(value);
    receiver->SetAudioBitRate(value);
    receiver->SetVideoBitRate(value);
    std::string codecType = MIME_VIDEO_H264;
    receiver->SetVideoCodecType(codecType);
    codecType = MIME_VIDEO_H265;
    receiver->SetVideoCodecType(codecType);
    receiver->SetAudioCodecType(codecType);
    receiver->SetAudioChannelMask(value);
    receiver->SetAudioSampleRate(value);
    receiver->SetAudioChannelLayout(value);
    receiver->SetAudioSampleFormat(value);
    receiver->SetAudioFrameSize(value);

    std::string params = "";
    receiver->SetSyncResult(params);
    receiver->SetStartAvSync(params);
    receiver->SetStopAvSync(params);
    receiver->SetSharedMemoryFd(params);
    receiver->SetEngineReady(params);

    receiver->currentState_ = StateId::IDLE;
    receiver->dhFwkKit_ = std::make_shared<DistributedHardwareFwkKit>();
    int32_t ret = receiver->Release();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, StartDumpMediaData_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = nullptr;
    bool ret = receiver->StartDumpMediaData();
    EXPECT_EQ(true, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, StartDumpMediaData_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    bool ret = receiver->StartDumpMediaData();
    EXPECT_EQ(true, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, StopDumpMediaData_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = nullptr;
    bool ret = receiver->StopDumpMediaData();
    EXPECT_EQ(true, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, StopDumpMediaData_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    bool ret = receiver->StopDumpMediaData();
    EXPECT_EQ(true, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, ReStartDumpMediaData_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = nullptr;
    bool ret = receiver->ReStartDumpMediaData();
    EXPECT_EQ(true, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, ReStartDumpMediaData_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    bool ret = receiver->ReStartDumpMediaData();
    EXPECT_EQ(true, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, HandleOutputBuffer_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::INITIALIZED;
    std::shared_ptr<Media::AVBuffer> buffer = std::make_shared<Media::AVBuffer>();
    receiver->receiverCallback_ = std::make_shared<ReceiverEngineCallback>();
    int32_t ret = receiver->HandleOutputBuffer(buffer);
    EXPECT_EQ(ERR_DH_AVT_OUTPUT_DATA_FAILED, ret);
}

HWTEST_F(AvAudioReceiverEngineTest, OnEvent_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);

    Pipeline::Event event;
    event.srcFilter = "filter";
    event.type = Pipeline::EventType::EVENT_READY;
    receiver->currentState_ = StateId::INITIALIZED;

    EXPECT_NO_FATAL_FAILURE(receiver->OnEvent(event));

    receiver->receiverCallback_ = std::make_shared<ReceiverEngineCallback>();
    EXPECT_NO_FATAL_FAILURE(receiver->OnEvent(event));
}

HWTEST_F(AvAudioReceiverEngineTest, LinkAudioDecoderFilter_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    std::shared_ptr<Pipeline::Filter> preFilter =
        std::make_shared<Pipeline::Filter>(ownerName, Pipeline::FilterType::FILTERTYPE_SOURCE, false);
    Status status = receiver->LinkAudioDecoderFilter(preFilter, Pipeline::StreamType::STREAMTYPE_DECODED_AUDIO);
    receiver->audioDecoder_ = nullptr;
    receiver->pipeline_ = std::make_shared<Pipeline::Pipeline>();
    status = receiver->LinkAudioDecoderFilter(preFilter, Pipeline::StreamType::STREAMTYPE_DECODED_AUDIO);
    receiver->pipeline_ = nullptr;
    status = receiver->LinkAudioDecoderFilter(preFilter, Pipeline::StreamType::STREAMTYPE_DECODED_AUDIO);
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvAudioReceiverEngineTest, LinkAudioSinkFilter_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    std::shared_ptr<Pipeline::Filter> preFilter =
        std::make_shared<Pipeline::Filter>(ownerName, Pipeline::FilterType::FILTERTYPE_SOURCE, false);
    Status status = receiver->LinkAudioSinkFilter(preFilter, Pipeline::StreamType::STREAMTYPE_DECODED_AUDIO);

    receiver->avOutput_ = nullptr;
    receiver->pipeline_ = std::make_shared<Pipeline::Pipeline>();
    status = receiver->LinkAudioSinkFilter(preFilter, Pipeline::StreamType::STREAMTYPE_DECODED_AUDIO);

    receiver->pipeline_ = nullptr;
    status = receiver->LinkAudioSinkFilter(preFilter, Pipeline::StreamType::STREAMTYPE_DECODED_AUDIO);
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvAudioReceiverEngineTest, Prepare_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);

    receiver->avInput_ = nullptr;
    EXPECT_EQ(receiver->Prepare(), Status::ERROR_INVALID_OPERATION);

    receiver->avInput_= std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
        Pipeline::FilterType::AUDIO_DATA_SOURCE);
    receiver->pipeline_ = nullptr;
    EXPECT_EQ(receiver->Prepare(), Status::ERROR_INVALID_OPERATION);

    receiver->pipeline_ = std::make_shared<Pipeline::Pipeline>();
    EXPECT_EQ(receiver->Prepare(), Status::OK);

    receiver->pipeline_->Prepare();
    EXPECT_EQ(receiver->Prepare(), Status::OK);
}

HWTEST_F(AvAudioReceiverEngineTest, OnCallback_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::INITIALIZED;

    EXPECT_EQ(Status::OK, receiver->OnCallback(nullptr, Pipeline::FilterCallBackCommand::FILTER_CALLBACK_COMMAND_MAX,
        Pipeline::StreamType::STREAMTYPE_MAX));
}

HWTEST_F(AvAudioReceiverEngineTest, OnCallback_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::INITIALIZED;

    EXPECT_EQ(Status::OK, receiver->OnCallback(nullptr, Pipeline::FilterCallBackCommand::NEXT_FILTER_NEEDED,
        Pipeline::StreamType::STREAMTYPE_MAX));
}

HWTEST_F(AvAudioReceiverEngineTest, OnCallback_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::INITIALIZED;

    EXPECT_EQ(Status::ERROR_NULL_POINTER, receiver->OnCallback(nullptr,
        Pipeline::FilterCallBackCommand::NEXT_FILTER_NEEDED, Pipeline::StreamType::STREAMTYPE_RAW_AUDIO));
}

HWTEST_F(AvAudioReceiverEngineTest, OnCallback_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::INITIALIZED;

    EXPECT_EQ(Status::ERROR_NULL_POINTER, receiver->OnCallback(nullptr,
        Pipeline::FilterCallBackCommand::NEXT_FILTER_NEEDED, Pipeline::StreamType::STREAMTYPE_DECODED_AUDIO));
}

} // namespace DistributedHardware
} // namespace OHOS