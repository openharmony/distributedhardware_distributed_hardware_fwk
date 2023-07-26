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

#include "histreamer_ability_parser_test.h"
#include <vector>
#include <gtest/gtest.h>

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
static const std::uint16_t UINT16_ONE = 1;

static const std::string TEST_STR = "test string";
static const std::string KEY = "key";
static const std::string NAME = "name";
static const std::string MIME = "mime";
static const std::string INS = "ins";
static const std::string OUTS = "outs";
static const std::string AUDIO_ENCODER_NAME = "ffmpegAuEnc_aac";
static const std::string AUDIO_ENCODERIN_MIME = "audio/raw";
static const std::string AUDIO_ENCODEROUT_MIME = "audio/mp4a-latm";
static const std::string AD_MPEG_VER = "ad_mpeg_ver";
static const std::uint32_t AD_MPEG_VER_VALUE = 4;
static const std::string AUDIO_AAC_PROFILE = "aac_profile";
static const std::uint32_t AUDIO_AAC_PROFILE_VALUE = 0;
static const std::string AUDIO_AAC_STREAM_FORMAT = "aac_stm_fmt";

static const std::string AUDIO_DECODER_NAME = "ffmpegAuDec_aac";
static const std::string AUDIO_DECODERIN_MIME = "audio/mp4a-latm";
static const std::string AUDIO_DECODEROUT_MIME = "audio/raw";

static const std::string VIDEO_ENCODER_NAME = "HdiCodecAdapter.OMX.rk.video_encoder.hevc";
static const std::string VIDEO_ENCODERIN_MIME = "video/raw";
static const std::string VIDEO_ENCODEROUT_MIME = "video/hevc";

static const std::string VIDEO_DECODER_NAME = "HdiCodecAdapter.OMX.rk.video_decoder.hevc";
static const std::string VIDEO_DECODERIN_MIME = "video/hevc";
static const std::string VIDEO_DECODEROUT_MIME = "video/raw";

static const std::string SAMPLE_RATE = "sample_rate";
static const std::string VIDEO_PIXEL_FMT = "pixel_fmt";

void HistreamerAbilityParserTest::SetUpTestCase(void) {}

void HistreamerAbilityParserTest::TearDownTestCase(void) {}

void HistreamerAbilityParserTest::SetUp() {}

void HistreamerAbilityParserTest::TearDown() {}

