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

#include "histreamer_ability_querier_test.h"
#include "histreamer_ability_querier.h"

#include <string>
#include <cstring>
#include <vector>
#include <gtest/gtest.h>

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void HistreamerAbilityQuerierTest::SetUpTestCase(void) {}

void HistreamerAbilityQuerierTest::TearDownTestCase(void) {}

void HistreamerAbilityQuerierTest::SetUp() {}

void HistreamerAbilityQuerierTest::TearDown() {}

namespace {
int32_t g_maxMessagesLen = 1 * 1024 * 1024;
static const std::uint16_t UINT16_ONE = 1;
static const std::vector<VideoEncoder>::size_type VIDEO_ENCODER_FOUR = 4;
static const std::vector<VideoDecoder>::size_type VIDEO_DECODER_FOUR = 4;

static const std::string TEST_STR = "test string";
static const std::string KEY = "key";
static const std::string NAME = "name";
static const std::string MIME = "mime";
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

static const std::string VIDEO_DECODER_NAME = "HdiCodecAdapter.OMX.rk.video_decoder.hevc";
static const std::string VIDEO_DECODERIN_MIME = "video/hevc";
static const std::string VIDEO_DECODEROUT_MIME = "video/raw";

static const std::vector<std::string> AUDIO_ENCODER_WANT = { "ffmpegAuEnc_aac" };
static const std::vector<std::string> AUDIO_DECODER_WANT = { "ffmpegAuDec_aac" };
static const std::vector<std::string> VIDEO_ENCODER_WANT = {
    "HdiCodecAdapter.OMX.hisi.video.encoder.hevc",
    "HdiCodecAdapter.OMX.hisi.video.encoder.avc",
    "HdiCodecAdapter.OMX.rk.video_encoder.hevc",
    "HdiCodecAdapter.OMX.rk.video_encoder.avc"
    };
static const std::vector<std::string> VIDEO_DECODER_WANT = {
    "HdiCodecAdapter.OMX.hisi.video.decoder.hevc",
    "HdiCodecAdapter.OMX.hisi.video.decoder.avc",
    "HdiCodecAdapter.OMX.rk.video_decoder.hevc",
    "HdiCodecAdapter.OMX.rk.video_decoder.avc"
    };
}

