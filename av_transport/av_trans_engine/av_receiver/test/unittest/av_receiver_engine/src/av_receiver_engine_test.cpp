/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "av_receiver_engine_test.h"

#include "pipeline/factory/filter_factory.h"
#include "plugin_video_tags.h"
#include "distributed_hardware_fwk_kit.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
const std::string FILTERNAME = "avreceivererengine";

void AvReceiverEngineTest::SetUp()
{
    AutoRegisterFilter<AVOutputFilter> registerOutputFilter(AVOUTPUT_NAME);
    AutoRegisterFilter<AVInputFilter> registerInputFilter(AVINPUT_NAME);
    AutoRegisterFilter<OHOS::Media::Pipeline::VideoEncoderFilter> registerEncoderFilter(VENCODER_NAME);
}

void AvReceiverEngineTest::TearDown()
{
}

void AvReceiverEngineTest::SetUpTestCase()
{
}

void AvReceiverEngineTest::TearDownTestCase()
{
}

HWTEST_F(AvReceiverEngineTest, Initialize_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->isInitialized_ = true;
    std::string value264 = MIME_VIDEO_H264;
    receiver->videoDecoder_ =
        FilterFactory::Instance().CreateFilterWithType<VideoDecoderFilter>(VDECODER_NAME, "videodecoder");
    receiver->avInput_ =
        FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    receiver->avOutput_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");
    receiver->SetVideoCodecType(value264);
    std::string value = MIME_VIDEO_H265;
    receiver->SetVideoCodecType(value);
    int32_t ret = receiver->Initialize();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvReceiverEngineTest, Initialize_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = nullptr;
    int32_t ret = receiver->Initialize();
    EXPECT_EQ(ERR_DH_AVT_INIT_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, Initialize_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->sessionName_ = "";
    int32_t ret = receiver->Initialize();
    EXPECT_EQ(ERR_DH_AVT_INIT_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, InitPipeline_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->pipeline_ = nullptr;
    receiver->SetEngineReady(ownerName);
    receiver->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    receiver->SetEngineReady(ownerName);
    int32_t ret = receiver->InitPipeline();
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM_VALUE, ret);
}

HWTEST_F(AvReceiverEngineTest, InitPipeline_002, testing::ext::TestSize.Level1)
{
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(OWNER_NAME_D_MIC, peerDevId);
    int32_t ret = receiver->InitPipeline();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvReceiverEngineTest, InitPipeline_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_SCREEN;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    int32_t ret = receiver->InitPipeline();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvReceiverEngineTest, CreateControlChannel_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    std::vector<std::string> dstDevIds;
    int32_t ret = receiver->CreateControlChannel(dstDevIds, ChannelAttribute{TransStrategy::LOW_LATANCY_STRATEGY});
    EXPECT_EQ(ERR_DH_AVT_NULL_POINTER, ret);
}

HWTEST_F(AvReceiverEngineTest, CreateControlChannel_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    std::vector<std::string> dstDevIds = {peerDevId};
    int32_t ret = receiver->CreateControlChannel(dstDevIds, ChannelAttribute{TransStrategy::LOW_LATANCY_STRATEGY});
    EXPECT_EQ(ERR_DH_AVT_CREATE_CHANNEL_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, CreateControlChannel_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    std::vector<std::string> dstDevIds = {peerDevId};
    int32_t ret = receiver->CreateControlChannel(dstDevIds, ChannelAttribute{TransStrategy::LOW_LATANCY_STRATEGY});
    EXPECT_EQ(ERR_DH_AVT_CREATE_CHANNEL_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, Start_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::IDLE;
    receiver->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    int32_t ret = receiver->Start();
    EXPECT_EQ(ERR_DH_AVT_START_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, Start_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::CH_CREATED;
    receiver->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    int32_t ret = receiver->Start();
    receiver->Stop();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvReceiverEngineTest, Start_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::CH_CREATED;
    receiver->pipeline_ = nullptr;
    int32_t ret = receiver->Start();
    receiver->Stop();
    EXPECT_EQ(ERR_DH_AVT_START_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, Stop_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::CH_CREATED;
    receiver->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    receiver->Start();
    int32_t ret = receiver->Stop();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvReceiverEngineTest, SetParameter_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    int32_t ret = receiver->SetParameter(AVTransTag::INVALID, value);
    EXPECT_EQ(ERR_DH_AVT_SETUP_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, SetParameter_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    int32_t ret = receiver->SetParameter(AVTransTag::INVALID, value);
    EXPECT_EQ(ERR_DH_AVT_SETUP_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, SetParameter_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    receiver->avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");
    int32_t ret = receiver->SetParameter(AVTransTag::INVALID, value);
    EXPECT_EQ(ERR_DH_AVT_SETUP_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, SetParameter_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "123";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    receiver->avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");

    std::shared_ptr<OHOS::Media::Pipeline::PipelineCore> pipeline_ = nullptr;
    receiver->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    int32_t ret = receiver->SetParameter(AVTransTag::VIDEO_WIDTH, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvReceiverEngineTest, SetParameter_005, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "123";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    receiver->avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");

    std::shared_ptr<OHOS::Media::Pipeline::PipelineCore> pipeline_ = nullptr;
    receiver->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    int32_t ret = receiver->SetParameter(AVTransTag::VIDEO_WIDTH, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvReceiverEngineTest, SetParameter_006, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "123";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    receiver->avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");

    std::shared_ptr<OHOS::Media::Pipeline::PipelineCore> pipeline_ = nullptr;
    receiver->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    int32_t ret = receiver->SetParameter(AVTransTag::VIDEO_HEIGHT, value);
    ret = receiver->SetParameter(AVTransTag::VIDEO_FRAME_RATE, value);
    ret = receiver->SetParameter(AVTransTag::AUDIO_BIT_RATE, value);
    ret = receiver->SetParameter(AVTransTag::VIDEO_BIT_RATE, value);
    ret = receiver->SetParameter(AVTransTag::VIDEO_CODEC_TYPE, value);
    ret = receiver->SetParameter(AVTransTag::AUDIO_CODEC_TYPE, value);
    ret = receiver->SetParameter(AVTransTag::AUDIO_CHANNEL_MASK, value);
    ret = receiver->SetParameter(AVTransTag::AUDIO_SAMPLE_RATE, value);
    ret = receiver->SetParameter(AVTransTag::AUDIO_CHANNEL_LAYOUT, value);
    ret = receiver->SetParameter(AVTransTag::AUDIO_SAMPLE_FORMAT, value);
    ret = receiver->SetParameter(AVTransTag::AUDIO_FRAME_SIZE, value);
    ret = receiver->SetParameter(AVTransTag::TIME_SYNC_RESULT, value);
    ret = receiver->SetParameter(AVTransTag::START_AV_SYNC, value);
    ret = receiver->SetParameter(AVTransTag::STOP_AV_SYNC, value);
    ret = receiver->SetParameter(AVTransTag::SHARED_MEMORY_FD, value);
    ret = receiver->SetParameter(AVTransTag::ENGINE_READY, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
    receiver->SetParameterInner(AVTransTag::AUDIO_RENDER_FLAGS, value);
    ret = receiver->SetParameter(AVTransTag::AUDIO_RENDER_FLAGS, value);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvReceiverEngineTest, SetParameter_007, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "123";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    receiver->avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");

    std::shared_ptr<OHOS::Media::Pipeline::PipelineCore> pipeline_ = nullptr;
    receiver->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    int32_t ret = receiver->SetParameter(AVTransTag::VIDEO_WIDTH, value);
    ret = receiver->SetParameter(AVTransTag::VIDEO_HEIGHT, value);
    ret = receiver->SetParameter(AVTransTag::VIDEO_FRAME_RATE, value);
    ret = receiver->SetParameter(AVTransTag::AUDIO_BIT_RATE, value);
    ret = receiver->SetParameter(AVTransTag::VIDEO_BIT_RATE, value);
    ret = receiver->SetParameter(AVTransTag::VIDEO_CODEC_TYPE, value);
    ret = receiver->SetParameter(AVTransTag::AUDIO_CODEC_TYPE, value);
    ret = receiver->SetParameter(AVTransTag::AUDIO_CHANNEL_MASK, value);
    ret = receiver->SetParameter(AVTransTag::AUDIO_SAMPLE_RATE, value);
    ret = receiver->SetParameter(AVTransTag::AUDIO_CHANNEL_LAYOUT, value);
    ret = receiver->SetParameter(AVTransTag::AUDIO_SAMPLE_FORMAT, value);
    ret = receiver->SetParameter(AVTransTag::AUDIO_FRAME_SIZE, value);
    ret = receiver->SetParameter(AVTransTag::TIME_SYNC_RESULT, value);
    ret = receiver->SetParameter(AVTransTag::START_AV_SYNC, value);
    ret = receiver->SetParameter(AVTransTag::STOP_AV_SYNC, value);
    ret = receiver->SetParameter(AVTransTag::SHARED_MEMORY_FD, value);
    ret = receiver->SetParameter(AVTransTag::ENGINE_READY, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
    receiver->SetParameterInner(AVTransTag::AUDIO_RENDER_FLAGS, value);
    ret = receiver->SetParameter(AVTransTag::AUDIO_RENDER_FLAGS, value);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvReceiverEngineTest, SetParameter_008, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "123";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    receiver->avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");
    std::shared_ptr<OHOS::Media::Pipeline::PipelineCore> pipeline_ = nullptr;
    receiver->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    int32_t ret = receiver->SetParameter(AVTransTag::VIDEO_HEIGHT, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
    ret = receiver->SetParameter(AVTransTag::VIDEO_FRAME_RATE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
    ret = receiver->SetParameter(AVTransTag::AUDIO_BIT_RATE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
    ret = receiver->SetParameter(AVTransTag::VIDEO_BIT_RATE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
    ret = receiver->SetParameter(AVTransTag::VIDEO_CODEC_TYPE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
    ret = receiver->SetParameter(AVTransTag::INVALID, value);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvReceiverEngineTest, PreparePipeline_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::STARTED;
    int32_t ret = receiver->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_PREPARE_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, PreparePipeline_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::INITIALIZED;
    int32_t ret = receiver->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_PREPARE_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, PreparePipeline_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::INITIALIZED;
    receiver->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    int32_t ret = receiver->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_PREPARE_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, PreparePipeline_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::INITIALIZED;
    std::shared_ptr<AVOutputFilter> avOutput_ = nullptr;
    avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");
    int32_t ret = receiver->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_PREPARE_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, PreparePipeline_005, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_MIC;
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    std::shared_ptr<AVInputFilter> avInput = nullptr;
    avInput = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    receiver->currentState_ = StateId::INITIALIZED;
    std::shared_ptr<AVOutputFilter> avOutput = nullptr;
    avOutput = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");
    receiver->avInput_ = avInput;
    receiver->avOutput_ = avOutput;
    receiver->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    int32_t ret = receiver->PreparePipeline(configParam);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvReceiverEngineTest, PreparePipeline_006, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_MIC;
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    std::shared_ptr<AVInputFilter> avInput = nullptr;
    avInput = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    receiver->currentState_ = StateId::INITIALIZED;
    std::shared_ptr<AVOutputFilter> avOutput = nullptr;
    avOutput = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");
    receiver->avInput_ = avInput;
    receiver->avOutput_ = avOutput;
    receiver->pipeline_ = nullptr;
    int32_t ret = receiver->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_SET_PARAM_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, SendMessage_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    int32_t ret = receiver->SendMessage(nullptr);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvReceiverEngineTest, SendMessage_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    auto avMessage = std::make_shared<AVTransMessage>((uint32_t)AVTransTag::START_AV_SYNC,
        ownerName, peerDevId);

    int32_t ret = receiver->SendMessage(avMessage);
    EXPECT_EQ(ERR_DH_AVT_SEND_DATA_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, RegisterReceiverCallback_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    std::shared_ptr<ReceiverEngineCallback> callback = nullptr;
    int32_t ret = receiver->RegisterReceiverCallback(callback);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvReceiverEngineTest, RegisterReceiverCallback_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    std::shared_ptr<ReceiverEngineCallback> callback = std::make_shared<ReceiverEngineCallback>();
    int32_t ret = receiver->RegisterReceiverCallback(callback);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvReceiverEngineTest, OnChannelEvent_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->receiverCallback_ = nullptr;
    AVTransEvent transEvent;
    transEvent.content = "content";
    transEvent.type = EventType::EVENT_CHANNEL_OPENED;
    receiver->OnChannelEvent(transEvent);
    receiver->receiverCallback_ = std::make_shared<ReceiverEngineCallback>();
    receiver->OnChannelEvent(transEvent);

    EXPECT_EQ(StateId::CH_CREATED, receiver->currentState_);
}

HWTEST_F(AvReceiverEngineTest, OnChannelEvent_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    AVTransEvent transEvent;
    transEvent.content = "content";
    transEvent.type = EventType::EVENT_CHANNEL_OPEN_FAIL;
    receiver->receiverCallback_ = std::make_shared<ReceiverEngineCallback>();
    EXPECT_NO_FATAL_FAILURE(receiver->OnChannelEvent(transEvent));
}

HWTEST_F(AvReceiverEngineTest, OnChannelEvent_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
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
    receiver->OnChannelEvent(event);
    EXPECT_NO_FATAL_FAILURE(receiver->OnChannelEvent(event));
}

HWTEST_F(AvReceiverEngineTest, OnChannelEvent_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    AVTransEvent event;
    event.content = "content";
    event.type = EventType::EVENT_REMOVE_STREAM;
    EXPECT_NO_FATAL_FAILURE(receiver->OnChannelEvent(event));
}

HWTEST_F(AvReceiverEngineTest, Release_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->RegRespFunMap();
    std::string value = "50";
    std::shared_ptr<AVInputFilter> avInput = nullptr;
    avInput = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    std::shared_ptr<AVOutputFilter> avOutput = nullptr;
    avOutput = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");
    receiver->avInput_ = avInput;
    receiver->avOutput_ = avOutput;
    receiver->SetVideoWidth(value);
    receiver->SetVideoHeight(value);
    receiver->SetVideoFrameRate(value);
    receiver->SetAudioBitRate(value);
    receiver->SetVideoBitRate(value);
    std::string codecType = MIME_VIDEO_H264;
    receiver->SetVideoCodecType(codecType);
    codecType = MIME_VIDEO_H265;
    receiver->SetVideoCodecType(MIME_VIDEO_RAW);
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

HWTEST_F(AvReceiverEngineTest, Release_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->RegRespFunMap();
    std::string value = "50";
    std::shared_ptr<AVInputFilter> avInput = nullptr;
    avInput = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    std::shared_ptr<AVOutputFilter> avOutput = nullptr;
    avOutput = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");
    receiver->avInput_ = avInput;
    receiver->avOutput_ = avOutput;
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
    receiver->dhFwkKit_ = nullptr;
    int32_t ret = receiver->Release();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvReceiverEngineTest, Release_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->RegRespFunMap();
    std::string value = "50";
    receiver->avInput_ = nullptr;
    receiver->avOutput_ = nullptr;
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

HWTEST_F(AvReceiverEngineTest, StartDumpMediaData_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = nullptr;
    bool ret = receiver->StartDumpMediaData();
    EXPECT_EQ(false, ret);
}

HWTEST_F(AvReceiverEngineTest, StartDumpMediaData_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    std::shared_ptr<AVInputFilter> avInput = nullptr;
    avInput = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    receiver->avInput_ = avInput;
    bool ret = receiver->StartDumpMediaData();
    EXPECT_EQ(true, ret);
}

HWTEST_F(AvReceiverEngineTest, StopDumpMediaData_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = nullptr;
    bool ret = receiver->StopDumpMediaData();
    EXPECT_EQ(false, ret);
}

HWTEST_F(AvReceiverEngineTest, StopDumpMediaData_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    std::shared_ptr<AVInputFilter> avInput = nullptr;
    avInput = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    receiver->avInput_ = avInput;
    bool ret = receiver->StopDumpMediaData();
    EXPECT_EQ(true, ret);
}

HWTEST_F(AvReceiverEngineTest, ReStartDumpMediaData_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->avInput_ = nullptr;
    bool ret = receiver->ReStartDumpMediaData();
    EXPECT_EQ(false, ret);
}

HWTEST_F(AvReceiverEngineTest, ReStartDumpMediaData_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    std::shared_ptr<AVInputFilter> avInput = nullptr;
    avInput = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    receiver->avInput_ = avInput;
    bool ret = receiver->ReStartDumpMediaData();
    EXPECT_EQ(true, ret);
}

HWTEST_F(AvReceiverEngineTest, HandleOutputBuffer_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::INITIALIZED;
    std::shared_ptr<AVBuffer> buffer = std::make_shared<AVBuffer>();
    receiver->receiverCallback_ = std::make_shared<ReceiverEngineCallback>();
    int32_t ret = receiver->HandleOutputBuffer(buffer);
    EXPECT_EQ(ERR_DH_AVT_OUTPUT_DATA_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, HandleOutputBuffer_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::INITIALIZED;
    std::shared_ptr<AVBuffer> buffer = nullptr;
    receiver->receiverCallback_ = std::make_shared<ReceiverEngineCallback>();
    int32_t ret = receiver->HandleOutputBuffer(buffer);
    EXPECT_EQ(ERR_DH_AVT_OUTPUT_DATA_FAILED, ret);
    buffer = std::make_shared<AVBuffer>();
    std::shared_ptr<IAVReceiverEngineCallback> receiverCallback = receiver->receiverCallback_;
    receiver->receiverCallback_ = nullptr;
    ret = receiver->HandleOutputBuffer(buffer);
    receiver->receiverCallback_ = receiverCallback;
    EXPECT_EQ(ERR_DH_AVT_OUTPUT_DATA_FAILED, ret);
}

HWTEST_F(AvReceiverEngineTest, OnEvent_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);

    OHOS::Media::Event event;
    event.srcFilter = "filter";
    event.type = OHOS::Media::EventType::EVENT_READY;
    event.param = PluginEventType::EVENT_CHANNEL_OPEN_FAIL;
    receiver->currentState_ = StateId::INITIALIZED;

    EXPECT_NO_FATAL_FAILURE(receiver->OnEvent(event));

    receiver->receiverCallback_ = std::make_shared<ReceiverEngineCallback>();
    EXPECT_NO_FATAL_FAILURE(receiver->OnEvent(event));
}

HWTEST_F(AvReceiverEngineTest, OnEvent_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    OHOS::Media::Event event;
    event.srcFilter = "filter";
    event.type = OHOS::Media::EventType::EVENT_BUFFER_PROGRESS;
    std::shared_ptr<AVBuffer> buffer = nullptr;
    event.param = buffer;
    receiver->currentState_ = StateId::INITIALIZED;
    EXPECT_NO_FATAL_FAILURE(receiver->OnEvent(event));
    event.param = std::make_shared<AVBuffer>();
    EXPECT_NO_FATAL_FAILURE(receiver->OnEvent(event));
    receiver->receiverCallback_ = std::make_shared<ReceiverEngineCallback>();
    event.type = OHOS::Media::EventType::EVENT_PLUGIN_EVENT;
    event.param = PluginEvent{PluginEventType::AUDIO_INTERRUPT, 0, "aaa"};
    EXPECT_NO_FATAL_FAILURE(receiver->OnEvent(event));
}

HWTEST_F(AvReceiverEngineTest, SetParameterInner_001, testing::ext::TestSize.Level1)
{
    auto receiver = std::make_shared<AVReceiverEngine>("001", "pEid");
    receiver->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    receiver->avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");
    receiver->SetParameterInner(AVTransTag::VIDEO_CODEC_TYPE, MIME_VIDEO_H264);
    EXPECT_EQ(receiver->avInput_->paramsMap_.find(Plugin::Tag::MIME) != receiver->avInput_->paramsMap_.end(), true);
    receiver->SetParameterInner(AVTransTag::AUDIO_CODEC_TYPE, MIME_VIDEO_H264);
    EXPECT_EQ(receiver->avInput_->paramsMap_.find(Plugin::Tag::MIME) != receiver->avInput_->paramsMap_.end(), true);
    receiver->SetParameterInner(AVTransTag::AUDIO_CHANNEL_MASK, "50");
    bool ret = receiver->avInput_->paramsMap_.find(Plugin::Tag::AUDIO_CHANNELS) !=
        receiver->avInput_->paramsMap_.end();
    EXPECT_EQ(ret, true);
    receiver->SetParameterInner(AVTransTag::AUDIO_SAMPLE_RATE, "50");
    ret = receiver->avInput_->paramsMap_.find(Plugin::Tag::AUDIO_SAMPLE_RATE) != receiver->avInput_->paramsMap_.end();
    EXPECT_EQ(ret, true);
    receiver->SetParameterInner(AVTransTag::AUDIO_CHANNEL_LAYOUT, "50");
    ret = receiver->avInput_->paramsMap_.find(Plugin::Tag::AUDIO_CHANNEL_LAYOUT) !=
        receiver->avInput_->paramsMap_.end();
    EXPECT_EQ(ret, true);
    receiver->SetParameterInner(AVTransTag::AUDIO_SAMPLE_FORMAT, "50");
    ret = receiver->avInput_->paramsMap_.find(Plugin::Tag::AUDIO_SAMPLE_FORMAT) !=
        receiver->avInput_->paramsMap_.end();
    EXPECT_EQ(ret, true);
    receiver->SetParameterInner(AVTransTag::AUDIO_FRAME_SIZE, "50");
    ret = receiver->avInput_->paramsMap_.find(Plugin::Tag::AUDIO_SAMPLE_PER_FRAME) !=
        receiver->avInput_->paramsMap_.end();
    EXPECT_EQ(ret, true);
}

HWTEST_F(AvReceiverEngineTest, SetParameterInner_002, testing::ext::TestSize.Level1)
{
    auto receiver = std::make_shared<AVReceiverEngine>("001", "pEid");
    receiver->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    receiver->avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");
    receiver->SetParameterInner(AVTransTag::TIME_SYNC_RESULT, MIME_VIDEO_H264);
    bool ret = receiver->avOutput_->paramsMap_.find(Plugin::Tag::USER_TIME_SYNC_RESULT) !=
        receiver->avOutput_->paramsMap_.end();
    EXPECT_EQ(ret, true);
    receiver->SetParameterInner(AVTransTag::START_AV_SYNC, MIME_VIDEO_H264);
    ret = receiver->avOutput_->paramsMap_.find(Plugin::Tag::USER_AV_SYNC_GROUP_INFO) !=
        receiver->avOutput_->paramsMap_.end();
    EXPECT_EQ(ret, true);
    receiver->SetParameterInner(AVTransTag::STOP_AV_SYNC, MIME_VIDEO_H264);
    ret = receiver->avOutput_->paramsMap_.find(Plugin::Tag::USER_AV_SYNC_GROUP_INFO) !=
        receiver->avOutput_->paramsMap_.end();
    EXPECT_EQ(ret, true);
    receiver->SetParameterInner(AVTransTag::SHARED_MEMORY_FD, MIME_VIDEO_H264);
    ret = receiver->avOutput_->paramsMap_.find(Plugin::Tag::USER_SHARED_MEMORY_FD) !=
        receiver->avOutput_->paramsMap_.end();
    EXPECT_EQ(ret, true);
    receiver->currentState_ = StateId::INITIALIZED;
    receiver->SetParameterInner(AVTransTag::ENGINE_READY, MIME_VIDEO_H264);
    ret = receiver->avInput_->paramsMap_.find(Plugin::Tag::MEDIA_TYPE) != receiver->avInput_->paramsMap_.end();
    EXPECT_EQ(ret, true);
    receiver->SetParameterInner(AVTransTag::INVALID, MIME_VIDEO_H264);
    ret = receiver->avInput_->paramsMap_.find(Plugin::Tag::INVALID) == receiver->avInput_->paramsMap_.end();
    EXPECT_EQ(ret, true);
}

HWTEST_F(AvReceiverEngineTest, OnStreamReceived_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName, peerDevId);
    receiver->currentState_ = StateId::INITIALIZED;
    StreamData *data = nullptr;
    StreamData *ext = nullptr;
    EXPECT_NO_FATAL_FAILURE(receiver->OnStreamReceived(data, ext));
}
} // namespace DistributedHardware
} // namespace OHOS