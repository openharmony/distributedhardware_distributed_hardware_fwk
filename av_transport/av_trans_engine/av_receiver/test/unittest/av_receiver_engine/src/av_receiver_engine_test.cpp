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
    receiver->OnChannelEvent(transEvent);

    EXPECT_EQ(StateId::INITIALIZED, receiver->currentState_);
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
    EXPECT_EQ(StateId::INITIALIZED, receiver->currentState_);
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

    receiver->OnEvent(event);
    EXPECT_EQ(StateId::INITIALIZED, receiver->currentState_);

    receiver->receiverCallback_ = std::make_shared<ReceiverEngineCallback>();
    receiver->OnEvent(event);
    EXPECT_EQ(StateId::INITIALIZED, receiver->currentState_);
}
} // namespace DistributedHardware
} // namespace OHOS