/**
 * @tc.name:histreamer_ability_querier_test_001
 * @tc.desc: Verify the QueryAudioEncoderAbility function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_001, TestSize.Level0)
{
    std::vector<AudioEncoder> audioEncoders = QueryAudioEncoderAbility();
    EXPECT_FALSE(audioEncoders.empty());
    for (std::vector<AudioEncoder>::size_type i = 0; i < audioEncoders.size(); i++) {
        auto it = find(AUDIO_ENCODER_WANT.begin(), AUDIO_ENCODER_WANT.end(), audioEncoders[i].name);
        EXPECT_TRUE(it != AUDIO_ENCODER_WANT.end());
    }
}

/**
 * @tc.name: histreamer_ability_querier_test_002
 * @tc.desc: Verify QueryAudioDecoderAbility function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_002, TestSize.Level0)
{
    std::vector<AudioDecoder> audioDecoders = QueryAudioDecoderAbility();
    EXPECT_FALSE(audioDecoders.empty());
    for (std::vector<AudioDecoder>::size_type i = 0; i < audioDecoders.size(); i++) {
        auto it = find(AUDIO_DECODER_WANT.begin(), AUDIO_DECODER_WANT.end(), audioDecoders[i].name);
        EXPECT_TRUE(it != AUDIO_DECODER_WANT.end());
    }
}

/**
 * @tc.name: histreamer_ability_querier_test_003
 * @tc.desc: Verify QueryVideoEncoderAbility function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_003, TestSize.Level0)
{
    std::vector<VideoEncoder> videoEncoders = QueryVideoEncoderAbility();
    EXPECT_FALSE(videoEncoders.empty() || videoEncoders.size() > VIDEO_ENCODER_FOUR);
    for (std::vector<VideoEncoder>::size_type i = 0; i < videoEncoders.size(); i++) {
        auto it = find(VIDEO_ENCODER_WANT.begin(), VIDEO_ENCODER_WANT.end(), videoEncoders[i].name);
        EXPECT_TRUE(it != VIDEO_ENCODER_WANT.end());
    }
}

/**
 * @tc.name: histreamer_ability_querier_test_004
 * @tc.desc: Verify QueryVideoDecoderAbility function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 *
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_004, TestSize.Level0)
{
    std::vector<VideoDecoder> videoDecoders = QueryVideoDecoderAbility();
    EXPECT_FALSE(videoDecoders.empty() || videoDecoders.size() > VIDEO_DECODER_FOUR);
    for (std::vector<VideoDecoder>::size_type i = 0; i < videoDecoders.size(); i++) {
        auto it = find(VIDEO_DECODER_WANT.begin(), VIDEO_DECODER_WANT.end(), videoDecoders[i].name);
        EXPECT_TRUE(it != VIDEO_DECODER_WANT.end());
    }
}

/**
 * @tc.name: histreamer_ability_querier_test_005
 * @tc.desc: Verify QueryAudioEncoderAbilityStr function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_005, TestSize.Level0)
{
    char* RES_MAX = new char[g_maxMessagesLen];
    EXPECT_TRUE(QueryAudioEncoderAbilityStr(RES_MAX) >= 0);
    EXPECT_TRUE(QueryAudioEncoderAbilityStr(RES_MAX) <= g_maxMessagesLen);
    delete[] RES_MAX;
}

/**
 * @tc.name: histreamer_ability_querier_test_006
 * @tc.desc: Verify QueryAudioDecoderAbilityStr function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_006, TestSize.Level0)
{
    char* RES_MAX = new char[g_maxMessagesLen];
    EXPECT_TRUE(QueryAudioDecoderAbilityStr(RES_MAX) >= 0);
    EXPECT_TRUE(QueryAudioDecoderAbilityStr(RES_MAX) <= g_maxMessagesLen);
    delete[] RES_MAX;
}

/**
 * @tc.name: histreamer_ability_querier_test_007
 * @tc.desc: Verify QueryVideoEncoderAbilityStr function
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_007, TestSize.Level0)
{
    char* RES_MAX = new char[g_maxMessagesLen];
    EXPECT_TRUE(QueryVideoEncoderAbilityStr(RES_MAX) >= 0);
    EXPECT_TRUE(QueryVideoEncoderAbilityStr(RES_MAX) <= g_maxMessagesLen);
    delete[] RES_MAX;
}

/**
 * @tc.name: histreamer_ability_querier_test_008
 * @tc.desc: Verify the QueryVideoDecoderAbilityStr function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_008, TestSize.Level0)
{
    char* RES_MAX = new char[g_maxMessagesLen];
    EXPECT_TRUE(QueryVideoDecoderAbilityStr(RES_MAX) >= 0);
    EXPECT_TRUE(QueryVideoDecoderAbilityStr(RES_MAX) <= g_maxMessagesLen);
    delete[] RES_MAX;
}

/**
 * @tc.name: histreamer_ability_querier_test_009
 * @tc.desc: Verify the IsString function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_009, TestSize.Level0)
{
    cJSON *jsonObject = cJSON_CreateObject();
    if (jsonObject == nullptr) {
        return;
    }
    cJSON_AddStringToObject(jsonObject, KEY.c_str(), TEST_STR.c_str());
    EXPECT_TRUE(IsString(jsonObject, KEY));
    cJSON_Delete(jsonObject);

    cJSON *jsonObject2 = cJSON_CreateObject();
    if (jsonObject2 == nullptr) {
        return;
    }
    cJSON_AddNumberToObject(jsonObject2, KEY.c_str(), 1);
    EXPECT_FALSE(IsString(jsonObject2, KEY));
    cJSON_Delete(jsonObject2);
}

/**
 * @tc.name: histreamer_ability_querier_test_010
 * @tc.desc: Verify the IsUint8 function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_010, TestSize.Level0)
{
    cJSON *jsonObject = cJSON_CreateObject();
    if (jsonObject == nullptr) {
        return;
    }
    const uint8_t num_s = 1;
    cJSON_AddNumberToObject(jsonObject, KEY.c_str(), num_s);
    EXPECT_TRUE(IsUInt8(jsonObject, KEY));

    const uint16_t num_b = UINT8_MAX + 1 ;
    cJSON_AddNumberToObject(jsonObject, KEY.c_str(), num_b);
    EXPECT_TRUE(IsUInt8(jsonObject, KEY));
    cJSON_Delete(jsonObject);
}

/**
 * @tc.name: histreamer_ability_querier_test_011
 * @tc.desc: Verify the IsUInt32 function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_011, TestSize.Level0)
{
    cJSON *jsonObject = cJSON_CreateObject();
    if (jsonObject == nullptr) {
        return;
    }
    const uint32_t num_s = 1;
    cJSON_AddNumberToObject(jsonObject, KEY.c_str(), num_s);
    EXPECT_TRUE(IsUInt32(jsonObject, KEY));

    const int32_t num_i = -1;
    cJSON_AddNumberToObject(jsonObject, KEY.c_str(), num_i);
    EXPECT_TRUE(IsUInt32(jsonObject, KEY));
    cJSON_Delete(jsonObject);
}

/**
 * @tc.name: histreamer_ability_querier_test_012
 * @tc.desc: Verify the FromJson(const cJSON *jsonObject, AudioEncoderIn &audioEncoderIn) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_012, TestSize.Level0)
{
    cJSON *jsonObject = cJSON_CreateObject();
    if (jsonObject == nullptr) {
        return;
    }
    AudioEncoderIn audioEncoderIn;
    cJSON_AddNumberToObject(jsonObject, MIME.c_str(), UINT16_ONE);
    FromJson(jsonObject, audioEncoderIn);
    EXPECT_TRUE(audioEncoderIn.mime.empty());
    cJSON_Delete(jsonObject);
}

/**
 * @tc.name: histreamer_ability_querier_test_013
 * @tc.desc: Verify the FromJson(const cJSON *jsonObject, AudioEncoderOut &audioEncoderOut) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_013, TestSize.Level0)
{
    cJSON *jsonObject = cJSON_CreateObject();
    if (jsonObject == nullptr) {
        return;
    }
    AudioEncoderOut audioEncoderOut;
    cJSON_AddNumberToObject(jsonObject, MIME.c_str(), UINT16_ONE);
    FromJson(jsonObject, audioEncoderOut);
    EXPECT_TRUE(audioEncoderOut.mime.empty());
    cJSON_Delete(jsonObject);
    cJSON *jsonObject2 = cJSON_CreateObject();
    if (jsonObject2 == nullptr) {
        return;
    }

    cJSON_AddStringToObject(jsonObject2, MIME.c_str(), AUDIO_ENCODEROUT_MIME.c_str());
    cJSON_AddNumberToObject(jsonObject2, AD_MPEG_VER.c_str(), AD_MPEG_VER_VALUE);
    FromJson(jsonObject2, audioEncoderOut);
    EXPECT_EQ(AUDIO_ENCODEROUT_MIME, audioEncoderOut.mime);
    EXPECT_EQ(AD_MPEG_VER_VALUE, (uint32_t)audioEncoderOut.ad_mpeg_ver);

    cJSON_AddNumberToObject(jsonObject2, AUDIO_AAC_PROFILE.c_str(), AUDIO_AAC_PROFILE_VALUE);
    cJSON_AddStringToObject(jsonObject2, AUDIO_AAC_STREAM_FORMAT.c_str(), TEST_STR.c_str());
    FromJson(jsonObject2, audioEncoderOut);
    EXPECT_EQ(AUDIO_AAC_PROFILE_VALUE, (uint32_t)audioEncoderOut.aac_profile);
    cJSON_Delete(jsonObject2);
}

/**
 * @tc.name: histreamer_ability_querier_test_014
 * @tc.desc: Verify the FromJson(const cJSON *jsonObject, AudioEncoder &audioEncoder) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_014, TestSize.Level0)
{
    cJSON *jsonObject = cJSON_CreateObject();
    if (jsonObject == nullptr) {
        return;
    }
    AudioEncoder audioEncoder;
    cJSON_AddNumberToObject(jsonObject, NAME.c_str(), UINT16_ONE);
    FromJson(jsonObject, audioEncoder);
    EXPECT_TRUE(audioEncoder.name.empty());
    cJSON_Delete(jsonObject);

    cJSON *jsonObject2 = cJSON_CreateObject();
    if (jsonObject2 == nullptr) {
        return;
    }

    cJSON_AddStringToObject(jsonObject2, NAME.c_str(), AUDIO_ENCODER_NAME.c_str());
    FromJson(jsonObject2, audioEncoder);
    EXPECT_EQ(AUDIO_ENCODER_NAME, audioEncoder.name);
    EXPECT_TRUE(audioEncoder.ins.empty());

    AudioEncoderIn audioEncoderIn;
    audioEncoderIn.mime = AUDIO_ENCODERIN_MIME;
    audioEncoderIn.sample_rate = {96000, 88200, 64000, 48000, 44100, 32000};
    audioEncoder.ins.push_back(audioEncoderIn);
    FromJson(jsonObject2, audioEncoder);
    EXPECT_FALSE(audioEncoder.ins.empty());
    EXPECT_TRUE(audioEncoder.outs.empty());
    cJSON_Delete(jsonObject2);
}

/**
 * @tc.name: histreamer_ability_querier_test_015
 * @tc.desc: Verify the FromJson(const cJSON *jsonObject, AudioDecoderIn &audioDecoderIn) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_015, TestSize.Level0)
{
    cJSON *jsonObject = cJSON_CreateObject();
    if (jsonObject == nullptr) {
        return;
    }
    AudioDecoderIn audioDecoderIn;
    cJSON_AddNumberToObject(jsonObject, MIME.c_str(), UINT16_ONE);
    FromJson(jsonObject, audioDecoderIn);
    EXPECT_TRUE(audioDecoderIn.mime.empty());
    cJSON_Delete(jsonObject);

    cJSON *jsonObject2 = cJSON_CreateObject();
    if (jsonObject2 == nullptr) {
        return;
    }

    cJSON_AddStringToObject(jsonObject2, MIME.c_str(), AUDIO_DECODERIN_MIME.c_str());
    FromJson(jsonObject2, audioDecoderIn);
    EXPECT_EQ(AUDIO_DECODERIN_MIME, audioDecoderIn.mime);
    EXPECT_TRUE(audioDecoderIn.channel_layout.empty());
    cJSON_Delete(jsonObject2);
}

/**
 * @tc.name: histreamer_ability_querier_test_016
 * @tc.desc: Verify the FromJson(const cJSON *jsonObject, AudioDecoderOut &audioDecoderOut) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_016, TestSize.Level0)
{
    cJSON *jsonObject = cJSON_CreateObject();
    if (jsonObject == nullptr) {
        return;
    }
    AudioDecoderOut audioDecoderOut;
    cJSON_AddNumberToObject(jsonObject, MIME.c_str(), UINT16_ONE);
    FromJson(jsonObject, audioDecoderOut);
    EXPECT_TRUE(audioDecoderOut.mime.empty());
    cJSON_Delete(jsonObject);

    cJSON *jsonObject2 = cJSON_CreateObject();
    if (jsonObject2 == nullptr) {
        return;
    }

    cJSON_AddStringToObject(jsonObject2, MIME.c_str(), AUDIO_DECODEROUT_MIME.c_str());
    FromJson(jsonObject2, audioDecoderOut);
    EXPECT_EQ(AUDIO_DECODEROUT_MIME, audioDecoderOut.mime);
    EXPECT_TRUE(audioDecoderOut.sample_fmt.empty());
    cJSON_Delete(jsonObject2);
}

/**
 * @tc.name: histreamer_ability_querier_test_017
 * @tc.desc: Verify the FromJson(const cJSON *jsonObject, AudioDecoder &audioDecoder) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_017, TestSize.Level0)
{
    cJSON *jsonObject = cJSON_CreateObject();
    if (jsonObject == nullptr) {
        return;
    }
    AudioDecoder audioDecoder;
    cJSON_AddNumberToObject(jsonObject, NAME.c_str(), UINT16_ONE);
    FromJson(jsonObject, audioDecoder);
    EXPECT_TRUE(audioDecoder.name.empty());
    cJSON_Delete(jsonObject);

    cJSON *jsonObject2 = cJSON_CreateObject();
    if (jsonObject2 == nullptr) {
        return;
    }

    cJSON_AddStringToObject(jsonObject2, NAME.c_str(), AUDIO_DECODER_NAME.c_str());
    FromJson(jsonObject2, audioDecoder);
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
    FromJson(jsonObject2, audioDecoder);
    EXPECT_FALSE(audioDecoder.ins.empty());
    EXPECT_TRUE(audioDecoder.outs.empty());
    cJSON_Delete(jsonObject2);
}

/**
 * @tc.name: histreamer_ability_querier_test_018
 * @tc.desc: Verify the FromJson(const cJSON *jsonObject, VideoEncoderIn &videoEncoderIn) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_018, TestSize.Level0)
{
    cJSON *jsonObject = cJSON_CreateObject();
    if (jsonObject == nullptr) {
        return;
    }
    VideoEncoderIn videoEncoderIn;
    cJSON_AddNumberToObject(jsonObject, MIME.c_str(), UINT16_ONE);
    FromJson(jsonObject, videoEncoderIn);
    EXPECT_TRUE(videoEncoderIn.mime.empty());
    cJSON_Delete(jsonObject);

    cJSON *jsonObject2 = cJSON_CreateObject();
    if (jsonObject2 == nullptr) {
        return;
    }

    cJSON_AddStringToObject(jsonObject2, MIME.c_str(), VIDEO_ENCODERIN_MIME.c_str());
    FromJson(jsonObject2, videoEncoderIn);
    EXPECT_EQ(VIDEO_ENCODERIN_MIME, videoEncoderIn.mime);
    EXPECT_TRUE(videoEncoderIn.pixel_fmt.empty());
    cJSON_Delete(jsonObject2);
}

/**
 * @tc.name: histreamer_ability_querier_test_019
 * @tc.desc: Verify the FromJson(const cJSON *jsonObject, VideoEncoderOut &videoEncoderOut) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_019, TestSize.Level0)
{
    cJSON *jsonObject = cJSON_CreateObject();
    if (jsonObject == nullptr) {
        return;
    }
    VideoEncoderOut videoEncoderOut;
    cJSON_AddNumberToObject(jsonObject, MIME.c_str(), UINT16_ONE);
    FromJson(jsonObject, videoEncoderOut);
    EXPECT_TRUE(videoEncoderOut.mime.empty());
    cJSON_Delete(jsonObject);
}

/**
 * @tc.name: histreamer_ability_querier_test_020
 * @tc.desc: Verify the FromJson(const cJSON *jsonObject, VideoEncoder &videoEncoder) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_020, TestSize.Level0)
{
    cJSON *jsonObject = cJSON_CreateObject();
    if (jsonObject == nullptr) {
        return;
    }
    VideoEncoder videoEncoder;
    cJSON_AddNumberToObject(jsonObject, NAME.c_str(), UINT16_ONE);
    FromJson(jsonObject, videoEncoder);
    EXPECT_TRUE(videoEncoder.name.empty());
    cJSON_Delete(jsonObject);

    cJSON *jsonObject2 = cJSON_CreateObject();
    if (jsonObject2 == nullptr) {
        return;
    }

    cJSON_AddStringToObject(jsonObject2, NAME.c_str(), VIDEO_ENCODER_NAME.c_str());
    FromJson(jsonObject2, videoEncoder);
    EXPECT_EQ(VIDEO_ENCODER_NAME, videoEncoder.name);
    EXPECT_TRUE(videoEncoder.ins.empty());

    VideoEncoderIn videoEncoderIn;
    videoEncoderIn.mime = VIDEO_ENCODERIN_MIME;
    videoEncoderIn.pixel_fmt = {
        VideoPixelFormat::YUV410P,
        VideoPixelFormat::RGBA,
    };
    videoEncoder.ins.push_back(videoEncoderIn);
    FromJson(jsonObject2, videoEncoder);
    EXPECT_FALSE(videoEncoder.ins.empty());
    EXPECT_TRUE(videoEncoder.outs.empty());
    cJSON_Delete(jsonObject2);
}

/**
 * @tc.name: histreamer_ability_querier_test_021
 * @tc.desc: Verify the FromJson(const cJSON *jsonObject, VideoDecoderIn &videoDecoderIn) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_021, TestSize.Level0)
{
    cJSON *jsonObject = cJSON_CreateObject();
    if (jsonObject == nullptr) {
        return;
    }
    VideoDecoderIn videoDecoderIn;
    cJSON_AddNumberToObject(jsonObject, MIME.c_str(), UINT16_ONE);
    FromJson(jsonObject, videoDecoderIn);
    EXPECT_TRUE(videoDecoderIn.mime.empty());
    cJSON_Delete(jsonObject);

    cJSON *jsonObject2 = cJSON_CreateObject();
    if (jsonObject2 == nullptr) {
        return;
    }

    cJSON_AddStringToObject(jsonObject2, MIME.c_str(), VIDEO_DECODERIN_MIME.c_str());
    FromJson(jsonObject2, videoDecoderIn);
    EXPECT_EQ(VIDEO_DECODERIN_MIME, videoDecoderIn.mime);
    EXPECT_TRUE(videoDecoderIn.vd_bit_stream_fmt.empty());
    cJSON_Delete(jsonObject2);
}

/**
 * @tc.name: histreamer_ability_querier_test_022
 * @tc.desc: Verify the FromJson(const cJSON *jsonObject, VideoDecoderOut &videoDecoderOut) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_022, TestSize.Level0)
{
    cJSON *jsonObject = cJSON_CreateObject();
    if (jsonObject == nullptr) {
        return;
    }
    VideoDecoderOut videoDecoderOut;
    cJSON_AddNumberToObject(jsonObject, MIME.c_str(), UINT16_ONE);
    FromJson(jsonObject, videoDecoderOut);
    EXPECT_TRUE(videoDecoderOut.mime.empty());
    cJSON_Delete(jsonObject);

    cJSON *jsonObject2 = cJSON_CreateObject();
    if (jsonObject2 == nullptr) {
        return;
    }

    cJSON_AddStringToObject(jsonObject2, MIME.c_str(), VIDEO_DECODEROUT_MIME.c_str());
    FromJson(jsonObject2, videoDecoderOut);
    EXPECT_EQ(VIDEO_DECODEROUT_MIME, videoDecoderOut.mime);
    EXPECT_TRUE(videoDecoderOut.pixel_fmt.empty());
    cJSON_Delete(jsonObject2);
}

/**
 * @tc.name: histreamer_ability_querier_test_023
 * @tc.desc: Verify the FromJson(const cJSON *jsonObject, VideoDecoder &videoDecoder) function.
 * @tc.type: FUNC
 * @tc.require: issuelI7MJPJ
 */
