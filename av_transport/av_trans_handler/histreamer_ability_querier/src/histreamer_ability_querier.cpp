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

bool IsString(const cJSON *jsonObj, const std::string &key)
{
    cJSON *keyObj = cJSON_GetObjectItemCaseSensitive(jsonObj, key.c_str());
    bool res = (keyObj != nullptr) && cJSON_IsString(keyObj) &&
        strlen(cJSON_GetStringValue(keyObj)) <= MAX_MESSAGES_LEN;
    if (!res) {
        AVTRANS_LOGE("the key %{public}s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsUInt8(const cJSON *jsonObj, const std::string &key)
{
    cJSON *keyObj = cJSON_GetObjectItemCaseSensitive(jsonObj, key.c_str());
    bool res = (keyObj != nullptr) && cJSON_IsNumber(keyObj) &&
        static_cast<uint8_t>(keyObj->valueint) <= UINT8_MAX;
    return res;
}

bool IsUInt32(const cJSON *jsonObj, const std::string &key)
{
    cJSON *keyObj = cJSON_GetObjectItemCaseSensitive(jsonObj, key.c_str());
    bool res = (keyObj != nullptr) && cJSON_IsNumber(keyObj) &&
        static_cast<uint32_t>(keyObj->valueint) <= UINT32_MAX;
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

void ToJson(cJSON *jsonObject, const AudioEncoderIn &audioEncoderIn)
{
    if (jsonObject == nullptr) {
        return;
    }
    cJSON_AddStringToObject(jsonObject, MIME.c_str(), audioEncoderIn.mime.c_str());
    cJSON *sampleRateJson = cJSON_CreateArray();
    if (sampleRateJson == nullptr) {
        return;
    }
    for (auto rate : audioEncoderIn.sample_rate) {
        cJSON_AddItemToArray(sampleRateJson, cJSON_CreateNumber(rate));
    }
    cJSON_AddItemToObject(jsonObject, SAMPLE_RATE.c_str(), sampleRateJson);
}

void FromJson(const cJSON *jsonObject, AudioEncoderIn &audioEncoderIn)
{
    if (!IsString(jsonObject, MIME)) {
        AVTRANS_LOGE("AudioEncoderIn MIME is invalid!");
        return;
    }
    cJSON *mimeObj = cJSON_GetObjectItemCaseSensitive(jsonObject, MIME.c_str());
    if (mimeObj == nullptr || !cJSON_IsString(mimeObj)) {
        return;
    }
    audioEncoderIn.mime = mimeObj->valuestring;
    cJSON *rateJson = cJSON_GetObjectItemCaseSensitive(jsonObject, SAMPLE_RATE.c_str());
    if (rateJson == nullptr || !cJSON_IsArray(rateJson)) {
        AVTRANS_LOGE("AudioEncoderIn SAMPLE_RATE is invalid");
        return;
    }
    cJSON *rateInfo = nullptr;
    cJSON_ArrayForEach(rateInfo, rateJson) {
        if (rateInfo->type == cJSON_Number) {
            audioEncoderIn.sample_rate.push_back(static_cast<uint32_t>(rateInfo->valueint));
        }
    }
}

void ToJson(cJSON *jsonObject, const AudioEncoderOut &audioEncoderOut)
{
    if (jsonObject == nullptr) {
        return;
    }
    cJSON_AddStringToObject(jsonObject, MIME.c_str(), audioEncoderOut.mime.c_str());
    cJSON_AddNumberToObject(jsonObject, AD_MPEG_VER.c_str(), static_cast<uint8_t>(audioEncoderOut.ad_mpeg_ver));
    cJSON_AddNumberToObject(jsonObject, AUDIO_AAC_PROFILE.c_str(), static_cast<uint8_t>(audioEncoderOut.aac_profile));
    cJSON_AddNumberToObject(jsonObject, AUDIO_AAC_STREAM_FORMAT.c_str(),
        static_cast<uint8_t>(audioEncoderOut.aac_stm_fmt));
}

void FromJson(const cJSON *jsonObject, AudioEncoderOut &audioEncoderOut)
{
    if (jsonObject == nullptr) {
        return;
    }
    if (!IsString(jsonObject, MIME)) {
        AVTRANS_LOGE("AudioEncoderOut MIME is invalid!");
        return;
    }
    cJSON *mimeObj = cJSON_GetObjectItemCaseSensitive(jsonObject, MIME.c_str());
    if (mimeObj == nullptr || !cJSON_IsString(mimeObj)) {
        return;
    }
    audioEncoderOut.mime = mimeObj->valuestring;
    if (!IsUInt32(jsonObject, AD_MPEG_VER)) {
        AVTRANS_LOGE("AudioEncoderOut AD_MPEG_VER is invalid");
        return;
    }
    cJSON *verObj = cJSON_GetObjectItemCaseSensitive(jsonObject, AD_MPEG_VER.c_str());
    if (verObj == nullptr || !cJSON_IsNumber(verObj)) {
        return;
    }
    audioEncoderOut.ad_mpeg_ver = verObj->valueint;

    if (!IsUInt8(jsonObject, AUDIO_AAC_PROFILE)) {
        AVTRANS_LOGE("AudioEncoderOut AUDIO_AAC_PROFILE is invalid");
        return;
    }
    cJSON *accObj = cJSON_GetObjectItemCaseSensitive(jsonObject, AUDIO_AAC_PROFILE.c_str());
    if (accObj == nullptr || !cJSON_IsNumber(accObj)) {
        return;
    }
    audioEncoderOut.aac_profile = static_cast<AudioAacProfile>(accObj->valueint);
    if (!IsUInt8(jsonObject, AUDIO_AAC_STREAM_FORMAT)) {
        AVTRANS_LOGE("AudioEncoderOut AUDIO_AAC_STREAM_FORMAT is invalid");
        return;
    }
    cJSON *formatObj = cJSON_GetObjectItemCaseSensitive(jsonObject, AUDIO_AAC_STREAM_FORMAT.c_str());
    if (formatObj == nullptr || !cJSON_IsNumber(formatObj)) {
        return;
    }
    audioEncoderOut.aac_stm_fmt = static_cast<AudioAacStreamFormat>(formatObj->valueint);
}

void ToJson(cJSON *jsonObject, const AudioEncoder &audioEncoder)
{
    if (jsonObject == nullptr) {
        return;
    }
    cJSON_AddStringToObject(jsonObject, NAME.c_str(), audioEncoder.name.c_str());
    cJSON *audioEncoderInsJson = cJSON_CreateArray();
    if (audioEncoderInsJson == nullptr) {
        return;
    }
    for (const auto &in : audioEncoder.ins) {
        cJSON *audioEncoderInJson = cJSON_CreateObject();
        if (audioEncoderInJson == nullptr) {
            cJSON_Delete(audioEncoderInsJson);
            return;
        }
        ToJson(audioEncoderInJson, in);
        cJSON_AddItemToArray(audioEncoderInsJson, audioEncoderInJson);
    }
    cJSON_AddItemToObject(jsonObject, INS.c_str(), audioEncoderInsJson);

    cJSON *audioEncoderOutsJson = cJSON_CreateArray();
    if (audioEncoderOutsJson == nullptr) {
        return;
    }
    for (const auto &out : audioEncoder.outs) {
        cJSON *audioEncoderOutJson = cJSON_CreateObject();
        if (audioEncoderOutJson == nullptr) {
            cJSON_Delete(audioEncoderOutsJson);
            return;
        }
        ToJson(audioEncoderOutJson, out);
        cJSON_AddItemToArray(audioEncoderOutsJson, audioEncoderOutJson);
    }
    cJSON_AddItemToObject(jsonObject, OUTS.c_str(), audioEncoderOutsJson);
}

void FromJson(const cJSON *jsonObject, AudioEncoder &audioEncoder)
{
    if (!IsString(jsonObject, NAME)) {
        AVTRANS_LOGE("AudioEncoder NAME is invalid");
        return;
    }
    cJSON *nameObj = cJSON_GetObjectItemCaseSensitive(jsonObject, NAME.c_str());
    if (nameObj == nullptr || !cJSON_IsString(nameObj)) {
        return;
    }
    audioEncoder.name = nameObj->valuestring;

    cJSON *audioEncoderInsJson = cJSON_GetObjectItemCaseSensitive(jsonObject, INS.c_str());
    if (audioEncoderInsJson == nullptr || !cJSON_IsArray(audioEncoderInsJson)) {
        AVTRANS_LOGE("AudioEncoder INS is invalid");
        return;
    }
    cJSON *inJson = nullptr;
    cJSON_ArrayForEach(inJson, audioEncoderInsJson) {
        AudioEncoderIn in;
        FromJson(inJson, in);
        audioEncoder.ins.push_back(in);
    }

    cJSON *audioEncoderOutsJson = cJSON_GetObjectItemCaseSensitive(jsonObject, OUTS.c_str());
    if (audioEncoderOutsJson == nullptr || !cJSON_IsArray(audioEncoderOutsJson)) {
        AVTRANS_LOGE("AudioEncoder OUTS is invalid");
        return;
    }
    cJSON *outJson = nullptr;
    cJSON_ArrayForEach(outJson, audioEncoderOutsJson) {
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

void ToJson(cJSON *jsonObject, const AudioDecoderIn &audioDecoderIn)
{
    if (jsonObject == nullptr) {
        return;
    }
    cJSON_AddStringToObject(jsonObject, MIME.c_str(), audioDecoderIn.mime.c_str());
    cJSON *channelLayoutJson = cJSON_CreateArray();
    if (channelLayoutJson == nullptr) {
        return;
    }
    for (auto &layout : audioDecoderIn.channel_layout) {
        cJSON_AddItemToArray(channelLayoutJson, cJSON_CreateNumber(static_cast<uint64_t>(layout)));
    }
    cJSON_AddItemToObject(jsonObject, AUDIO_CHANNEL_LAYOUT.c_str(), channelLayoutJson);
}

void FromJson(const cJSON *jsonObject, AudioDecoderIn &audioDecoderIn)
{
    if (!IsString(jsonObject, MIME)) {
        AVTRANS_LOGE("AudioDecoderIn MIME is invalid");
        return;
    }
    cJSON *mimeObj = cJSON_GetObjectItemCaseSensitive(jsonObject, MIME.c_str());
    if (mimeObj == nullptr || !cJSON_IsString(mimeObj)) {
        return;
    }
    audioDecoderIn.mime = mimeObj->valuestring;

    cJSON *channelLayoutJson = cJSON_GetObjectItemCaseSensitive(jsonObject, AUDIO_CHANNEL_LAYOUT.c_str());
    if (channelLayoutJson == nullptr || !cJSON_IsArray(channelLayoutJson)) {
        AVTRANS_LOGE("AudioEncoder AUDIO_CHANNEL_LAYOUT is invalid");
        return;
    }
    cJSON *layoutInfo = nullptr;
    cJSON_ArrayForEach(layoutInfo, channelLayoutJson) {
        if (layoutInfo->type == cJSON_Number) {
            audioDecoderIn.channel_layout.push_back(static_cast<AudioChannelLayout>(layoutInfo->valueint));
        }
    }
}

void ToJson(cJSON *jsonObject, const AudioDecoderOut &audioDecoderOut)
{
    if (jsonObject == nullptr) {
        return;
    }
    cJSON_AddStringToObject(jsonObject, MIME.c_str(), audioDecoderOut.mime.c_str());
    cJSON *sampleFormatsJson = cJSON_CreateArray();
    if (sampleFormatsJson == nullptr) {
        return;
    }
    for (auto &sampleFormat : audioDecoderOut.sample_fmt) {
        cJSON_AddItemToArray(sampleFormatsJson, cJSON_CreateNumber(static_cast<uint8_t>(sampleFormat)));
    }
    cJSON_AddItemToObject(jsonObject, AUDIO_SAMPLE_FORMAT.c_str(), sampleFormatsJson);
}

void FromJson(const cJSON *jsonObject, AudioDecoderOut &audioDecoderOut)
{
    if (!IsString(jsonObject, MIME)) {
        AVTRANS_LOGE("AudioDecoderOut MIME is invalid");
        return;
    }
    cJSON *mimeObj = cJSON_GetObjectItemCaseSensitive(jsonObject, MIME.c_str());
    if (mimeObj == nullptr || !cJSON_IsString(mimeObj)) {
        return;
    }
    audioDecoderOut.mime = mimeObj->valuestring;

    cJSON *sampleFormatJson = cJSON_GetObjectItemCaseSensitive(jsonObject, AUDIO_SAMPLE_FORMAT.c_str());
    if (sampleFormatJson == nullptr || !cJSON_IsArray(sampleFormatJson)) {
        AVTRANS_LOGE("AudioDecoderOut AUDIO_SAMPLE_FORMAT is invalid");
        return;
    }

    cJSON *sampleInfo = nullptr;
    cJSON_ArrayForEach(sampleInfo, sampleFormatJson) {
        if (sampleInfo->type == cJSON_Number) {
            audioDecoderOut.sample_fmt.push_back(static_cast<AudioSampleFormat>(sampleInfo->valueint));
        }
    }
}

void ToJson(cJSON *jsonObject, const AudioDecoder &audioDecoder)
{
    if (jsonObject == nullptr) {
        return;
    }
    cJSON_AddStringToObject(jsonObject, NAME.c_str(), audioDecoder.name.c_str());
    cJSON *audioDecoderInsJson = cJSON_CreateArray();
    if (audioDecoderInsJson == nullptr) {
        return;
    }

    for (const auto &in : audioDecoder.ins) {
        cJSON *audioDecoderInJson = cJSON_CreateObject();
        if (audioDecoderInJson == nullptr) {
            cJSON_Delete(audioDecoderInsJson);
            return;
        }
        ToJson(audioDecoderInJson, in);
        cJSON_AddItemToArray(audioDecoderInsJson, audioDecoderInJson);
    }
    cJSON_AddItemToObject(jsonObject, INS.c_str(), audioDecoderInsJson);

    cJSON *audioDecoderOutsJson = cJSON_CreateArray();
    if (audioDecoderOutsJson == nullptr) {
        return;
    }
    for (const auto &out : audioDecoder.outs) {
        cJSON *audioDecoderOutJson = cJSON_CreateObject();
        if (audioDecoderOutJson == nullptr) {
            cJSON_Delete(audioDecoderOutsJson);
            return;
        }
        ToJson(audioDecoderOutJson, out);
        cJSON_AddItemToArray(audioDecoderOutsJson, audioDecoderOutJson);
    }
    cJSON_AddItemToObject(jsonObject, OUTS.c_str(), audioDecoderOutsJson);
}

void FromJson(const cJSON *jsonObject, AudioDecoder &audioDecoder)
{
    if (!IsString(jsonObject, NAME)) {
        AVTRANS_LOGE("AudioDecoder NAME is invalid");
        return;
    }
    cJSON *nameObj = cJSON_GetObjectItemCaseSensitive(jsonObject, NAME.c_str());
    if (nameObj == nullptr || !cJSON_IsString(nameObj)) {
        return;
    }
    audioDecoder.name = nameObj->valuestring;

    cJSON *audioDecoderInsJson = cJSON_GetObjectItemCaseSensitive(jsonObject, INS.c_str());
    if (audioDecoderInsJson == nullptr || !cJSON_IsArray(audioDecoderInsJson)) {
        AVTRANS_LOGE("AudioDecoder INS is invalid");
        return;
    }
    cJSON *inJson = nullptr;
    cJSON_ArrayForEach(inJson, audioDecoderInsJson) {
        AudioDecoderIn in;
        FromJson(inJson, in);
        audioDecoder.ins.push_back(in);
    }
    cJSON *audioDecoderOutsJson = cJSON_GetObjectItemCaseSensitive(jsonObject, OUTS.c_str());
    if (audioDecoderOutsJson == nullptr || !cJSON_IsArray(audioDecoderOutsJson)) {
        AVTRANS_LOGE("AudioDecoder OUTS is invalid");
        return;
    }
    cJSON *outJson = nullptr;
    cJSON_ArrayForEach(outJson, audioDecoderOutsJson) {
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

void ToJson(cJSON *jsonObject, const VideoEncoderIn &videoEncoderIn)
{
    if (jsonObject == nullptr) {
        return;
    }
    cJSON_AddStringToObject(jsonObject, MIME.c_str(), videoEncoderIn.mime.c_str());
    cJSON *pixelFmtJson = cJSON_CreateArray();
    if (pixelFmtJson == nullptr) {
        return;
    }
    for (auto &fmt : videoEncoderIn.pixel_fmt) {
        cJSON_AddItemToArray(pixelFmtJson, cJSON_CreateNumber(static_cast<uint32_t>(fmt)));
    }
    cJSON_AddItemToObject(jsonObject, VIDEO_PIXEL_FMT.c_str(), pixelFmtJson);
}

void FromJson(const cJSON *jsonObject, VideoEncoderIn &videoEncoderIn)
{
    if (jsonObject == nullptr) {
        return;
    }
    if (!IsString(jsonObject, MIME)) {
        AVTRANS_LOGE("VideoEncoderIn MIME is invalid");
        return;
    }
    cJSON *mimeObj = cJSON_GetObjectItemCaseSensitive(jsonObject, MIME.c_str());
    if (mimeObj == nullptr || !cJSON_IsString(mimeObj)) {
        return;
    }
    videoEncoderIn.mime = mimeObj->valuestring;

    cJSON *videoPixelFmtJson = cJSON_GetObjectItemCaseSensitive(jsonObject, VIDEO_PIXEL_FMT.c_str());
    if (videoPixelFmtJson == nullptr || !cJSON_IsArray(videoPixelFmtJson)) {
        AVTRANS_LOGE("VideoEncoderIn VIDEO_PIXEL_FMT is invalid");
        return;
    }
    cJSON *fmt = nullptr;
    cJSON_ArrayForEach(fmt, videoPixelFmtJson) {
        if (fmt->type == cJSON_Number) {
            videoEncoderIn.pixel_fmt.push_back(static_cast<VideoPixelFormat>(fmt->valueint));
        }
    }
}

void ToJson(cJSON *jsonObject, const VideoEncoderOut &videoEncoderOut)
{
    if (jsonObject == nullptr) {
        return;
    }
    cJSON_AddStringToObject(jsonObject, MIME.c_str(), videoEncoderOut.mime.c_str());
}

void FromJson(const cJSON *jsonObject, VideoEncoderOut &videoEncoderOut)
{
    if (jsonObject == nullptr) {
        return;
    }
    if (!IsString(jsonObject, MIME)) {
        AVTRANS_LOGE("VideoEncoderOut MIME is invalid");
        return;
    }
    cJSON *mimeObj = cJSON_GetObjectItemCaseSensitive(jsonObject, MIME.c_str());
    if (mimeObj == nullptr || !cJSON_IsString(mimeObj)) {
        return;
    }
    videoEncoderOut.mime = mimeObj->valuestring;
}

void ToJson(cJSON *jsonObject, const VideoEncoder &videoEncoder)
{
    if (jsonObject == nullptr) {
        return;
    }
    cJSON_AddStringToObject(jsonObject, NAME.c_str(), videoEncoder.name.c_str());
    cJSON *videoEncoderInsJson = cJSON_CreateArray();
    if (videoEncoderInsJson == nullptr) {
        return;
    }
    for (const auto &in : videoEncoder.ins) {
        cJSON *videoEncoderInJson = cJSON_CreateObject();
        if (videoEncoderInJson == nullptr) {
            cJSON_Delete(videoEncoderInsJson);
            return;
        }
        ToJson(videoEncoderInJson, in);
        cJSON_AddItemToArray(videoEncoderInsJson, videoEncoderInJson);
    }
    cJSON_AddItemToObject(jsonObject, INS.c_str(), videoEncoderInsJson);

    cJSON *videoEncoderOutsJson = cJSON_CreateArray();
    if (videoEncoderOutsJson == nullptr) {
        return;
    }
    for (const auto &out : videoEncoder.outs) {
        cJSON *videoEncoderOutJson = cJSON_CreateObject();
        if (videoEncoderOutJson == nullptr) {
            cJSON_Delete(videoEncoderOutsJson);
            return;
        }
        ToJson(videoEncoderOutJson, out);
        cJSON_AddItemToArray(videoEncoderOutsJson, videoEncoderOutJson);
    }
    cJSON_AddItemToObject(jsonObject, OUTS.c_str(), videoEncoderOutsJson);
}

void FromJson(const cJSON *jsonObject, VideoEncoder &videoEncoder)
{
    if (jsonObject == nullptr) {
        return;
    }
    if (!IsString(jsonObject, NAME)) {
        AVTRANS_LOGE("VideoEncoder NAME is invalid");
        return;
    }
    cJSON *nameObj = cJSON_GetObjectItemCaseSensitive(jsonObject, NAME.c_str());
    if (nameObj == nullptr || !cJSON_IsString(nameObj)) {
        return;
    }
    videoEncoder.name = nameObj->valuestring;

    cJSON *videoEncoderInsJson = cJSON_GetObjectItemCaseSensitive(jsonObject, INS.c_str());
    if (videoEncoderInsJson == nullptr || !cJSON_IsArray(videoEncoderInsJson)) {
        AVTRANS_LOGE("VideoEncoder INS is invalid");
        return;
    }
    cJSON *inJson = nullptr;
    cJSON_ArrayForEach(inJson, videoEncoderInsJson) {
        VideoEncoderIn in;
        FromJson(inJson, in);
        videoEncoder.ins.push_back(in);
    }

    cJSON *videoEncoderOutsJson = cJSON_GetObjectItemCaseSensitive(jsonObject, OUTS.c_str());
    if (videoEncoderOutsJson == nullptr || !cJSON_IsArray(videoEncoderOutsJson)) {
        AVTRANS_LOGE("VideoEncoder OUTS is invalid");
        return;
    }
    cJSON *outJson = nullptr;
    cJSON_ArrayForEach(outJson, videoEncoderOutsJson) {
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

void ToJson(cJSON *jsonObject, const VideoDecoderIn &videoDecoderIn)
{
    if (jsonObject == nullptr) {
        return;
    }
    cJSON_AddStringToObject(jsonObject, MIME.c_str(), videoDecoderIn.mime.c_str());
    cJSON *fmtJson = cJSON_CreateArray();
    if (fmtJson == nullptr) {
        return;
    }
    for (auto &fmt : videoDecoderIn.vd_bit_stream_fmt) {
        cJSON_AddItemToArray(fmtJson, cJSON_CreateNumber(static_cast<uint32_t>(fmt)));
    }
    cJSON_AddItemToObject(jsonObject, VIDEO_BIT_STREAM_FMT.c_str(), fmtJson);
}

void FromJson(const cJSON *jsonObject, VideoDecoderIn &videoDecoderIn)
{
    if (jsonObject == nullptr) {
        return;
    }
    if (!IsString(jsonObject, MIME)) {
        AVTRANS_LOGE("VideoDecoderIn MIME is invalid");
        return;
    }
    cJSON *mimeObj = cJSON_GetObjectItemCaseSensitive(jsonObject, MIME.c_str());
    if (mimeObj == nullptr || !cJSON_IsString(mimeObj)) {
        return;
    }
    videoDecoderIn.mime = mimeObj->valuestring;

    cJSON *videoBitStreamJson = cJSON_GetObjectItemCaseSensitive(jsonObject, VIDEO_BIT_STREAM_FMT.c_str());
    if (videoBitStreamJson == nullptr || !cJSON_IsArray(videoBitStreamJson)) {
        AVTRANS_LOGE("VideoDecoderIn VIDEO_BIT_STREAM_FMT is invalid");
        return;
    }
    cJSON *fmt = nullptr;
    cJSON_ArrayForEach(fmt, videoBitStreamJson) {
        if (fmt->type == cJSON_Number) {
            videoDecoderIn.vd_bit_stream_fmt.push_back(static_cast<VideoBitStreamFormat>(fmt->valueint));
        }
    }
}

void ToJson(cJSON *jsonObject, const VideoDecoderOut &videoDecoderOut)
{
    if (jsonObject == nullptr) {
        return;
    }
    cJSON_AddStringToObject(jsonObject, MIME.c_str(), videoDecoderOut.mime.c_str());
    cJSON *fmtJson = cJSON_CreateArray();
    if (fmtJson == nullptr) {
        return;
    }
    for (auto &fmt : videoDecoderOut.pixel_fmt) {
        cJSON_AddItemToArray(fmtJson, cJSON_CreateNumber(static_cast<uint32_t>(fmt)));
    }
    cJSON_AddItemToObject(jsonObject, VIDEO_PIXEL_FMT.c_str(), fmtJson);
}

void FromJson(const cJSON *jsonObject, VideoDecoderOut &videoDecoderOut)
{
    if (jsonObject == nullptr) {
        return;
    }
    if (!IsString(jsonObject, MIME)) {
        AVTRANS_LOGE("VideoDecoderOut MIME is invalid");
        return;
    }
    cJSON *mimeObj = cJSON_GetObjectItemCaseSensitive(jsonObject, MIME.c_str());
    if (mimeObj == nullptr || !cJSON_IsString(mimeObj)) {
        return;
    }
    videoDecoderOut.mime = mimeObj->valuestring;

    cJSON *videoPixelFmtJson = cJSON_GetObjectItemCaseSensitive(jsonObject, VIDEO_PIXEL_FMT.c_str());
    if (videoPixelFmtJson == nullptr || !cJSON_IsArray(videoPixelFmtJson)) {
        AVTRANS_LOGE("VideoDecoderOut VIDEO_PIXEL_FMT is invalid");
        return;
    }
    cJSON *fmtInfo = nullptr;
    cJSON_ArrayForEach(fmtInfo, videoPixelFmtJson) {
        if (fmtInfo->type == cJSON_Number) {
            videoDecoderOut.pixel_fmt.push_back(static_cast<VideoPixelFormat>(fmtInfo->valueint));
        }
    }
}

void ToJson(cJSON *jsonObject, const VideoDecoder &videoDecoder)
{
    if (jsonObject == nullptr) {
        return;
    }
    cJSON_AddStringToObject(jsonObject, NAME.c_str(), videoDecoder.name.c_str());
    cJSON *videoDecoderInsJson = cJSON_CreateArray();
    if (videoDecoderInsJson == nullptr) {
        return;
    }
    for (const auto &in : videoDecoder.ins) {
        cJSON *videoDecoderInJson = cJSON_CreateObject();
        if (videoDecoderInJson == nullptr) {
            cJSON_Delete(videoDecoderInsJson);
            return;
        }
        ToJson(videoDecoderInJson, in);
        cJSON_AddItemToArray(videoDecoderInsJson, videoDecoderInJson);
    }
    cJSON_AddItemToObject(jsonObject, INS.c_str(), videoDecoderInsJson);

    cJSON *videoDecoderOutsJson = cJSON_CreateArray();
    if (videoDecoderOutsJson == nullptr) {
        return;
    }
    for (const auto &out : videoDecoder.outs) {
        cJSON *videoDecoderOutJson = cJSON_CreateObject();
        if (videoDecoderOutJson == nullptr) {
            cJSON_Delete(videoDecoderOutsJson);
            return;
        }
        ToJson(videoDecoderOutJson, out);
        cJSON_AddItemToArray(videoDecoderOutsJson, videoDecoderOutJson);
    }
    cJSON_AddItemToObject(jsonObject, OUTS.c_str(), videoDecoderOutsJson);
}

void FromJson(const cJSON *jsonObject, VideoDecoder &videoDecoder)
{
    if (jsonObject == nullptr) {
        return;
    }
    if (!IsString(jsonObject, NAME)) {
        AVTRANS_LOGE("VideoDecoder NAME is invalid");
        return;
    }
    cJSON *nameObj = cJSON_GetObjectItemCaseSensitive(jsonObject, NAME.c_str());
    if (nameObj == nullptr || !cJSON_IsString(nameObj)) {
        return;
    }
    videoDecoder.name = nameObj->valuestring;

    cJSON *videoDecoderInsJson = cJSON_GetObjectItemCaseSensitive(jsonObject, INS.c_str());
    if (videoDecoderInsJson == nullptr || !cJSON_IsArray(videoDecoderInsJson)) {
        AVTRANS_LOGE("VideoDecoder INS is invalid");
        return;
    }

    cJSON *inJson = nullptr;
    cJSON_ArrayForEach(inJson, videoDecoderInsJson) {
        VideoDecoderIn in;
        FromJson(inJson, in);
        videoDecoder.ins.push_back(in);
    }

    cJSON *videoDecoderOutsJson = cJSON_GetObjectItemCaseSensitive(jsonObject, OUTS.c_str());
    if (videoDecoderOutsJson == nullptr || !cJSON_IsArray(videoDecoderOutsJson)) {
        AVTRANS_LOGE("VideoDecoder OUTS is invalid");
        return;
    }
    cJSON *outJson = nullptr;
    cJSON_ArrayForEach(outJson, videoDecoderOutsJson) {
        VideoDecoderOut out;
        FromJson(outJson, out);
        videoDecoder.outs.push_back(out);
    }
}

template<typename T>
void ToJson(const std::string &key, cJSON *jsonObject, std::vector<T> &objs)
{
    cJSON *jsonObjs = cJSON_CreateArray();
    if (jsonObjs == nullptr) {
        return;
    }
    for (auto &obj : objs) {
        cJSON *jsonObj = cJSON_CreateObject();
        if (jsonObj == nullptr) {
            cJSON_Delete(jsonObjs);
            return;
        }
        ToJson(jsonObj, obj);
        cJSON_AddItemToArray(jsonObjs, jsonObj);
    }
    cJSON_AddItemToObject(jsonObject, key.c_str(), jsonObjs);
}

template<typename T>
void FromJson(const std::string &key, const cJSON *jsonObject, std::vector<T> &objs)
{
    cJSON *objJson = cJSON_GetObjectItemCaseSensitive(jsonObject, key.c_str());
    if (objJson == nullptr) {
        AVTRANS_LOGE("JSONObject key invalid, key: %{public}s", key.c_str());
        return;
    }
    cJSON *json = nullptr;
    cJSON_ArrayForEach(json, objJson) {
        T obj;
        FromJson(json, obj);
        objs.push_back(obj);
    }
}

int32_t QueryAudioEncoderAbilityStr(char* res)
{
    std::vector<AudioEncoder> audioEncoders = QueryAudioEncoderAbility();
    cJSON *jsonObject = cJSON_CreateObject();
    if (jsonObject == nullptr) {
        return 0;
    }
    ToJson<AudioEncoder>(AUDIO_ENCODERS, jsonObject, audioEncoders);
    char *jsonStr = cJSON_Print(jsonObject);
    if (jsonStr == nullptr) {
        cJSON_Delete(jsonObject);
        return 0;
    }
    std::string ret = std::string(jsonStr);
    cJSON_free(jsonStr);
    cJSON_Delete(jsonObject);
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
    cJSON *jsonObject = cJSON_CreateObject();
    if (jsonObject == nullptr) {
        return 0;
    }
    ToJson<AudioDecoder>(AUDIO_DECODERS, jsonObject, audioDecoders);
    char *jsonStr = cJSON_Print(jsonObject);
    if (jsonStr == nullptr) {
        cJSON_Delete(jsonObject);
        return 0;
    }
    std::string ret = std::string(jsonStr);
    cJSON_free(jsonStr);
    cJSON_Delete(jsonObject);
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
    cJSON *jsonObject = cJSON_CreateObject();
    if (jsonObject == nullptr) {
        return 0;
    }
    ToJson<VideoEncoder>(VIDEO_ENCODERS, jsonObject, viudeoEncoders);
    char *jsonStr = cJSON_Print(jsonObject);
    if (jsonStr == nullptr) {
        cJSON_Delete(jsonObject);
        return 0;
    }
    std::string ret = std::string(jsonStr);
    cJSON_free(jsonStr);
    cJSON_Delete(jsonObject);
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
    cJSON *jsonObject = cJSON_CreateObject();
    if (jsonObject == nullptr) {
        return 0;
    }
    ToJson<VideoDecoder>(VIDEO_DECODERS, jsonObject, videoDecoders);
    char *jsonStr = cJSON_Print(jsonObject);
    if (jsonStr == nullptr) {
        cJSON_Delete(jsonObject);
        return 0;
    }
    std::string ret = std::string(jsonStr);
    cJSON_free(jsonStr);
    cJSON_Delete(jsonObject);
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