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

#include "av_audio_sender_engine_test.h"

#include "distributed_hardware_fwk_kit.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware;
using namespace std;

using namespace OHOS::Media;

namespace OHOS {
namespace DistributedHardware {
const std::string FILTERNAME = "avsenderengine";

void AvAudioSenderEngineTest::SetUp()
{
}

void AvAudioSenderEngineTest::TearDown()
{
}

void AvAudioSenderEngineTest::SetUpTestCase()
{
}

void AvAudioSenderEngineTest::TearDownTestCase()
{
}

HWTEST_F(AvAudioSenderEngineTest, Initialize_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->isInitialized_ = true;
    int32_t ret = sender->Initialize();
    std::string value264 = MIME_VIDEO_H264;
    sender->SetVideoCodecType(value264);
    std::string value = MIME_VIDEO_H265;
    sender->SetVideoCodecType(value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioSenderEngineTest, Initialize_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->avInput_ = nullptr;
    int32_t ret = sender->Initialize();
    EXPECT_EQ(ERR_DH_AVT_INIT_FAILED, ret);
}

HWTEST_F(AvAudioSenderEngineTest, Initialize_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->sessionName_ = "";
    int32_t ret = sender->Initialize();
    sender->pipeline_ = nullptr;
    sender->SetEnginePause(ownerName);
    sender->pipeline_ = std::make_shared<Pipeline::Pipeline>();
    sender->SetEnginePause(ownerName);
    EXPECT_EQ(ERR_DH_AVT_INIT_FAILED, ret);
}

HWTEST_F(AvAudioSenderEngineTest, InitPipeline_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->pipeline_ = nullptr;
    sender->SetEngineResume(ownerName);
    sender->pipeline_ = std::make_shared<Pipeline::Pipeline>();
    sender->SetEngineResume(ownerName);
    int32_t ret = sender->InitPipeline();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioSenderEngineTest, InitPipeline_002, testing::ext::TestSize.Level1)
{
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(OWNER_NAME_D_MIC, peerDevId);
    int32_t ret = sender->InitPipeline();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioSenderEngineTest, CreateControlChannel_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    std::vector<std::string> dstDevIds;
    int32_t ret = sender->CreateControlChannel(dstDevIds, ChannelAttribute{TransStrategy::LOW_LATANCY_STRATEGY});
    EXPECT_EQ(ERR_DH_AVT_NULL_POINTER, ret);
}

HWTEST_F(AvAudioSenderEngineTest, CreateControlChannel_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    std::vector<std::string> dstDevIds = {peerDevId};
    int32_t ret = sender->CreateControlChannel(dstDevIds, ChannelAttribute{TransStrategy::LOW_LATANCY_STRATEGY});
    EXPECT_EQ(ERR_DH_AVT_CREATE_CHANNEL_FAILED, ret);
}

HWTEST_F(AvAudioSenderEngineTest, CreateControlChannel_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    std::vector<std::string> dstDevIds = {peerDevId};
    int32_t ret = sender->CreateControlChannel(dstDevIds, ChannelAttribute{TransStrategy::LOW_LATANCY_STRATEGY});
    EXPECT_EQ(ERR_DH_AVT_CREATE_CHANNEL_FAILED, ret);
}

HWTEST_F(AvAudioSenderEngineTest, Start_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::IDLE;
    sender->pipeline_ = std::make_shared<Pipeline::Pipeline>();
    int32_t ret = sender->Start();
    EXPECT_EQ(ERR_DH_AVT_START_FAILED, ret);
}

HWTEST_F(AvAudioSenderEngineTest, Start_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::CH_CREATED;
    sender->pipeline_ = std::make_shared<Pipeline::Pipeline>();
    sender->dhFwkKit_ = std::make_shared<DistributedHardwareFwkKit>();
    int32_t ret = sender->Start();
    sender->Stop();
    EXPECT_EQ(ERR_DH_AVT_CREATE_CHANNEL_FAILED, ret);
}

HWTEST_F(AvAudioSenderEngineTest, Start_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::CH_CREATED;
    sender->pipeline_ = std::make_shared<Pipeline::Pipeline>();
    sender->dhFwkKit_ = std::make_shared<DistributedHardwareFwkKit>();
    int32_t ret = sender->Start();
    sender->Stop();
    EXPECT_EQ(ERR_DH_AVT_CREATE_CHANNEL_FAILED, ret);
}

HWTEST_F(AvAudioSenderEngineTest, Start_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::IDLE;
    sender->pipeline_ = nullptr;
    int32_t ret = sender->Start();
    sender->Stop();
    EXPECT_EQ(ERR_DH_AVT_START_FAILED, ret);
}

HWTEST_F(AvAudioSenderEngineTest, Stop_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::CH_CREATED;
    sender->pipeline_ = std::make_shared<Pipeline::Pipeline>();
    sender->Start();
    int32_t ret = sender->Stop();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioSenderEngineTest, Stop_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::CH_CREATED;
    sender->pipeline_ = nullptr;
    sender->Start();
    int32_t ret = sender->Stop();
    EXPECT_EQ(ERR_DH_AVT_STOP_FAILED, ret);
}

HWTEST_F(AvAudioSenderEngineTest, SetParameter_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    int32_t ret = sender->SetParameter(AVTransTag::INVALID, value);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvAudioSenderEngineTest, SetParameter_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    int32_t ret = sender->SetParameter(AVTransTag::INVALID, value);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvAudioSenderEngineTest, SetParameter_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    int32_t ret = sender->SetParameter(AVTransTag::INVALID, value);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvAudioSenderEngineTest, SetParameter_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "123";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);

    std::shared_ptr<Pipeline::Pipeline> pipeline_ = nullptr;
    sender->pipeline_ = std::make_shared<Pipeline::Pipeline>();
    int32_t ret = sender->SetParameter(AVTransTag::VIDEO_WIDTH, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioSenderEngineTest, SetParameter_005, testing::ext::TestSize.Level1)
{
    std::string ownerName = "ohos.dhardware.dscreen";
    std::string peerDevId = "pEid";
    std::string value = "123";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->InitPipeline();
    sender->RegRespFunMap();

    int32_t ret = sender->SetParameter(AVTransTag::VIDEO_WIDTH, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);

    ret = sender->SetParameter(AVTransTag::VIDEO_HEIGHT, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioSenderEngineTest, SetParameter_006, testing::ext::TestSize.Level1)
{
    std::string ownerName = "ohos.dhardware.dscreen";
    std::string peerDevId = "pEid";
    std::string value = "123";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->InitPipeline();
    sender->RegRespFunMap();

    int32_t ret = sender->SetParameter(AVTransTag::VIDEO_PIXEL_FORMAT, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);

    ret = sender->SetParameter(AVTransTag::VIDEO_FRAME_RATE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioSenderEngineTest, SetParameter_007, testing::ext::TestSize.Level1)
{
    std::string ownerName = "ohos.dhardware.dscreen";
    std::string peerDevId = "pEid";
    std::string value = "123";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->InitPipeline();
    sender->RegRespFunMap();

    int32_t ret = sender->SetParameter(AVTransTag::AUDIO_BIT_RATE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);

    ret = sender->SetParameter(AVTransTag::VIDEO_BIT_RATE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioSenderEngineTest, SetParameter_008, testing::ext::TestSize.Level1)
{
    std::string ownerName = "ohos.dhardware.dscreen";
    std::string peerDevId = "pEid";
    std::string value = "video/avc";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
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

HWTEST_F(AvAudioSenderEngineTest, SetParameter_009, testing::ext::TestSize.Level1)
{
    std::string ownerName = "ohos.dhardware.dscreen";
    std::string peerDevId = "pEid";
    std::string value = "12 32";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->InitPipeline();
    sender->RegRespFunMap();

    int32_t ret = sender->SetParameter(AVTransTag::AUDIO_CODEC_TYPE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioSenderEngineTest, SetParameter_010, testing::ext::TestSize.Level1)
{
    std::string ownerName = "ohos.dhardware.dscreen";
    std::string peerDevId = "pEid";
    std::string value = "video/hevc";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->InitPipeline();
    sender->RegRespFunMap();

    int32_t ret = sender->SetParameter(AVTransTag::VIDEO_CODEC_TYPE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioSenderEngineTest, SetParameter_011, testing::ext::TestSize.Level1)
{
    std::string ownerName = "ohos.dhardware.dscreen";
    std::string peerDevId = "pEid";
    std::string value = "500";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
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

HWTEST_F(AvAudioSenderEngineTest, SetParameter_012, testing::ext::TestSize.Level1)
{
    std::string ownerName = "ohos.dhardware.dscreen";
    std::string peerDevId = "pEid";
    std::string value = "500";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->InitPipeline();
    sender->RegRespFunMap();

    int32_t ret = sender->SetParameter(AVTransTag::ENGINE_READY, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);

    ret = sender->SetParameter(AVTransTag::ENGINE_PAUSE, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);

    ret = sender->SetParameter(AVTransTag::ENGINE_RESUME, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioSenderEngineTest, SetParameter_013, testing::ext::TestSize.Level1)
{
    std::string ownerName = "ohos.dhardware.dscreen";
    std::string peerDevId = "pEid";
    std::string value = "500";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->InitPipeline();
    sender->RegRespFunMap();

    int32_t ret = sender->SetParameter(AVTransTag::FRAME_NUMBER, value);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvAudioSenderEngineTest, SetParameter_014, testing::ext::TestSize.Level1)
{
    std::string ownerName = "ohos.dhardware.dscreen";
    std::string peerDevId = "pEid";
    std::string value = "500";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->meta_ = nullptr;
    sender->SetParameter(AVTransTag::VIDEO_WIDTH, "H264");
    sender->SetParameter(AVTransTag::VIDEO_HEIGHT, "1");
    sender->SetParameter(AVTransTag::VIDEO_FRAME_RATE, "2");
    sender->SetParameter(AVTransTag::AUDIO_BIT_RATE, "3");
    sender->SetParameter(AVTransTag::VIDEO_FRAME_RATE, "4");
    sender->SetParameter(AVTransTag::VIDEO_CODEC_TYPE, "5");
    sender->SetParameter(AVTransTag::AUDIO_CODEC_TYPE, "6");
    sender->SetParameter(AVTransTag::AUDIO_CHANNEL_MASK, "7");
    sender->SetParameter(AVTransTag::AUDIO_SAMPLE_RATE, "8");
    sender->SetParameter(AVTransTag::AUDIO_CHANNEL_LAYOUT, "9");
    sender->SetParameter(AVTransTag::AUDIO_SAMPLE_FORMAT, "10");
    sender->SetParameter(AVTransTag::AUDIO_FRAME_SIZE, "11");
    sender->SetParameter(AVTransTag::TIME_SYNC_RESULT, "12");
    sender->SetParameter(AVTransTag::START_AV_SYNC, "13");
    sender->SetParameter(AVTransTag::STOP_AV_SYNC, "14");
    sender->SetParameter(AVTransTag::SHARED_MEMORY_FD, "15");
    EXPECT_NO_FATAL_FAILURE(sender->SetParameter(AVTransTag::ENGINE_READY, "16"));
}

HWTEST_F(AvAudioSenderEngineTest, SetParameterInner_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "ohos.dhardware.dscreen";
    std::string peerDevId = "pEid";
    std::string value = "500";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->meta_ = nullptr;
    sender->SetVideoPixelFormat("10");
    sender->SetVideoBitRate("11");
    EXPECT_NO_FATAL_FAILURE(sender->SetParameterInner(AVTransTag::INVALID, "H264"));
}

HWTEST_F(AvAudioSenderEngineTest, PushData_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    std::shared_ptr<AVTransBuffer> buffer = std::make_shared<AVTransBuffer>();
    sender->currentState_ = StateId::IDLE;
    int32_t ret = sender->PushData(buffer);
    EXPECT_EQ(ERR_DH_AVT_PUSH_DATA_FAILED, ret);
}

HWTEST_F(AvAudioSenderEngineTest, PushData_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    std::shared_ptr<AVTransBuffer> buffer = std::make_shared<AVTransBuffer>();
    sender->currentState_ = StateId::IDLE;
    int32_t ret = sender->PushData(buffer);
    EXPECT_EQ(ERR_DH_AVT_PUSH_DATA_FAILED, ret);
}

HWTEST_F(AvAudioSenderEngineTest, PushData_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    std::shared_ptr<AVTransBuffer> buffer = std::make_shared<AVTransBuffer>();
    sender->currentState_ = StateId::STARTED;
    int32_t ret = sender->PushData(buffer);
    EXPECT_EQ(ERR_DH_AVT_PUSH_DATA_FAILED, ret);
}

HWTEST_F(AvAudioSenderEngineTest, PushData_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    std::shared_ptr<AVTransBuffer> buffer = std::make_shared<AVTransBuffer>();
    sender->currentState_ = StateId::STARTED;
    int32_t ret = sender->PushData(buffer);
    EXPECT_EQ(ERR_DH_AVT_PUSH_DATA_FAILED, ret);
}

HWTEST_F(AvAudioSenderEngineTest, PushData_005, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    std::shared_ptr<AVTransBuffer> buffer = nullptr;
    sender->currentState_ = StateId::STARTED;
    int32_t ret = sender->PushData(buffer);
    EXPECT_EQ(ERR_DH_AVT_PUSH_DATA_FAILED, ret);
}

HWTEST_F(AvAudioSenderEngineTest, PushData_006, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    std::shared_ptr<AVTransBuffer> buffer = std::make_shared<AVTransBuffer>();
    sender->currentState_ = StateId::STARTED;
    sender->avInput_ = std::make_shared<Pipeline::AVTransAudioInputFilter>("builtin.avtrans.audio.input",
        Pipeline::FilterType::FILTERTYPE_SOURCE);
    int32_t ret = sender->PushData(buffer);
    EXPECT_EQ(ERR_DH_AVT_PUSH_DATA_FAILED, ret);
}

HWTEST_F(AvAudioSenderEngineTest, Prepare_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string value = "value";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->avInput_ = std::make_shared<Pipeline::AVTransAudioInputFilter>("builtin.avtrans.audio.input",
        Pipeline::FilterType::FILTERTYPE_SOURCE);
    EXPECT_EQ(sender->Prepare(), Status::ERROR_INVALID_OPERATION);

    sender->avInput_ = std::make_shared<Pipeline::AVTransAudioInputFilter>("builtin.avtrans.audio.input",
        Pipeline::FilterType::FILTERTYPE_SOURCE);
    sender->pipeline_ = nullptr;
    EXPECT_EQ(sender->Prepare(), Status::ERROR_INVALID_OPERATION);

    sender->pipeline_ = std::make_shared<Pipeline::Pipeline>();
    EXPECT_EQ(sender->Prepare(), Status::OK);
}

HWTEST_F(AvAudioSenderEngineTest, PreparePipeline_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::STARTED;
    int32_t ret = sender->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_PREPARE_FAILED, ret);
}

HWTEST_F(AvAudioSenderEngineTest, PreparePipeline_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::INITIALIZED;
    int32_t ret = sender->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_NULL_POINTER, ret);
}

HWTEST_F(AvAudioSenderEngineTest, PreparePipeline_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::INITIALIZED;
    int32_t ret = sender->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_NULL_POINTER, ret);
}

HWTEST_F(AvAudioSenderEngineTest, PreparePipeline_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::INITIALIZED;
    int32_t ret = sender->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_NULL_POINTER, ret);
}

HWTEST_F(AvAudioSenderEngineTest, PreparePipeline_005, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::INITIALIZED;
    sender->pipeline_ = std::make_shared<Pipeline::Pipeline>();

    int32_t ret = sender->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_NULL_POINTER, ret);
}

HWTEST_F(AvAudioSenderEngineTest, PreparePipeline_006, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::INITIALIZED;
    sender->pipeline_ = std::make_shared<Pipeline::Pipeline>();
    sender->meta_ = std::make_shared<Media::Meta>();
    int32_t ret = sender->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_PREPARE_FAILED, ret);
}

HWTEST_F(AvAudioSenderEngineTest, PreparePipeline_007, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::INITIALIZED;
    sender->pipeline_ = nullptr;
    sender->meta_ = std::make_shared<Media::Meta>();
    int32_t ret = sender->PreparePipeline(configParam);
    EXPECT_EQ(ERR_DH_AVT_SET_PARAM_FAILED, ret);
}

HWTEST_F(AvAudioSenderEngineTest, StartDumpMediaData_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    bool ret = sender->StartDumpMediaData();
    EXPECT_EQ(true, ret);
    sender->avOutput_ = nullptr;
    ret = sender->StartDumpMediaData();
    EXPECT_EQ(true, ret);
}

HWTEST_F(AvAudioSenderEngineTest, StopDumpMediaData_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    bool ret = sender->StopDumpMediaData();
    EXPECT_EQ(true, ret);

    sender->avOutput_ = nullptr;
    ret = sender->StopDumpMediaData();
    EXPECT_EQ(true, ret);
}

HWTEST_F(AvAudioSenderEngineTest, ReStartDumpMediaData_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "001";
    std::string peerDevId = "pEid";
    std::string configParam = "value";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    bool ret = sender->ReStartDumpMediaData();
    EXPECT_EQ(true, ret);
    sender->avOutput_ = nullptr;
    ret = sender->ReStartDumpMediaData();
    EXPECT_EQ(true, ret);
}

HWTEST_F(AvAudioSenderEngineTest, SendMessage_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_MIC;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    EventType type = EventType::EVENT_ADD_STREAM;
    sender->NotifyStreamChange(type);
    int32_t ret = sender->SendMessage(nullptr);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvAudioSenderEngineTest, SendMessage_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_SPEAKER;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    EventType type = EventType::EVENT_ADD_STREAM;
    sender->NotifyStreamChange(type);
    std::shared_ptr<AVTransMessage> message = std::make_shared<AVTransMessage>();
    int32_t ret = sender->SendMessage(message);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvAudioSenderEngineTest, SendMessage_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_VIRMODEM_MIC;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    EventType type = EventType::EVENT_ADD_STREAM;
    sender->NotifyStreamChange(type);
    int32_t ret = sender->SendMessage(nullptr);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvAudioSenderEngineTest, SendMessage_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_VIRMODEM_SPEAKER;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    EventType type = EventType::EVENT_ADD_STREAM;
    sender->NotifyStreamChange(type);
    std::shared_ptr<AVTransMessage> message = std::make_shared<AVTransMessage>();
    int32_t ret = sender->SendMessage(message);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvAudioSenderEngineTest, SendMessage_005, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_VIRMODEM_SPEAKER;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    EventType type = EventType::EVENT_ADD_STREAM;
    sender->NotifyStreamChange(type);
    std::shared_ptr<AVTransMessage> message = std::make_shared<AVTransMessage>();
    int32_t ret = sender->SendMessage(message);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvAudioSenderEngineTest, SendMessage_006, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    EventType type = EventType::EVENT_ADD_STREAM;
    sender->NotifyStreamChange(type);
    int32_t ret = sender->SendMessage(nullptr);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvAudioSenderEngineTest, RegisterSenderCallback_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_SCREEN;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    EventType type = EventType::EVENT_ADD_STREAM;
    sender->NotifyStreamChange(type);
    std::shared_ptr<SenderEngineCallback> callback = std::make_shared<SenderEngineCallback>();
    int32_t ret = sender->RegisterSenderCallback(callback);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(AvAudioSenderEngineTest, RegisterSenderCallback_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    std::shared_ptr<SenderEngineCallback> callback = nullptr;
    int32_t ret = sender->RegisterSenderCallback(callback);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(AvAudioSenderEngineTest, OnChannelEvent_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->senderCallback_ = nullptr;
    AVTransEvent transEvent;
    transEvent.content = "content";
    transEvent.type = EventType::EVENT_CHANNEL_OPENED;
    sender->OnChannelEvent(transEvent);
    sender->senderCallback_ = std::make_shared<SenderEngineCallback>();
    sender->OnChannelEvent(transEvent);

    EXPECT_EQ(StateId::CH_CREATED, sender->currentState_);
}

HWTEST_F(AvAudioSenderEngineTest, OnChannelEvent_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    AVTransEvent transEvent;
    transEvent.content = "content";
    transEvent.type = EventType::EVENT_CHANNEL_OPEN_FAIL;
    sender->senderCallback_ = std::make_shared<SenderEngineCallback>();
    sender->OnChannelEvent(transEvent);

    EXPECT_EQ(StateId::INITIALIZED, sender->currentState_);
}

HWTEST_F(AvAudioSenderEngineTest, OnChannelEvent_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
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

HWTEST_F(AvAudioSenderEngineTest, OnChannelEvent_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
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
    EXPECT_NO_FATAL_FAILURE(sender->OnChannelEvent(event));
}

HWTEST_F(AvAudioSenderEngineTest, OnEvent_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);

    Pipeline::Event event;
    event.srcFilter = "filter";
    event.type = Pipeline::EventType::EVENT_READY;
    sender->currentState_ = StateId::INITIALIZED;

    EXPECT_NO_FATAL_FAILURE(sender->OnEvent(event));

    sender->senderCallback_ = std::make_shared<SenderEngineCallback>();
    EXPECT_NO_FATAL_FAILURE(sender->OnEvent(event));
}

HWTEST_F(AvAudioSenderEngineTest, OnCallback_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::INITIALIZED;

    EXPECT_EQ(Status::OK, sender->OnCallback(nullptr, Pipeline::FilterCallBackCommand::FILTER_CALLBACK_COMMAND_MAX,
        Pipeline::StreamType::STREAMTYPE_MAX));
}

HWTEST_F(AvAudioSenderEngineTest, OnCallback_002, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::INITIALIZED;

    EXPECT_EQ(Status::OK, sender->OnCallback(nullptr, Pipeline::FilterCallBackCommand::NEXT_FILTER_NEEDED,
        Pipeline::StreamType::STREAMTYPE_MAX));
}

HWTEST_F(AvAudioSenderEngineTest, OnCallback_003, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::INITIALIZED;

    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, sender->OnCallback(nullptr,
        Pipeline::FilterCallBackCommand::NEXT_FILTER_NEEDED, Pipeline::StreamType::STREAMTYPE_RAW_AUDIO));
}

HWTEST_F(AvAudioSenderEngineTest, OnCallback_004, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::INITIALIZED;

    EXPECT_EQ(Status::ERROR_NULL_POINTER, sender->OnCallback(nullptr,
        Pipeline::FilterCallBackCommand::NEXT_FILTER_NEEDED, Pipeline::StreamType::STREAMTYPE_ENCODED_AUDIO));
}

HWTEST_F(AvAudioSenderEngineTest, LinkAudioSinkFilter_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = OWNER_NAME_D_CAMERA;
    std::string peerDevId = "pEid";
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName, peerDevId);
    sender->currentState_ = StateId::INITIALIZED;
    sender->avOutput_ = nullptr;
    sender->pipeline_ = nullptr;

    sender->LinkAudioSinkFilter(nullptr, Pipeline::StreamType::STREAMTYPE_ENCODED_AUDIO);
    sender->pipeline_ = std::make_shared<Pipeline::Pipeline>();

    EXPECT_EQ(Status::OK, sender->LinkAudioSinkFilter(nullptr, Pipeline::StreamType::STREAMTYPE_ENCODED_AUDIO));
}
} // namespace DistributedHardware
} // namespace OHOS