HWTEST_F(HistreamerAbilityQuerierTest, histreamer_ability_querier_test_023, TestSize.Level0)
{
    cJSON *jsonObject = cJSON_CreateObject();
    if (jsonObject == nullptr) {
        return;
    }
    VideoDecoder videoDecoder;
    cJSON_AddNumberToObject(jsonObject, NAME.c_str(), UINT16_ONE);
    FromJson(jsonObject, videoDecoder);
    EXPECT_TRUE(videoDecoder.name.empty());
    cJSON_Delete(jsonObject);

    cJSON *jsonObject2 = cJSON_CreateObject();
    if (jsonObject2 == nullptr) {
        return;
    }

    cJSON_AddStringToObject(jsonObject2, NAME.c_str(), VIDEO_DECODER_NAME.c_str());
    FromJson(jsonObject2, videoDecoder);
    EXPECT_EQ(VIDEO_DECODER_NAME, videoDecoder.name);
    EXPECT_TRUE(videoDecoder.ins.empty());

    VideoDecoderIn videoDecoderIn;
    videoDecoderIn.mime = VIDEO_DECODERIN_MIME;
    videoDecoderIn.vd_bit_stream_fmt = {
        VideoBitStreamFormat::AVC1,
        VideoBitStreamFormat::HEVC,
    };
    videoDecoder.ins.push_back(videoDecoderIn);
    FromJson(jsonObject2, videoDecoder);
    EXPECT_FALSE(videoDecoder.ins.empty());
    EXPECT_TRUE(videoDecoder.outs.empty());
    cJSON_Delete(jsonObject2);
}
} // namespace DistributedHardware
} // namespace OHOS