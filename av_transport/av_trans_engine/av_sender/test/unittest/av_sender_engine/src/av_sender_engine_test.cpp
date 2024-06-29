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

#include "av_sender_engine_test.h"

#include "pipeline/factory/filter_factory.h"
#include "plugin_video_tags.h"
#include "distributed_hardware_fwk_kit.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware;
using namespace std;

using namespace OHOS::Media;
using namespace OHOS::Media::Plugin;
using namespace OHOS::Media::Pipeline;
using AVBuffer = OHOS::Media::Plugin::Buffer;

namespace OHOS {
namespace DistributedHardware {
const std::string FILTERNAME = "avsenderengine";

void AvSenderEngineTest::SetUp()
{
    AutoRegisterFilter<AVOutputFilter> registerOutputFilter(AVOUTPUT_NAME);
    AutoRegisterFilter<AVInputFilter> registerInputFilter(AVINPUT_NAME);
    AutoRegisterFilter<OHOS::Media::Pipeline::VideoEncoderFilter> registerEncoderFilter(VENCODER_NAME);
}

void AvSenderEngineTest::TearDown()
{
}

void AvSenderEngineTest::SetUpTestCase()
{
}

void AvSenderEngineTest::TearDownTestCase()
{
}

HWTEST_F(AvSenderEngineTest, Initialize_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->isInitialized_ = true;
    int32_t ret = sender->Initialize();
    std::string value264 = MIME_VIDEO_H264;
    sender->videoEncoder_ =
        FilterFactory::Instance().CreateFilterWithType<VideoEncoderFilter>(VENCODER_NAME, "videoencoder");
    sender->avInput_ =
        FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    sender->avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");
    sender->SetVideoCodecType(value264);
    std::string value = MIME_VIDEO_H265;
    sender->SetVideoCodecType(value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvSenderEngineTest, Initialize_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->avInput_ = nullptr;
    int32_t ret = sender->Initialize();
    EXPECT_EQ(ERR_DH_AVT_INIT_FAILED, ret);
}

HWTEST_F(AvSenderEngineTest, Initialize_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->sessionName_ = "";
    int32_t ret = sender->Initialize();
    sender->pipeline_ = nullptr;
    sender->SetEnginePause(ownerName);
    sender->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    sender->SetEnginePause(ownerName);
    EXPECT_EQ(ERR_DH_AVT_INIT_FAILED, ret);
}

HWTEST_F(AvSenderEngineTest, InitPipeline_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->pipeline_ = nullptr;
    sender->SetEngineResume(ownerName);
    sender->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    sender->SetEngineResume(ownerName);
    int32_t ret = sender->InitPipeline();
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM_VALUE, ret);
}

HWTEST_F(AvSenderEngineTest, InitPipeline_002, testing::ext::TestSize.Level1)
{
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(OWNER_NAME_D_MIC, peerDevId);
    int32_t ret = sender->InitPipeline();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvSenderEngineTest, CreateControlChannel_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    std::vector<std::string> dstDevIds;
    int32_t ret = sender->CreateControlChannel(dstDevIds, ChannelAttribute{TransStrategy::LOW_LATANCY_STRATEGY});
    EXPECT_EQ(ERR_DH_AVT_NULL_POINTER, ret);
}

HWTEST_F(AvSenderEngineTest, CreateControlChannel_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    std::vector<std::string> dstDevIds = {peerDevId};
    int32_t ret = sender->CreateControlChannel(dstDevIds, ChannelAttribute{TransStrategy::LOW_LATANCY_STRATEGY});
    EXPECT_EQ(ERR_DH_AVT_CREATE_CHANNEL_FAILED, ret);
}

HWTEST_F(AvSenderEngineTest, CreateControlChannel_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    std::vector<std::string> dstDevIds = {peerDevId};
    int32_t ret = sender->CreateControlChannel(dstDevIds, ChannelAttribute{TransStrategy::LOW_LATANCY_STRATEGY});
    EXPECT_EQ(ERR_DH_AVT_CREATE_CHANNEL_FAILED, ret);
}

HWTEST_F(AvSenderEngineTest, Start_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::IDLE;
    sender->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    int32_t ret = sender->Start();
    EXPECT_EQ(ERR_DH_AVT_START_FAILED, ret);
}

HWTEST_F(AvSenderEngineTest, Start_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::CH_CREATED;
    sender->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    int32_t ret = sender->Start();
    sender->Stop();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvSenderEngineTest, Start_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::CH_CREATED;
    sender->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    sender->dhFwkKit_ = std::make_shared<DistributedHardwareFwkKit>();
    int32_t ret = sender->Start();
    sender->Stop();
    EXPECT_EQ(ERR_DH_AVT_CREATE_CHANNEL_FAILED, ret);
}

HWTEST_F(AvSenderEngineTest, Stop_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::CH_CREATED;
    sender->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    sender->Start();
    int32_t ret = sender->Stop();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvSenderEngineTest, SetParameter_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    int32_t ret = sender->SetParameter(AVTransTag::INVALID, value);
    EXPECT_EQ(ERR_DH_AVT_SETUP_FAILED, ret);
}

HWTEST_F(AvSenderEngineTest, SetParameter_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    int32_t ret = sender->SetParameter(AVTransTag::INVALID, value);
    EXPECT_EQ(ERR_DH_AVT_SETUP_FAILED, ret);
}

HWTEST_F(AvSenderEngineTest, SetParameter_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    sender->avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");
    int32_t ret = sender->SetParameter(AVTransTag::INVALID, value);
    EXPECT_EQ(ERR_DH_AVT_SETUP_FAILED, ret);
}

HWTEST_F(AvSenderEngineTest, SetParameter_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "123";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    sender->avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");

    std::shared_ptr<OHOS::Media::Pipeline::PipelineCore> pipeline_ = nullptr;
    sender->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    int32_t ret = sender->SetParameter(AVTransTag::VIDEO_WIDTH, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvSenderEngineTest, SetParameter_005, testing::ext::TestSize.Level1)
{
    std::string ownerName = "ohos.dhardware.dscreen";
    std::string peerDevId = "pEid";
    std::string value = "123";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->InitPipeline();
    sender->RegRespFunMap();

    int32_t ret = sender->SetParameter(AVTransTag::VIDEO_WIDTH, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);

    ret = sender->SetParameter(AVTransTag::VIDEO_HEIGHT, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvSenderEngineTest, SetParameter_006, testing::ext::TestSize.Level1)
{
    std::string ownerName = "ohos.dhardware.dscreen";
    std::string peerDevId = "pEid";
    std::string value = "123";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->InitPipeline();
    sender->RegRespFunMap();

    int32_t ret = sender->SetParameter(AVTransTag::VIDEO_PIXEL_FORMAT, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);

    ret = sender->SetParameter(AVTransTag::VIDEO_FRAME_RATE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvSenderEngineTest, SetParameter_007, testing::ext::TestSize.Level1)
{
    std::string ownerName = "ohos.dhardware.dscreen";
    std::string peerDevId = "pEid";
    std::string value = "123";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->InitPipeline();
    sender->RegRespFunMap();

    int32_t ret = sender->SetParameter(AVTransTag::AUDIO_BIT_RATE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);

    ret = sender->SetParameter(AVTransTag::VIDEO_BIT_RATE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvSenderEngineTest, SetParameter_008, testing::ext::TestSize.Level1)
{
    std::string ownerName = "ohos.dhardware.dscreen";
    std::string peerDevId = "pEid";
    std::string value = "video/avc";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->InitPipeline();
    sender->RegRespFunMap();

    int32_t ret = sender->SetParameter(AVTransTag::VIDEO_CODEC_TYPE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);

    value = "video/hevc";
    ret = sender->SetParameter(AVTransTag::VIDEO_CODEC_TYPE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);

    value = "";
    ret = sender->SetParameter(AVTransTag::VIDEO_CODEC_TYPE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvSenderEngineTest, SetParameter_009, testing::ext::TestSize.Level1)
{
    std::string ownerName = "ohos.dhardware.dscreen";
    std::string peerDevId = "pEid";
    std::string value = "";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->InitPipeline();
    sender->RegRespFunMap();

    int32_t ret = sender->SetParameter(AVTransTag::AUDIO_CODEC_TYPE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvSenderEngineTest, SetParameter_010, testing::ext::TestSize.Level1)
{
    std::string ownerName = "ohos.dhardware.dscreen";
    std::string peerDevId = "pEid";
    std::string value = "video/hevc";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->InitPipeline();
    sender->RegRespFunMap();

    int32_t ret = sender->SetParameter(AVTransTag::VIDEO_CODEC_TYPE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvSenderEngineTest, SetParameter_011, testing::ext::TestSize.Level1)
{
    std::string ownerName = "ohos.dhardware.dscreen";
    std::string peerDevId = "pEid";
    std::string value = "500";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->InitPipeline();
    sender->RegRespFunMap();

    int32_t ret = sender->SetParameter(AVTransTag::AUDIO_CHANNEL_MASK, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);

    ret = sender->SetParameter(AVTransTag::AUDIO_SAMPLE_RATE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);

    ret = sender->SetParameter(AVTransTag::AUDIO_CHANNEL_LAYOUT, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);

    ret = sender->SetParameter(AVTransTag::AUDIO_SAMPLE_FORMAT, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);

    ret = sender->SetParameter(AVTransTag::AUDIO_FRAME_SIZE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);

    ret = sender->SetParameter(AVTransTag::SHARED_MEMORY_FD, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvSenderEngineTest, SetParameter_012, testing::ext::TestSize.Level1)
{
    std::string ownerName = "ohos.dhardware.dscreen";
    std::string peerDevId = "pEid";
    std::string value = "500";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->InitPipeline();
    sender->RegRespFunMap();

    int32_t ret = sender->SetParameter(AVTransTag::ENGINE_READY, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);

    ret = sender->SetParameter(AVTransTag::ENGINE_PAUSE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);

    ret = sender->SetParameter(AVTransTag::ENGINE_RESUME, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvSenderEngineTest, PushData_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    std::shared_ptr<AVTransBuffer> buffer = std::make_shared<AVTransBuffer>();
    sender->currentState_ = StateId::IDLE;
    int32_t ret = sender->PushData(buffer);
    EXPECT_EQ(ERR_DH_AVT_PUSH_DATA_FAILED, ret);
}

HWTEST_F(AvSenderEngineTest, PushData_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    std::shared_ptr<AVTransBuffer> buffer = std::make_shared<AVTransBuffer>();
    sender->currentState_ = StateId::IDLE;
    int32_t ret = sender->PushData(buffer);
    EXPECT_EQ(ERR_DH_AVT_PUSH_DATA_FAILED, ret);
}

HWTEST_F(AvSenderEngineTest, PushData_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    std::shared_ptr<AVTransBuffer> buffer = std::make_shared<AVTransBuffer>();
    sender->currentState_ = StateId::STARTED;
    int32_t ret = sender->PushData(buffer);
    EXPECT_EQ(ERR_DH_AVT_PUSH_DATA_FAILED, ret);
}

HWTEST_F(AvSenderEngineTest, PushData_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    std::shared_ptr<AVTransBuffer> buffer = std::make_shared<AVTransBuffer>();
    sender->currentState_ = StateId::STARTED;
    sender->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    int32_t ret = sender->PushData(buffer);
    EXPECT_EQ(ERR_DH_AVT_PUSH_DATA_FAILED, ret);
}

HWTEST_F(AvSenderEngineTest, PushData_005, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    std::shared_ptr<AVTransBuffer> buffer = nullptr;
    sender->currentState_ = StateId::STARTED;
    sender->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    int32_t ret = sender->PushData(buffer);
    EXPECT_EQ(ERR_DH_AVT_PUSH_DATA_FAILED, ret);
}

HWTEST_F(AvSenderEngineTest, PreparePipeline_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::STARTED;
    int32_t ret = sender->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_PREPARE_FAILED, ret);
}

HWTEST_F(AvSenderEngineTest, PreparePipeline_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::INITIALIZED;
    int32_t ret = sender->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_PREPARE_FAILED, ret);
}

HWTEST_F(AvSenderEngineTest, PreparePipeline_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::INITIALIZED;
    sender->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    int32_t ret = sender->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_PREPARE_FAILED, ret);
}

HWTEST_F(AvSenderEngineTest, PreparePipeline_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::INITIALIZED;
    sender->avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");
    int32_t ret = sender->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_PREPARE_FAILED, ret);
}

HWTEST_F(AvSenderEngineTest, PreparePipeline_005, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::INITIALIZED;
    sender->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    sender->avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");
    sender->pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();

    int32_t ret = sender->PreparePipeline(configParam);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvSenderEngineTest, StartDumpMediaData_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");

    bool ret = sender->StartDumpMediaData();
    EXPECT_EQ(true, ret);

    sender->avOutput_ = nullptr;
    ret = sender->StartDumpMediaData();
    EXPECT_EQ(false, ret);
}

HWTEST_F(AvSenderEngineTest, StopDumpMediaData_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");

    bool ret = sender->StopDumpMediaData();
    EXPECT_EQ(true, ret);

    sender->avOutput_ = nullptr;
    ret = sender->StopDumpMediaData();
    EXPECT_EQ(false, ret);
}

HWTEST_F(AvSenderEngineTest, ReStartDumpMediaData_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");

    bool ret = sender->ReStartDumpMediaData();
    EXPECT_EQ(true, ret);

    sender->avOutput_ = nullptr;
    ret = sender->ReStartDumpMediaData();
    EXPECT_EQ(false, ret);
}

HWTEST_F(AvSenderEngineTest, SendMessage_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_MIC;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    EventType type = EventType::EVENT_ADD_STREAM;
    sender->NotifyStreamChange(type);
    int32_t ret = sender->SendMessage(nullptr);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvSenderEngineTest, SendMessage_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_SPEAKER;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    EventType type = EventType::EVENT_ADD_STREAM;
    sender->NotifyStreamChange(type);
    std::shared_ptr<AVTransMessage> message = std::make_shared<AVTransMessage>();
    int32_t ret = sender->SendMessage(message);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvSenderEngineTest, SendMessage_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_VIRMODEM_MIC;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    EventType type = EventType::EVENT_ADD_STREAM;
    sender->NotifyStreamChange(type);
    int32_t ret = sender->SendMessage(nullptr);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvSenderEngineTest, SendMessage_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_VIRMODEM_SPEAKER;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    EventType type = EventType::EVENT_ADD_STREAM;
    sender->NotifyStreamChange(type);
    std::shared_ptr<AVTransMessage> message = std::make_shared<AVTransMessage>();
    int32_t ret = sender->SendMessage(message);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvSenderEngineTest, RegisterSenderCallback_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_SCREEN;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    EventType type = EventType::EVENT_ADD_STREAM;
    sender->NotifyStreamChange(type);
    std::shared_ptr<SenderEngineCallback> callback = std::make_shared<SenderEngineCallback>();
    int32_t ret = sender->RegisterSenderCallback(callback);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvSenderEngineTest, RegisterSenderCallback_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    std::shared_ptr<SenderEngineCallback> callback = nullptr;
    int32_t ret = sender->RegisterSenderCallback(callback);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvSenderEngineTest, OnChannelEvent_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    sender->senderCallback_ = nullptr;
    AVTransEvent transEvent;
    transEvent.content = "content";
    transEvent.type = EventType::EVENT_CHANNEL_OPENED;
    sender->OnChannelEvent(transEvent);
    sender->senderCallback_ = std::make_shared<SenderEngineCallback>();
    sender->OnChannelEvent(transEvent);

    EXPECT_EQ(StateId::CH_CREATED, sender->currentState_);
}

HWTEST_F(AvSenderEngineTest, OnChannelEvent_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    AVTransEvent transEvent;
    transEvent.content = "content";
    transEvent.type = EventType::EVENT_CHANNEL_OPEN_FAIL;
    sender->senderCallback_ = std::make_shared<SenderEngineCallback>();
    sender->OnChannelEvent(transEvent);

    EXPECT_EQ(StateId::INITIALIZED, sender->currentState_);
}

HWTEST_F(AvSenderEngineTest, OnChannelEvent_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    AVTransEvent event;
    event.content = "content";
    event.type = EventType::EVENT_CHANNEL_CLOSED;
    sender->senderCallback_ = std::make_shared<SenderEngineCallback>();
    sender->currentState_ = StateId::CH_CREATED;
    sender->OnChannelEvent(event);

    event.type = EventType::EVENT_DATA_RECEIVED;
    sender->OnChannelEvent(event);

    EXPECT_EQ(StateId::INITIALIZED, sender->currentState_);
}

HWTEST_F(AvSenderEngineTest, OnChannelEvent_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    AVTransEvent event;
    event.content = "content";
    event.type = EventType::EVENT_CHANNEL_CLOSED;
    sender->senderCallback_ = std::make_shared<SenderEngineCallback>();
    sender->currentState_ = StateId::CH_CREATED;
    sender->OnChannelEvent(event);
    sender->currentState_ = StateId::IDLE;
    sender->OnChannelEvent(event);
    sender->currentState_ = StateId::INITIALIZED;
    sender->OnChannelEvent(event);

    event.type = EventType::EVENT_ADD_STREAM;
    sender->OnChannelEvent(event);

    EXPECT_EQ(StateId::INITIALIZED, sender->currentState_);
}

HWTEST_F(AvSenderEngineTest, OnEvent_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);

    OHOS::Media::Event event;
    event.srcFilter = "filter";
    event.type = OHOS::Media::EventType::EVENT_READY;
    event.param = PluginEventType::EVENT_CHANNEL_OPEN_FAIL;
    sender->currentState_ = StateId::INITIALIZED;

    sender->OnEvent(event);
    EXPECT_EQ(StateId::INITIALIZED, sender->currentState_);

    sender->senderCallback_ = std::make_shared<SenderEngineCallback>();
    sender->OnEvent(event);
    EXPECT_EQ(StateId::INITIALIZED, sender->currentState_);
}
} // namespace DistributedHardware
} // namespace OHOS