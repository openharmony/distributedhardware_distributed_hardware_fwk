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

#include "histreamer_ability_querier.h"

#include <algorithm>
#include <memory>
#include <vector>
#include <securec.h>
#include "plugin_caps.h"
#include "plugin_manager.h"
#include "pipeline/filters/common/plugin_utils.h"

#include "av_trans_log.h"

using OHOS::Media::Plugin::PluginManager;
using OHOS::Media::Plugin::PluginType;
using OHOS::Media::Plugin::PluginInfo;
using OHOS::Media::Plugin::Capability;
using OHOS::Media::Plugin::CapabilitySet;
using OHOS::Media::Plugin::AnyCast;
using CapabilityID = OHOS::Media::Plugin::Capability::Key;

namespace OHOS {
namespace DistributedHardware {

const uint32_t MAX_MESSAGES_LEN = 1 * 1024 * 1024;
static const std::string NAME = "name";
static const std::string INS = "ins";
static const std::string OUTS = "outs";
static const std::string MIME = "mime";
static const std::string SAMPLE_RATE = "sample_rate";
static const std::string AUDIO_SAMPLE_FORMAT = "sample_fmt";
static const std::string AD_MPEG_VER = "ad_mpeg_ver";
static const std::string AUDIO_AAC_PROFILE = "aac_profile";
static const std::string AUDIO_AAC_STREAM_FORMAT = "aac_stm_fmt";
static const std::string AUDIO_CHANNEL_LAYOUT = "channel_layout";

static const std::string VIDEO_PIXEL_FMT = "pixel_fmt";
static const std::string VIDEO_BIT_STREAM_FMT = "vd_bit_stream_fmt";

static const std::string AUDIO_ENCODERS = "audioEncoders";
static const std::string AUDIO_DECODERS = "audioDecoders";
static const std::string VIDEO_ENCODERS = "videoEncoders";
static const std::string VIDEO_DECODERS = "videoDecoders";

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

bool IsString(const nlohmann::json &jsonObj, const std::string &key)
{
    bool res = jsonObj.contains(key) && jsonObj[key].is_string() && jsonObj[key].size() <= MAX_MESSAGES_LEN;
    if (!res) {
        AVTRANS_LOGE("the key %{public}s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsUInt8(const nlohmann::json &jsonObj, const std::string &key)
{
    bool res = jsonObj.contains(key) && jsonObj[key].is_number_unsigned() && jsonObj[key] <= UINT8_MAX;
    return res;
}

bool IsUInt32(const nlohmann::json &jsonObj, const std::string &key)
{
    bool res = jsonObj.contains(key) && jsonObj[key].is_number_unsigned() && jsonObj[key] <= UINT32_MAX;
    if (!res) {
        AVTRANS_LOGE("the key %{public}s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

std::vector<AudioEncoderIn> ParseAudioEncoderIn(CapabilitySet &inCaps)
{
    std::vector<AudioEncoderIn> ins;
    for (auto &cap : inCaps) {
        AVTRANS_LOGD("AudioEncoderIn Raw: %{public}s", OHOS::Media::Pipeline::Capability2String(cap).c_str());
        AudioEncoderIn in;
        in.mime = cap.mime;
        in.sample_rate = AnyCast<std::vector<uint32_t>>(cap.keys[CapabilityID::AUDIO_SAMPLE_RATE]);
        ins.push_back(in);
    }

    return ins;
}

std::vector<AudioEncoderOut> ParseAudioEncoderOut(CapabilitySet &outCaps)
{
    std::vector<AudioEncoderOut> outs;
    for (auto &cap : outCaps) {
        AVTRANS_LOGD("AudioEncoderOut Raw: %{public}s", OHOS::Media::Pipeline::Capability2String(cap).c_str());
        AudioEncoderOut out;
        out.mime = cap.mime;
        out.ad_mpeg_ver = AnyCast<uint32_t>(cap.keys[CapabilityID::AUDIO_MPEG_VERSION]);
        outs.push_back(out);
    }

    return outs;
}

std::vector<AudioEncoder> QueryAudioEncoderAbility()
{
    std::vector<AudioEncoder> audioEncoders;
    auto audioEncNameList = PluginManager::Instance().ListPlugins(PluginType::AUDIO_ENCODER);
    for (const std::string& name : audioEncNameList) {
        if (find(AUDIO_ENCODER_WANT.begin(), AUDIO_ENCODER_WANT.end(), name) == AUDIO_ENCODER_WANT.end()) {
            AVTRANS_LOGI("AudioEncoder Plugin not want: %{public}s", name.c_str());
            continue;
        }
        auto pluginInfo = PluginManager::Instance().GetPluginInfo(PluginType::AUDIO_ENCODER, name);
        AudioEncoder audioEncoder;
        audioEncoder.name = name;
        audioEncoder.ins = ParseAudioEncoderIn(pluginInfo->inCaps);
        audioEncoder.outs = ParseAudioEncoderOut(pluginInfo->outCaps);
        audioEncoders.push_back(audioEncoder);
    }

    return audioEncoders;
}

void ToJson(nlohmann::json &jsonObject, const AudioEncoderIn &audioEncoderIn)
{
    jsonObject[MIME] = audioEncoderIn.mime;
    nlohmann::json sampleRateJson;
    for (auto rate : audioEncoderIn.sample_rate) {
        sampleRateJson.push_back(rate);
    }
    jsonObject[SAMPLE_RATE] = sampleRateJson;
}

void FromJson(const nlohmann::json &jsonObject, AudioEncoderIn &audioEncoderIn)
{
    if (!IsString(jsonObject, MIME)) {
        AVTRANS_LOGE("AudioEncoderIn MIME is invalid!");
        return;
    }
    audioEncoderIn.mime = jsonObject.at(MIME).get<std::string>();
    if (jsonObject.find(SAMPLE_RATE) == jsonObject.end()) {
        AVTRANS_LOGE("AudioEncoderIn SAMPLE_RATE is invalid");
    }
    audioEncoderIn.sample_rate = jsonObject.at(SAMPLE_RATE).get<std::vector<uint32_t>>();
}

void ToJson(nlohmann::json &jsonObject, const AudioEncoderOut &audioEncoderOut)
{
    jsonObject[MIME] = audioEncoderOut.mime;
    jsonObject[AD_MPEG_VER] = audioEncoderOut.ad_mpeg_ver;
    jsonObject[AUDIO_AAC_PROFILE] = (uint8_t)audioEncoderOut.aac_profile;
    jsonObject[AUDIO_AAC_STREAM_FORMAT] = (uint8_t)audioEncoderOut.aac_stm_fmt;
}

void FromJson(const nlohmann::json &jsonObject, AudioEncoderOut &audioEncoderOut)
{
    if (!IsString(jsonObject, MIME)) {
        AVTRANS_LOGE("AudioEncoderOut MIME is invalid!");
        return;
    }
    audioEncoderOut.mime = jsonObject.at(MIME).get<std::string>();

    if (!IsUInt32(jsonObject, AD_MPEG_VER)) {
        AVTRANS_LOGE("AudioEncoderOut AD_MPEG_VER is invalid");
        return;
    }
    audioEncoderOut.ad_mpeg_ver = jsonObject.at(AD_MPEG_VER).get<uint32_t>();

    if (!IsUInt8(jsonObject, AUDIO_AAC_PROFILE)) {
        AVTRANS_LOGE("AudioEncoderOut AUDIO_AAC_PROFILE is invalid");
        return;
    }
    audioEncoderOut.aac_profile = (AudioAacProfile)jsonObject.at(AUDIO_AAC_PROFILE).get<uint8_t>();

    if (!IsUInt8(jsonObject, AUDIO_AAC_STREAM_FORMAT)) {
        AVTRANS_LOGE("AudioEncoderOut AUDIO_AAC_STREAM_FORMAT is invalid");
        return;
    }
    audioEncoderOut.aac_stm_fmt = (AudioAacStreamFormat)jsonObject.at(AUDIO_AAC_STREAM_FORMAT).get<uint8_t>();
}

void ToJson(nlohmann::json &jsonObject, const AudioEncoder &audioEncoder)
{
    jsonObject[NAME] = audioEncoder.name;

    nlohmann::json audioEncoderInsJson;
    for (const auto &in : audioEncoder.ins) {
        nlohmann::json audioEncoderInJson;
        ToJson(audioEncoderInJson, in);
        audioEncoderInsJson.push_back(audioEncoderInJson);
    }
    jsonObject[INS] = audioEncoderInsJson;

    nlohmann::json audioEncoderOutsJson;
    for (const auto &out : audioEncoder.outs) {
        nlohmann::json audioEncoderOutJson;
        ToJson(audioEncoderOutJson, out);
        audioEncoderOutsJson.push_back(audioEncoderOutJson);
    }
    jsonObject[OUTS] = audioEncoderOutsJson;
}

void FromJson(const nlohmann::json &jsonObject, AudioEncoder &audioEncoder)
{
    if (!IsString(jsonObject, NAME)) {
        AVTRANS_LOGE("AudioEncoder NAME is invalid");
        return;
    }
    audioEncoder.name = jsonObject.at(NAME).get<std::string>();

    if (jsonObject.find(INS) == jsonObject.end()) {
        AVTRANS_LOGE("AudioEncoder INS is invalid");
        return;
    }

    nlohmann::json audioEncoderInsJson = jsonObject[INS];
    for (const auto &inJson : audioEncoderInsJson) {
        AudioEncoderIn in;
        FromJson(inJson, in);
        audioEncoder.ins.push_back(in);
    }

    if (jsonObject.find(OUTS) == jsonObject.end()) {
        AVTRANS_LOGE("AudioEncoder OUTS is invalid");
        return;
    }
    nlohmann::json audioEncoderOutsJson = jsonObject[OUTS];
    for (const auto &outJson : audioEncoderOutsJson) {
        AudioEncoderOut out;
        FromJson(outJson, out);
        audioEncoder.outs.push_back(out);
    }
}

std::vector<AudioDecoderIn> ParseAudioDecoderIn(CapabilitySet &inCaps)
{
    std::vector<AudioDecoderIn> ins;
    for (auto &cap : inCaps) {
        AVTRANS_LOGD("AudioDecoderIn Raw: %{public}s", OHOS::Media::Pipeline::Capability2String(cap).c_str());
        AudioDecoderIn in;
        in.mime = cap.mime;
        in.channel_layout = AnyCast<std::vector<AudioChannelLayout>>(cap.keys[CapabilityID::AUDIO_CHANNEL_LAYOUT]);
        ins.push_back(in);
    }

    return ins;
}

std::vector<AudioDecoderOut> ParseAudioDecoderOut(CapabilitySet &outCaps)
{
    std::vector<AudioDecoderOut> outs;
    for (auto &cap : outCaps) {
        AVTRANS_LOGD("AudioDecoderOut Raw: %{public}s", OHOS::Media::Pipeline::Capability2String(cap).c_str());
        AudioDecoderOut out;
        out.mime = cap.mime;
        out.sample_fmt = AnyCast<std::vector<AudioSampleFormat>>(cap.keys[CapabilityID::AUDIO_SAMPLE_FORMAT]);
        outs.push_back(out);
    }

    return outs;
}

std::vector<AudioDecoder> QueryAudioDecoderAbility()
{
    std::vector<AudioDecoder> audioDecoders;
    auto audioDecNameList = PluginManager::Instance().ListPlugins(PluginType::AUDIO_DECODER);
    for (const std::string &name : audioDecNameList) {
        if (find(AUDIO_DECODER_WANT.begin(), AUDIO_DECODER_WANT.end(), name) == AUDIO_DECODER_WANT.end()) {
            AVTRANS_LOGI("AudioDecoder Plugin not want: %{public}s", name.c_str());
            continue;
        }
        auto pluginInfo = PluginManager::Instance().GetPluginInfo(PluginType::AUDIO_DECODER, name);
        AudioDecoder audioDecoder;
        audioDecoder.name = name;
        audioDecoder.ins = ParseAudioDecoderIn(pluginInfo->inCaps);
        audioDecoder.outs = ParseAudioDecoderOut(pluginInfo->outCaps);
        audioDecoders.push_back(audioDecoder);
    }

    return audioDecoders;
}

void ToJson(nlohmann::json &jsonObject, const AudioDecoderIn &audioDecoderIn)
{
    jsonObject[MIME] = audioDecoderIn.mime;
    nlohmann::json channelLayoutJson;
    for (auto &layout : audioDecoderIn.channel_layout) {
        channelLayoutJson.push_back((uint64_t)layout);
    }
    jsonObject[AUDIO_CHANNEL_LAYOUT] = channelLayoutJson;
}

void FromJson(const nlohmann::json &jsonObject, AudioDecoderIn &audioDecoderIn)
{
    if (!IsString(jsonObject, MIME)) {
        AVTRANS_LOGE("AudioDecoderIn MIME is invalid");
        return;
    }
    audioDecoderIn.mime = jsonObject.at(MIME).get<std::string>();

    if (jsonObject.find(AUDIO_CHANNEL_LAYOUT) == jsonObject.end()) {
        AVTRANS_LOGE("AudioEncoder AUDIO_CHANNEL_LAYOUT is invalid");
        return;
    }
    nlohmann::json channelLayoutJson = jsonObject[AUDIO_CHANNEL_LAYOUT];
    for (auto layout : channelLayoutJson) {
        audioDecoderIn.channel_layout.push_back((AudioChannelLayout)layout);
    }
}

void ToJson(nlohmann::json &jsonObject, const AudioDecoderOut &audioDecoderOut)
{
    jsonObject[MIME] = audioDecoderOut.mime;
    nlohmann::json sampleFormatsJson;
    for (auto &sampleFormat : audioDecoderOut.sample_fmt) {
        sampleFormatsJson.push_back((uint8_t)sampleFormat);
    }
    jsonObject[AUDIO_SAMPLE_FORMAT] = sampleFormatsJson;
}

void FromJson(const nlohmann::json &jsonObject, AudioDecoderOut &audioDecoderOut)
{
    if (!IsString(jsonObject, MIME)) {
        AVTRANS_LOGE("AudioDecoderOut MIME is invalid");
        return;
    }
    audioDecoderOut.mime = jsonObject.at(MIME).get<std::string>();

    if (jsonObject.find(AUDIO_SAMPLE_FORMAT) == jsonObject.end()) {
        AVTRANS_LOGE("AudioDecoderOut AUDIO_SAMPLE_FORMAT is invalid");
        return;
    }

    for (auto sampleFormatJson : jsonObject[AUDIO_SAMPLE_FORMAT]) {
        audioDecoderOut.sample_fmt.push_back((AudioSampleFormat)sampleFormatJson);
    }
}

void ToJson(nlohmann::json &jsonObject, const AudioDecoder &audioDecoder)
{
    jsonObject[NAME] = audioDecoder.name;
    nlohmann::json audioDecoderInsJson;
    for (const auto &in : audioDecoder.ins) {
        nlohmann::json audioDecoderInJson;
        ToJson(audioDecoderInJson, in);
        audioDecoderInsJson.push_back(audioDecoderInJson);
    }
    jsonObject[INS] = audioDecoderInsJson;

    nlohmann::json audioDecoderOutsJson;
    for (const auto &out : audioDecoder.outs) {
        nlohmann::json audioDecoderOutJson;
        ToJson(audioDecoderOutJson, out);
        audioDecoderOutsJson.push_back(audioDecoderOutJson);
    }
    jsonObject[OUTS] = audioDecoderOutsJson;
}

void FromJson(const nlohmann::json &jsonObject, AudioDecoder &audioDecoder)
{
    if (!IsString(jsonObject, NAME)) {
        AVTRANS_LOGE("AudioDecoder NAME is invalid");
        return;
    }
    audioDecoder.name = jsonObject.at(NAME).get<std::string>();

    if (jsonObject.find(INS) == jsonObject.end()) {
        AVTRANS_LOGE("AudioDecoder INS is invalid");
        return;
    }

    nlohmann::json audioDecoderInsJson = jsonObject[INS];
    for (const auto &inJson : audioDecoderInsJson) {
        AudioDecoderIn in;
        FromJson(inJson, in);
        audioDecoder.ins.push_back(in);
    }

    if (jsonObject.find(OUTS) == jsonObject.end()) {
        AVTRANS_LOGE("AudioDecoder OUTS is invalid");
        return;
    }
    nlohmann::json audioDecoderOutsJson = jsonObject[OUTS];
    for (const auto &outJson : audioDecoderOutsJson) {
        AudioDecoderOut out;
        FromJson(outJson, out);
        audioDecoder.outs.push_back(out);
    }
}

std::vector<VideoEncoderIn> ParseVideoEncoderIn(CapabilitySet &inCaps)
{
    std::vector<VideoEncoderIn> ins;
    for (auto &cap : inCaps) {
        AVTRANS_LOGD("VideoEncoderIn Raw: %{public}s", OHOS::Media::Pipeline::Capability2String(cap).c_str());
        VideoEncoderIn in;
        in.mime = cap.mime;
        in.pixel_fmt = AnyCast<std::vector<VideoPixelFormat>>(cap.keys[CapabilityID::VIDEO_PIXEL_FORMAT]);
        ins.push_back(in);
    }

    return ins;
}

std::vector<VideoEncoderOut> ParseVideoEncoderOut(CapabilitySet &outCaps)
{
    std::vector<VideoEncoderOut> outs;
    for (auto &cap : outCaps) {
        AVTRANS_LOGD("VideoEncoderOut Raw: %{public}s", OHOS::Media::Pipeline::Capability2String(cap).c_str());
        VideoEncoderOut out;
        out.mime = cap.mime;
        outs.push_back(out);
    }

    return outs;
}

std::vector<VideoEncoder> QueryVideoEncoderAbility()
{
    std::vector<VideoEncoder> videoEncoders;
    auto videoEncNameList = PluginManager::Instance().ListPlugins(PluginType::VIDEO_ENCODER);
    for (const std::string& name : videoEncNameList) {
        if (find(VIDEO_ENCODER_WANT.begin(), VIDEO_ENCODER_WANT.end(), name) == VIDEO_ENCODER_WANT.end()) {
            AVTRANS_LOGI("VideoEncoder Plugin not want: %{public}s", name.c_str());
            continue;
        }
        auto pluginInfo = PluginManager::Instance().GetPluginInfo(PluginType::VIDEO_ENCODER, name);
        VideoEncoder videoEncoder;
        videoEncoder.name = name;
        videoEncoder.ins = ParseVideoEncoderIn(pluginInfo->inCaps);
        videoEncoder.outs = ParseVideoEncoderOut(pluginInfo->outCaps);
        videoEncoders.push_back(videoEncoder);
    }

    return videoEncoders;
}

void ToJson(nlohmann::json &jsonObject, const VideoEncoderIn &videoEncoderIn)
{
    jsonObject[MIME] = videoEncoderIn.mime;
    nlohmann::json pixelFmtJson;
    for (auto &fmt : videoEncoderIn.pixel_fmt) {
        pixelFmtJson.push_back((uint32_t)fmt);
    }
    jsonObject[VIDEO_PIXEL_FMT] = pixelFmtJson;
}

void FromJson(const nlohmann::json &jsonObject, VideoEncoderIn &videoEncoderIn)
{
    if (!IsString(jsonObject, MIME)) {
        AVTRANS_LOGE("VideoEncoderIn MIME is invalid");
        return;
    }
    videoEncoderIn.mime = jsonObject.at(MIME).get<std::string>();

    if (jsonObject.find(VIDEO_PIXEL_FMT) == jsonObject.end()) {
        AVTRANS_LOGE("VideoEncoderIn VIDEO_PIXEL_FMT is invalid");
        return;
    }
    for (auto fmt : jsonObject[VIDEO_PIXEL_FMT]) {
        videoEncoderIn.pixel_fmt.push_back((VideoPixelFormat)fmt);
    }
}

void ToJson(nlohmann::json &jsonObject, const VideoEncoderOut &videoEncoderOut)
{
    jsonObject[MIME] = videoEncoderOut.mime;
}

void FromJson(const nlohmann::json &jsonObject, VideoEncoderOut &videoEncoderOut)
{
    if (!IsString(jsonObject, MIME)) {
        AVTRANS_LOGE("VideoEncoderOut MIME is invalid");
        return;
    }
    videoEncoderOut.mime = jsonObject[MIME].get<std::string>();
}

void ToJson(nlohmann::json &jsonObject, const VideoEncoder &videoEncoder)
{
    jsonObject[NAME] = videoEncoder.name;
    nlohmann::json videoEncoderInsJson;
    for (const auto &in : videoEncoder.ins) {
        nlohmann::json videoEncoderInJson;
        ToJson(videoEncoderInJson, in);
        videoEncoderInsJson.push_back(videoEncoderInJson);
    }
    jsonObject[INS] = videoEncoderInsJson;

    nlohmann::json videoEncoderOutsJson;
    for (const auto &out : videoEncoder.outs) {
        nlohmann::json videoEncoderOutJson;
        ToJson(videoEncoderOutJson, out);
        videoEncoderOutsJson.push_back(videoEncoderOutJson);
    }
    jsonObject[OUTS] = videoEncoderOutsJson;
}

void FromJson(const nlohmann::json &jsonObject, VideoEncoder &videoEncoder)
{
    if (!IsString(jsonObject, NAME)) {
        AVTRANS_LOGE("VideoEncoder NAME is invalid");
        return;
    }
    videoEncoder.name = jsonObject.at(NAME).get<std::string>();

    if (jsonObject.find(INS) == jsonObject.end()) {
        AVTRANS_LOGE("VideoEncoder INS is invalid");
        return;
    }

    nlohmann::json videoEncoderInsJson = jsonObject[INS];
    for (const auto &inJson : videoEncoderInsJson) {
        VideoEncoderIn in;
        FromJson(inJson, in);
        videoEncoder.ins.push_back(in);
    }

    if (jsonObject.find(OUTS) == jsonObject.end()) {
        AVTRANS_LOGE("VideoEncoder OUTS is invalid");
        return;
    }
    nlohmann::json videoEncoderOutsJson = jsonObject[OUTS];
    for (const auto &outJson : videoEncoderOutsJson) {
        VideoEncoderOut out;
        FromJson(outJson, out);
        videoEncoder.outs.push_back(out);
    }
}

std::vector<VideoDecoderIn> ParseVideoDecoderIn(CapabilitySet &inCaps)
{
    std::vector<VideoDecoderIn> ins;
    for (auto &cap : inCaps) {
        AVTRANS_LOGD("VideoDecoderIn Raw: %{public}s", OHOS::Media::Pipeline::Capability2String(cap).c_str());
        VideoDecoderIn in;
        in.mime = cap.mime;
        in.vd_bit_stream_fmt =
            AnyCast<std::vector<VideoBitStreamFormat>>(cap.keys[CapabilityID::VIDEO_BIT_STREAM_FORMAT]);
        ins.push_back(in);
    }

    return ins;
}

std::vector<VideoDecoderOut> ParseVideoDecoderOut(CapabilitySet &outCaps)
{
    std::vector<VideoDecoderOut> outs;
    for (auto &cap : outCaps) {
        AVTRANS_LOGD("VideoDecoderOut Raw: %{public}s", OHOS::Media::Pipeline::Capability2String(cap).c_str());
        VideoDecoderOut out;
        out.mime = cap.mime;
        out.pixel_fmt = AnyCast<std::vector<VideoPixelFormat>>(cap.keys[CapabilityID::VIDEO_PIXEL_FORMAT]);
        outs.push_back(out);
    }

    return outs;
}

std::vector<VideoDecoder> QueryVideoDecoderAbility()
{
    std::vector<VideoDecoder> VideoDecoders;
    auto videoDecNameList = PluginManager::Instance().ListPlugins(PluginType::VIDEO_DECODER);
    for (const std::string& name : videoDecNameList) {
        if (find(VIDEO_DECODER_WANT.begin(), VIDEO_DECODER_WANT.end(), name) == VIDEO_DECODER_WANT.end()) {
            AVTRANS_LOGI("VideoDecoder Plugin not want: %{public}s", name.c_str());
            continue;
        }
        auto pluginInfo = PluginManager::Instance().GetPluginInfo(PluginType::VIDEO_DECODER, name);
        VideoDecoder videoDecoder;
        videoDecoder.name = name;
        videoDecoder.ins = ParseVideoDecoderIn(pluginInfo->inCaps);
        videoDecoder.outs = ParseVideoDecoderOut(pluginInfo->outCaps);
        VideoDecoders.push_back(videoDecoder);
    }

    return VideoDecoders;
}

void ToJson(nlohmann::json &jsonObject, const VideoDecoderIn &videoDecoderIn)
{
    jsonObject[MIME] = videoDecoderIn.mime;
    nlohmann::json fmtJson;
    for (auto &fmt : videoDecoderIn.vd_bit_stream_fmt) {
        fmtJson.push_back((uint32_t)fmt);
    }
    jsonObject[VIDEO_BIT_STREAM_FMT] = fmtJson;
}

void FromJson(const nlohmann::json &jsonObject, VideoDecoderIn &videoDecoderIn)
{
    if (!IsString(jsonObject, MIME)) {
        AVTRANS_LOGE("VideoDecoderIn MIME is invalid");
        return;
    }
    videoDecoderIn.mime = jsonObject.at(MIME).get<std::string>();

    if (jsonObject.find(VIDEO_BIT_STREAM_FMT) == jsonObject.end()) {
        AVTRANS_LOGE("VideoDecoderIn VIDEO_BIT_STREAM_FMT is invalid");
        return;
    }
    for (auto fmt : jsonObject[VIDEO_BIT_STREAM_FMT]) {
        videoDecoderIn.vd_bit_stream_fmt.push_back((VideoBitStreamFormat)fmt);
    }
}

void ToJson(nlohmann::json &jsonObject, const VideoDecoderOut &videoDecoderOut)
{
    jsonObject[MIME] = videoDecoderOut.mime;
    nlohmann::json fmtJson;
    for (auto &fmt : videoDecoderOut.pixel_fmt) {
        fmtJson.push_back((uint32_t)fmt);
    }
    jsonObject[VIDEO_PIXEL_FMT] = fmtJson;
}

void FromJson(const nlohmann::json &jsonObject, VideoDecoderOut &videoDecoderOut)
{
    if (!IsString(jsonObject, MIME)) {
        AVTRANS_LOGE("VideoDecoderOut MIME is invalid");
        return;
    }
    videoDecoderOut.mime = jsonObject.at(MIME).get<std::string>();

    if (jsonObject.find(VIDEO_PIXEL_FMT) == jsonObject.end()) {
        AVTRANS_LOGE("VideoDecoderOut VIDEO_PIXEL_FMT is invalid");
        return;
    }
    for (auto fmt : jsonObject[VIDEO_PIXEL_FMT]) {
        videoDecoderOut.pixel_fmt.push_back((VideoPixelFormat)fmt);
    }
}

void ToJson(nlohmann::json &jsonObject, const VideoDecoder &videoDecoder)
{
    jsonObject[NAME] = videoDecoder.name;
    nlohmann::json videoDecoderInsJson;
    for (const auto &in : videoDecoder.ins) {
        nlohmann::json videoDecoderInJson;
        ToJson(videoDecoderInJson, in);
        videoDecoderInsJson.push_back(videoDecoderInJson);
    }
    jsonObject[INS] = videoDecoderInsJson;

    nlohmann::json videoDecoderOutsJson;
    for (const auto &out : videoDecoder.outs) {
        nlohmann::json videoDecoderOutJson;
        ToJson(videoDecoderOutJson, out);
        videoDecoderOutsJson.push_back(videoDecoderOutJson);
    }
    jsonObject[OUTS] = videoDecoderOutsJson;
}

void FromJson(const nlohmann::json &jsonObject, VideoDecoder &videoDecoder)
{
    if (!IsString(jsonObject, NAME)) {
        AVTRANS_LOGE("VideoDecoder NAME is invalid");
        return;
    }
    videoDecoder.name = jsonObject.at(NAME).get<std::string>();

    if (jsonObject.find(INS) == jsonObject.end()) {
        AVTRANS_LOGE("VideoDecoder INS is invalid");
        return;
    }

    nlohmann::json videoDecoderInsJson = jsonObject[INS];
    for (const auto &inJson : videoDecoderInsJson) {
        VideoDecoderIn in;
        FromJson(inJson, in);
        videoDecoder.ins.push_back(in);
    }

    if (jsonObject.find(OUTS) == jsonObject.end()) {
        AVTRANS_LOGE("VideoDecoder OUTS is invalid");
        return;
    }
    nlohmann::json videoDecoderOutsJson = jsonObject[OUTS];
    for (const auto &outJson : videoDecoderOutsJson) {
        VideoDecoderOut out;
        FromJson(outJson, out);
        videoDecoder.outs.push_back(out);
    }
}

template<typename T>
void ToJson(const std::string &key, nlohmann::json &jsonObject, std::vector<T> &objs)
{
    nlohmann::json jsonObjs;
    for (auto &obj : objs) {
        nlohmann::json jsonObj;
        ToJson(jsonObj, obj);
        jsonObjs.push_back(jsonObj);
    }
    jsonObject[key] = jsonObjs;
}

template<typename T>
void FromJson(const std::string &key, const nlohmann::json &jsonObject, std::vector<T> &objs)
{
    if (jsonObject.find(key) == jsonObject.end()) {
        AVTRANS_LOGE("JSONObject key invalid, key: %{public}s", key.c_str());
        return;
    }
    for (const auto &json : jsonObject[key]) {
        T obj;
        FromJson(json, obj);
        objs.push_back(obj);
    }
}

int32_t QueryAudioEncoderAbilityStr(char* res)
{
    std::vector<AudioEncoder> audioEncoders = QueryAudioEncoderAbility();
    nlohmann::json jsonObject;
    ToJson<AudioEncoder>(AUDIO_ENCODERS, jsonObject, audioEncoders);
    std::string ret = jsonObject.dump();
    AVTRANS_LOGI("QueryAudioEncoderAbilityStr ret: %{public}s", ret.c_str());
    if (ret.length() > MAX_MESSAGES_LEN) {
        AVTRANS_LOGE("QueryAudioEncoderAbilityStr too long");
        return 0;
    }
    if (memcpy_s(res, MAX_MESSAGES_LEN, ret.c_str(), ret.length()) != EOK) {
        return 0;
    }
    return ret.length();
}

int32_t QueryAudioDecoderAbilityStr(char* res)
{
    std::vector<AudioDecoder> audioDecoders = QueryAudioDecoderAbility();
    nlohmann::json jsonObject;
    ToJson<AudioDecoder>(AUDIO_DECODERS, jsonObject, audioDecoders);
    std::string ret = jsonObject.dump();
    AVTRANS_LOGI("QueryAudioDecoderAbilityStr ret: %{public}s", ret.c_str());
    if (ret.length() > MAX_MESSAGES_LEN) {
        AVTRANS_LOGE("QueryAudioDecoderAbilityStr too long");
        return 0;
    }
    if (memcpy_s(res, MAX_MESSAGES_LEN, ret.c_str(), ret.length()) != EOK) {
        return 0;
    }
    return ret.length();
}

int32_t QueryVideoEncoderAbilityStr(char* res)
{
    std::vector<VideoEncoder> viudeoEncoders = QueryVideoEncoderAbility();
    nlohmann::json jsonObject;
    ToJson<VideoEncoder>(VIDEO_ENCODERS, jsonObject, viudeoEncoders);
    std::string ret = jsonObject.dump();
    AVTRANS_LOGI("QueryVideoEncoderAbilityStr ret: %{public}s", ret.c_str());
    if (ret.length() > MAX_MESSAGES_LEN) {
        AVTRANS_LOGE("QueryVideoEncoderAbilityStr too long");
        return 0;
    }
    if (memcpy_s(res, MAX_MESSAGES_LEN, ret.c_str(), ret.length()) != EOK) {
        return 0;
    }
    return ret.length();
}

int32_t QueryVideoDecoderAbilityStr(char* res)
{
    std::vector<VideoDecoder> videoDecoders = QueryVideoDecoderAbility();
    nlohmann::json jsonObject;
    ToJson<VideoDecoder>(VIDEO_DECODERS, jsonObject, videoDecoders);
    std::string ret = jsonObject.dump();
    AVTRANS_LOGI("QueryVideoDecoderAbilityStr ret: %{public}s", ret.c_str());
    if (ret.length() > MAX_MESSAGES_LEN) {
        AVTRANS_LOGE("QueryVideoDecoderAbilityStr too long");
        return 0;
    }
    if (memcpy_s(res, MAX_MESSAGES_LEN, ret.c_str(), ret.length()) != EOK) {
        return 0;
    }
    return ret.length();
}
}
}