/**
 * @tc.name: histreamer_ability_parser_test_001
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, AudioEncoderIn &audioEncoderIn) function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityParserTest, histreamer_ability_parser_test_001, TestSize.Level0)
{
    nlohmann::json jsonObject;
    AudioEncoderIn audioEncoderIn;
    jsonObject[MIME] = UINT16_ONE;
    FromJson(jsonObject, audioEncoderIn);
    EXPECT_TRUE(audioEncoderIn.mime.empty());
}

/**
 * @tc.name: histreamer_ability_parser_test_002
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, AudioEncoderOut &audioEncoderOut) function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityParserTest, histreamer_ability_parser_test_002, TestSize.Level0)
{
    nlohmann::json jsonObject;
    AudioEncoderOut audioEncoderOut;
    jsonObject[MIME] = UINT16_ONE;
    FromJson(jsonObject, audioEncoderOut);
    EXPECT_TRUE(audioEncoderOut.mime.empty());

    jsonObject[MIME] = AUDIO_ENCODEROUT_MIME;
    jsonObject[AD_MPEG_VER] = TEST_STR;
    FromJson(jsonObject, audioEncoderOut);
    EXPECT_EQ(AUDIO_ENCODEROUT_MIME, audioEncoderOut.mime);

    jsonObject[AD_MPEG_VER] = AD_MPEG_VER_VALUE;
    jsonObject[AUDIO_AAC_PROFILE] = TEST_STR;
    FromJson(jsonObject, audioEncoderOut);
    EXPECT_EQ(AD_MPEG_VER_VALUE, (uint32_t)audioEncoderOut.ad_mpeg_ver);

    jsonObject[AUDIO_AAC_PROFILE] = AUDIO_AAC_PROFILE_VALUE;
    jsonObject[AUDIO_AAC_STREAM_FORMAT] = TEST_STR;
    FromJson(jsonObject, audioEncoderOut);
    EXPECT_EQ(AUDIO_AAC_PROFILE_VALUE, (uint32_t)audioEncoderOut.aac_profile);
}

/**
 * @tc.name: histreamer_ability_parser_test_003
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, AudioEncoder &audioEncoder) function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityParserTest, histreamer_ability_parser_test_003, TestSize.Level0)
{
    nlohmann::json jsonObject;
    AudioEncoder audioEncoder;
    jsonObject[NAME] = UINT16_ONE;
    FromJson(jsonObject, audioEncoder);
    EXPECT_TRUE(audioEncoder.name.empty());

    jsonObject[NAME] = AUDIO_ENCODER_NAME;
    FromJson(jsonObject, audioEncoder);
    EXPECT_EQ(AUDIO_ENCODER_NAME, audioEncoder.name);
    EXPECT_TRUE(audioEncoder.ins.empty());

    AudioEncoderIn audioEncoderIn;
    audioEncoderIn.mime = AUDIO_ENCODERIN_MIME;
    audioEncoderIn.sample_rate = {96000, 88200, 64000, 48000, 44100, 32000};
    audioEncoder.ins.push_back(audioEncoderIn);
    FromJson(jsonObject, audioEncoder);
    EXPECT_FALSE(audioEncoder.ins.empty());
    EXPECT_TRUE(audioEncoder.outs.empty());
}

/**
 * @tc.name: histreamer_ability_parser_test_004
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, AudioDecoderIn &audioDecoderIn) function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityParserTest, histreamer_ability_parser_test_004, TestSize.Level0)
{
    nlohmann::json jsonObject;
    AudioDecoderIn audioDecoderIn;
    jsonObject[MIME] = UINT16_ONE;
    FromJson(jsonObject, audioDecoderIn);
    EXPECT_TRUE(audioDecoderIn.mime.empty());

    jsonObject[MIME] = AUDIO_DECODERIN_MIME;
    FromJson(jsonObject, audioDecoderIn);
    EXPECT_EQ(AUDIO_DECODERIN_MIME, audioDecoderIn.mime);
    EXPECT_TRUE(audioDecoderIn.channel_layout.empty());
}

/**
 * @tc.name: histreamer_ability_parser_test_005
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, AudioDecoderOut &audioDecoderOut) function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityParserTest, histreamer_ability_parser_test_005, TestSize.Level0)
{
    nlohmann::json jsonObject;
    AudioDecoderOut audioDecoderOut;
    jsonObject[MIME] = UINT16_ONE;
    FromJson(jsonObject, audioDecoderOut);
    EXPECT_TRUE(audioDecoderOut.mime.empty());

    jsonObject[MIME] = AUDIO_DECODEROUT_MIME;
    FromJson(jsonObject, audioDecoderOut);
    EXPECT_EQ(AUDIO_DECODEROUT_MIME, audioDecoderOut.mime);
    EXPECT_TRUE(audioDecoderOut.sample_fmt.empty());
}

/**
 * @tc.name: histreamer_ability_parser_test_006
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, AudioDecoder &audioDecoder) function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityParserTest, histreamer_ability_parser_test_006, TestSize.Level0)
{
    nlohmann::json jsonObject;
    AudioDecoder audioDecoder;
    jsonObject[NAME] = UINT16_ONE;
    FromJson(jsonObject, audioDecoder);
    EXPECT_TRUE(audioDecoder.name.empty());

    jsonObject[NAME] = AUDIO_DECODER_NAME;
    FromJson(jsonObject, audioDecoder);
    EXPECT_EQ(AUDIO_DECODER_NAME, audioDecoder.name);
    EXPECT_TRUE(audioDecoder.ins.empty());

    AudioDecoderIn audioDecoderIn;
    audioDecoderIn.mime = AUDIO_DECODERIN_MIME;
    audioDecoderIn.channel_layout = {
        AudioChannelLayout::CH_2POINT1,
        AudioChannelLayout::CH_2_1,
        AudioChannelLayout::SURROUND,
    };
    audioDecoder.ins.push_back(audioDecoderIn);
    FromJson(jsonObject, audioDecoder);
    EXPECT_FALSE(audioDecoder.ins.empty());
    EXPECT_TRUE(audioDecoder.outs.empty());
}

/**
 * @tc.name: histreamer_ability_parser_test_007
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, VideoEncoderIn &videoEncoderIn) function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityParserTest, histreamer_ability_parser_test_007, TestSize.Level0)
{
    nlohmann::json jsonObject;
    VideoEncoderIn videoEncoderIn;
    jsonObject[MIME] = UINT16_ONE;
    FromJson(jsonObject, videoEncoderIn);
    EXPECT_TRUE(videoEncoderIn.mime.empty());

    jsonObject[MIME] = VIDEO_ENCODERIN_MIME;
    FromJson(jsonObject, videoEncoderIn);
    EXPECT_EQ(VIDEO_ENCODERIN_MIME, videoEncoderIn.mime);
    EXPECT_TRUE(videoEncoderIn.pixel_fmt.empty());
}

/**
 * @tc.name: histreamer_ability_parser_test_008
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, VideoEncoderOut &videoEncoderOut) function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityParserTest, histreamer_ability_parser_test_008, TestSize.Level0)
{
    nlohmann::json jsonObject;
    VideoEncoderOut videoEncoderOut;
    jsonObject[MIME] = UINT16_ONE;
    FromJson(jsonObject, videoEncoderOut);
    EXPECT_TRUE(videoEncoderOut.mime.empty());
}

/**
 * @tc.name: histreamer_ability_parser_test_009
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, VideoEncoder &videoEncoder) function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityParserTest, histreamer_ability_parser_test_009, TestSize.Level0)
{
    nlohmann::json jsonObject;
    VideoEncoder videoEncoder;
    jsonObject[NAME] = UINT16_ONE;
    FromJson(jsonObject, videoEncoder);
    EXPECT_TRUE(videoEncoder.name.empty());

    jsonObject[NAME] = VIDEO_ENCODER_NAME;
    FromJson(jsonObject, videoEncoder);
    EXPECT_EQ(VIDEO_ENCODER_NAME, videoEncoder.name);
    EXPECT_TRUE(videoEncoder.ins.empty());

    VideoEncoderIn videoEncoderIn;
    videoEncoderIn.mime = VIDEO_ENCODERIN_MIME;
    videoEncoderIn.pixel_fmt = {
        VideoPixelFormat::YUV410P,
        VideoPixelFormat::RGBA,
    };
    videoEncoder.ins.push_back(videoEncoderIn);
    FromJson(jsonObject, videoEncoder);
    EXPECT_FALSE(videoEncoder.ins.empty());
    EXPECT_TRUE(videoEncoder.outs.empty());
}

/**
 * @tc.name: histreamer_ability_parser_test_010
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, VideoDecoderIn &videoDecoderIn) function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityParserTest, histreamer_ability_parser_test_010, TestSize.Level0)
{
    nlohmann::json jsonObject;
    VideoDecoderIn videoDecoderIn;
    jsonObject[MIME] = UINT16_ONE;
    FromJson(jsonObject, videoDecoderIn);
    EXPECT_TRUE(videoDecoderIn.mime.empty());

    jsonObject[MIME] = VIDEO_DECODERIN_MIME;
    FromJson(jsonObject, videoDecoderIn);
    EXPECT_EQ(VIDEO_DECODERIN_MIME, videoDecoderIn.mime);
    EXPECT_TRUE(videoDecoderIn.vd_bit_stream_fmt.empty());
}

/**
 * @tc.name: histreamer_ability_parser_test_011
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, VideoDecoderOut &videoDecoderOut) function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityParserTest, histreamer_ability_parser_test_011, TestSize.Level0)
{
    nlohmann::json jsonObject;
    VideoDecoderOut videoDecoderOut;
    jsonObject[MIME] = UINT16_ONE;
    FromJson(jsonObject, videoDecoderOut);
    EXPECT_TRUE(videoDecoderOut.mime.empty());

    jsonObject[MIME] = VIDEO_DECODEROUT_MIME;
    FromJson(jsonObject, videoDecoderOut);
    EXPECT_EQ(VIDEO_DECODEROUT_MIME, videoDecoderOut.mime);
    EXPECT_TRUE(videoDecoderOut.pixel_fmt.empty());
}

/**
 * @tc.name: histreamer_ability_parser_test_012
 * @tc.desc: Verify the FromJson(const nlohmann::json &jsonObject, VideoDecoder &videoDecoder) function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityParserTest, histreamer_ability_parser_test_012, TestSize.Level0)
{
    nlohmann::json jsonObject;
    VideoDecoder videoDecoder;
    jsonObject[NAME] = UINT16_ONE;
    FromJson(jsonObject, videoDecoder);
    EXPECT_TRUE(videoDecoder.name.empty());

    jsonObject[NAME] = VIDEO_DECODER_NAME;
    FromJson(jsonObject, videoDecoder);
    EXPECT_EQ(VIDEO_DECODER_NAME, videoDecoder.name);
    EXPECT_TRUE(videoDecoder.ins.empty());

    VideoDecoderIn videoDecoderIn;
    videoDecoderIn.mime = VIDEO_DECODERIN_MIME;
    videoDecoderIn.vd_bit_stream_fmt = {
        VideoBitStreamFormat::AVC1,
        VideoBitStreamFormat::HEVC,
    };
    videoDecoder.ins.push_back(videoDecoderIn);
    FromJson(jsonObject, videoDecoder);
    EXPECT_FALSE(videoDecoder.ins.empty());
    EXPECT_TRUE(videoDecoder.outs.empty());
}

/**
 * @tc.name: histreamer_ability_parser_test_013
 * @tc.desc: Verify the FromJson<AudioEncoder> function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityParserTest, histreamer_ability_parser_test_013, TestSize.Level0)
{
    nlohmann::json jsonObject;
    AudioEncoder audioEncoder;
    jsonObject[NAME] = AUDIO_ENCODER_NAME;
    audioEncoder.name = AUDIO_ENCODER_NAME;
    std::vector<AudioEncoder>audioEncoders;
    
    FromJson(VIDEO_PIXEL_FMT, jsonObject, audioEncoders);
    EXPECT_TRUE(audioEncoders.empty());

    FromJson(NAME, jsonObject, audioEncoders);
    EXPECT_FALSE(audioEncoders.empty());
}

/**
 * @tc.name: histreamer_ability_parser_test_014
 * @tc.desc: Verify the FromJson<AudioDecoder> function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityParserTest, histreamer_ability_parser_test_014, TestSize.Level0)
{
    nlohmann::json jsonObject;
    AudioDecoder audioDecoder;
    jsonObject[NAME] = AUDIO_DECODER_NAME;
    audioDecoder.name = AUDIO_DECODER_NAME;
    std::vector<AudioDecoder>audioDecoders;

    FromJson(VIDEO_PIXEL_FMT, jsonObject, audioDecoders);
    EXPECT_TRUE(audioDecoders.empty());

    FromJson(NAME, jsonObject, audioDecoders);
    EXPECT_FALSE(audioDecoders.empty());
}

/**
 * @tc.name: histreamer_ability_parser_test_015
 * @tc.desc: Verify the FromJson<VideoEncoder> function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityParserTest, histreamer_ability_parser_test_015, TestSize.Level0)
{
    nlohmann::json jsonObject;
    VideoEncoder videoEncoder;
    jsonObject[NAME] = VIDEO_ENCODER_NAME;
    videoEncoder.name = VIDEO_ENCODER_NAME;
    std::vector<VideoEncoder>videoEncoders;

    FromJson(AUDIO_AAC_PROFILE, jsonObject, videoEncoders);
    EXPECT_TRUE(videoEncoders.empty());

    FromJson(NAME, jsonObject, videoEncoders);
    EXPECT_FALSE(videoEncoders.empty());
}

/**
 * @tc.name: histreamer_ability_parser_test_016
 * @tc.desc: Verify the FromJson<VideoDecoder> function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityParserTest, histreamer_ability_parser_test_016, TestSize.Level0)
{
    nlohmann::json jsonObject;
    VideoDecoder videoDecoder;
    jsonObject[NAME] = VIDEO_DECODER_NAME;
    videoDecoder.name = VIDEO_DECODER_NAME;
    std::vector<VideoDecoder>videoDecoders;

    FromJson(AUDIO_AAC_PROFILE, jsonObject, videoDecoders);
    EXPECT_TRUE(videoDecoders.empty());

    FromJson(NAME, jsonObject, videoDecoders);
    EXPECT_FALSE(videoDecoders.empty());
}

} // namespace DistributedHardware
} // namespace OHOS
