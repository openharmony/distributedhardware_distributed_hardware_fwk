/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "histreamer_ability_parser.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_log.h"


namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr const char *NAME = "name";
    constexpr const char *INS = "ins";
    constexpr const char *OUTS = "outs";
    constexpr const char *MIME = "mime";
    constexpr const char *SAMPLE_RATE = "sample_rate";
    constexpr const char *AUDIO_SAMPLE_FORMAT = "sample_fmt";
    constexpr const char *AD_MPEG_VER = "ad_mpeg_ver";
    constexpr const char *AUDIO_AAC_PROFILE = "aac_profile";
    constexpr const char *AUDIO_AAC_STREAM_FORMAT = "aac_stream_fmt";
    constexpr const char *AUDIO_CHANNEL_LAYOUT = "channel_layout";
    constexpr const char *VIDEO_PIXEL_FMT = "pixel_fmt";
    constexpr const char *VIDEO_BIT_STREAM_FMT = "vd_bit_stream_fmt";
}

void FromJson(const cJSON *jsonObject, AudioEncoderIn &audioEncoderIn)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *mimeJsonObj = cJSON_GetObjectItem(jsonObject, MIME);
    if (!IsString(mimeJsonObj)) {
        DHLOGE("AudioEncoderIn MIME is invalid!");
        return;
    }
    audioEncoderIn.mime = mimeJsonObj->valuestring;

    cJSON *sampleRate = cJSON_GetObjectItem(jsonObject, SAMPLE_RATE);
    if (!IsArray(sampleRate)) {
        DHLOGE("AudioEncoderIn SAMPLE_RATE is invalid!");
        return;
    }
    cJSON *sampleRateItem = nullptr;
    cJSON_ArrayForEach(sampleRateItem, sampleRate) {
        if (sampleRateItem && sampleRateItem->type == cJSON_Number) {
            audioEncoderIn.sample_rate.push_back((uint32_t)sampleRateItem->valuedouble);
        }
    }
}

void FromJson(const cJSON *jsonObject, AudioEncoderOut &audioEncoderOut)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *mimeJsonObj = cJSON_GetObjectItem(jsonObject, MIME);
    if (!IsString(mimeJsonObj)) {
        DHLOGE("AudioEncoderOut MIME is invalid!");
        return;
    }
    audioEncoderOut.mime = mimeJsonObj->valuestring;

    cJSON *mpegVerJsonObj = cJSON_GetObjectItem(jsonObject, AD_MPEG_VER);
    if (!IsUInt32(mpegVerJsonObj)) {
        DHLOGE("AudioEncoderOut AD_MPEG_VER is invalid!");
        return;
    }
    audioEncoderOut.ad_mpeg_ver = static_cast<uint32_t>(mpegVerJsonObj->valuedouble);

    cJSON *aacProfileJsonObj = cJSON_GetObjectItem(jsonObject, AUDIO_AAC_PROFILE);
    if (!IsUInt8(aacProfileJsonObj)) {
        DHLOGE("AudioEncoderOut AUDIO_AAC_PROFILE is invalid!");
        return;
    }
    audioEncoderOut.aac_profile = (AudioAacProfile)aacProfileJsonObj->valuedouble;

    cJSON *aacStreamFmtJsonObj = cJSON_GetObjectItem(jsonObject, AUDIO_AAC_STREAM_FORMAT);
    if (!IsUInt8(aacStreamFmtJsonObj)) {
        DHLOGE("AudioEncoderOut AUDIO_AAC_STREAM_FORMAT is invalid!");
        return;
    }
    audioEncoderOut.aac_stm_fmt = (AudioAacStreamFormat)aacStreamFmtJsonObj->valuedouble;
}

void FromJson(const cJSON *jsonObject, AudioEncoder &audioEncoder)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *nameJsonObj = cJSON_GetObjectItem(jsonObject, NAME);
    if (!IsString(nameJsonObj)) {
        DHLOGE("AudioEncoder NAME is invalid!");
        return;
    }
    audioEncoder.name = nameJsonObj->valuestring;

    cJSON *insJson = cJSON_GetObjectItem(jsonObject, INS);
    if (!IsArray(insJson)) {
        DHLOGE("AudioEncoder INS is invalid!");
        return;
    }
    cJSON *inJson = nullptr;
    cJSON_ArrayForEach(inJson, insJson) {
        AudioEncoderIn in;
        FromJson(inJson, in);
        audioEncoder.ins.push_back(in);
    }

    cJSON *outsJson = cJSON_GetObjectItem(jsonObject, OUTS);
    if (!IsArray(outsJson)) {
        DHLOGE("AudioEncoder OUTS is invalid!");
        return;
    }
    cJSON *outJson = nullptr;
    cJSON_ArrayForEach(outJson, outsJson) {
        AudioEncoderOut out;
        FromJson(outJson, out);
        audioEncoder.outs.push_back(out);
    }
}

void FromJson(const cJSON *jsonObject, AudioDecoderIn &audioDecoderIn)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *mimeJsonObj = cJSON_GetObjectItem(jsonObject, MIME);
    if (!IsString(mimeJsonObj)) {
        DHLOGE("AudioDecoderIn MIME is invalid!");
        return;
    }
    audioDecoderIn.mime = mimeJsonObj->valuestring;

    cJSON *channelLayoutJson = cJSON_GetObjectItem(jsonObject, AUDIO_CHANNEL_LAYOUT);
    if (!IsArray(channelLayoutJson)) {
        DHLOGE("AudioDecoder AUDIO_CHANNEL_LAYOUT is invalid!");
        return;
    }
    const cJSON *layout = nullptr;
    cJSON_ArrayForEach(layout, channelLayoutJson) {
        if (layout && layout->type == cJSON_Number) {
            audioDecoderIn.channel_layout.push_back((AudioChannelLayout)layout->valuedouble);
        }
    }
}

void FromJson(const cJSON *jsonObject, AudioDecoderOut &audioDecoderOut)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *mimeJsonObj = cJSON_GetObjectItem(jsonObject, MIME);
    if (!IsString(mimeJsonObj)) {
        DHLOGE("AudioDecoderOut MIME is invalid!");
        return;
    }
    audioDecoderOut.mime = mimeJsonObj->valuestring;

    cJSON *sampleFormatJson = cJSON_GetObjectItem(jsonObject, AUDIO_SAMPLE_FORMAT);
    if (!IsArray(sampleFormatJson)) {
        DHLOGE("AudioDecoderOut AUDIO_SAMPLE_FORMAT is invalid!");
        return;
    }
    cJSON *format = nullptr;
    cJSON_ArrayForEach(format, sampleFormatJson) {
        if (format && format->type == cJSON_Number) {
            audioDecoderOut.sample_fmt.push_back((AudioSampleFormat)format->valuedouble);
        }
    }
}

void FromJson(const cJSON *jsonObject, AudioDecoder &audioDecoder)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *nameJsonObj = cJSON_GetObjectItem(jsonObject, NAME);
    if (!IsString(nameJsonObj)) {
        DHLOGE("AudioDecoderOut MIME is invalid!");
        return;
    }
    audioDecoder.name = nameJsonObj->valuestring;

    cJSON *insJson = cJSON_GetObjectItem(jsonObject, INS);
    if (!IsArray(insJson)) {
        DHLOGE("AudioDecoder INS is invalid!");
        return;
    }
    cJSON *inJson = nullptr;
    cJSON_ArrayForEach(inJson, insJson) {
        AudioDecoderIn in;
        FromJson(inJson, in);
        audioDecoder.ins.push_back(in);
    }

    cJSON *outsJson = cJSON_GetObjectItem(jsonObject, OUTS);
    if (!IsArray(outsJson)) {
        DHLOGE("AudioDecoder OUTS is invalid!");
        return;
    }
    cJSON *outJson = nullptr;
    cJSON_ArrayForEach(outJson, outsJson) {
        AudioDecoderOut out;
        FromJson(outJson, out);
        audioDecoder.outs.push_back(out);
    }
}

void FromJson(const cJSON *jsonObject, VideoEncoderIn &videoEncoderIn)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *mimeJsonObj = cJSON_GetObjectItem(jsonObject, MIME);
    if (!IsString(mimeJsonObj)) {
        DHLOGE("VideoEncoderIn MIME is invalid!");
        return;
    }
    videoEncoderIn.mime = mimeJsonObj->valuestring;

    cJSON *videoPixelFmt = cJSON_GetObjectItem(jsonObject, VIDEO_PIXEL_FMT);
    if (!IsArray(videoPixelFmt)) {
        DHLOGE("VideoEncoderIn VIDEO_PIXEL_FMT is invalid!");
        return;
    }
    cJSON *pixelFmt = nullptr;
    cJSON_ArrayForEach(pixelFmt, videoPixelFmt) {
        if (pixelFmt && pixelFmt->type == cJSON_Number) {
            videoEncoderIn.pixel_fmt.push_back((VideoPixelFormat)pixelFmt->valuedouble);
        }
    }
}

void FromJson(const cJSON *jsonObject, VideoEncoderOut &videoEncoderOut)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *mimeJsonObj = cJSON_GetObjectItem(jsonObject, MIME);
    if (!IsString(mimeJsonObj)) {
        DHLOGE("VideoEncoderOut MIME is invalid!");
        return;
    }
    videoEncoderOut.mime = mimeJsonObj->valuestring;
}

void FromJson(const cJSON *jsonObject, VideoEncoder &videoEncoder)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *nameJsonObj = cJSON_GetObjectItem(jsonObject, NAME);
    if (!IsString(nameJsonObj)) {
        DHLOGE("VideoEncoder NAME is invalid!");
        return;
    }
    videoEncoder.name = nameJsonObj->valuestring;

    cJSON *videoEncoderInsJson = cJSON_GetObjectItem(jsonObject, INS);
    if (!IsArray(videoEncoderInsJson)) {
        DHLOGE("VideoEncoder INS is invalid!");
        return;
    }
    cJSON *inJson = nullptr;
    cJSON_ArrayForEach(inJson, videoEncoderInsJson) {
        VideoEncoderIn in;
        FromJson(inJson, in);
        videoEncoder.ins.push_back(in);
    }

    cJSON *videoEncoderOutsJson = cJSON_GetObjectItem(jsonObject, OUTS);
    if (!IsArray(videoEncoderOutsJson)) {
        DHLOGE("VideoEncoder OUTS is invalid!");
        return;
    }
    cJSON *outJson = nullptr;
    cJSON_ArrayForEach(outJson, videoEncoderOutsJson) {
        VideoEncoderOut out;
        FromJson(outJson, out);
        videoEncoder.outs.push_back(out);
    }
}

void FromJson(const cJSON *jsonObject, VideoDecoderIn &videoDecoderIn)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *mimeJsonObj = cJSON_GetObjectItem(jsonObject, MIME);
    if (!IsString(mimeJsonObj)) {
        DHLOGE("VideoDecoderIn MIME is invalid!");
        return;
    }
    videoDecoderIn.mime = mimeJsonObj->valuestring;

    cJSON *videoBitStreamFmtJson = cJSON_GetObjectItem(jsonObject, VIDEO_BIT_STREAM_FMT);
    if (!IsArray(videoBitStreamFmtJson)) {
        DHLOGE("VideoDecoderIn VIDEO_BIT_STREAM_FMT is invalid!");
        return;
    }
    cJSON *fmt = nullptr;
    cJSON_ArrayForEach(fmt, videoBitStreamFmtJson) {
        if (fmt && fmt->type == cJSON_Number) {
            videoDecoderIn.vd_bit_stream_fmt.push_back((VideoBitStreamFormat)(fmt->valuedouble));
        }
    }
}

void FromJson(const cJSON *jsonObject, VideoDecoderOut &videoDecoderOut)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *mimeJsonObj = cJSON_GetObjectItem(jsonObject, MIME);
    if (!IsString(mimeJsonObj)) {
        DHLOGE("VideoDecoderOut MIME is invalid!");
        return;
    }
    videoDecoderOut.mime = mimeJsonObj->valuestring;

    cJSON *videoPixelFmtJson = cJSON_GetObjectItem(jsonObject, VIDEO_PIXEL_FMT);
    if (!IsArray(videoPixelFmtJson)) {
        DHLOGE("videoDecoderOut VIDEO_PIXEL_FMT is invalid!");
        return;
    }
    cJSON *fmt = nullptr;
    cJSON_ArrayForEach(fmt, videoPixelFmtJson) {
        if (fmt && fmt->type == cJSON_Number) {
            videoDecoderOut.pixel_fmt.push_back((VideoPixelFormat)(fmt->valuedouble));
        }
    }
}

void FromJson(const cJSON *jsonObject, VideoDecoder &videoDecoder)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *nameJsonObj = cJSON_GetObjectItem(jsonObject, NAME);
    if (!IsString(nameJsonObj)) {
        DHLOGE("VideoDecoder NAME is invalid!");
        return;
    }
    videoDecoder.name = nameJsonObj->valuestring;

    cJSON *videoDecoderInsJson = cJSON_GetObjectItem(jsonObject, INS);
    if (!IsArray(videoDecoderInsJson)) {
        DHLOGE("VideoDecoder INS is invalid!");
        return;
    }
    cJSON *inJson = nullptr;
    cJSON_ArrayForEach(inJson, videoDecoderInsJson) {
        VideoDecoderIn in;
        FromJson(inJson, in);
        videoDecoder.ins.push_back(in);
    }

    cJSON *videoDecoderOutsJson =  cJSON_GetObjectItem(jsonObject, OUTS);
    if (!IsArray(videoDecoderOutsJson)) {
        DHLOGE("VideoDecoder OUTS is invalid!");
        return;
    }
    cJSON *outJson = nullptr;
    cJSON_ArrayForEach(outJson, videoDecoderOutsJson) {
        VideoDecoderOut out;
        FromJson(outJson, out);
        videoDecoder.outs.push_back(out);
    }
}

template <typename T>
void FromJson(const std::string &key, const cJSON *jsonObject, std::vector<T> &objs)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *json = cJSON_GetObjectItem(jsonObject, key.c_str());
    if (json == NULL) {
        DHLOGE("JSONObject key invalid, key: %{public}s", key.c_str());
        return;
    }
    if (cJSON_IsArray(json)) {
        cJSON *item;
        cJSON_ArrayForEach(item, json) {
            T obj;
            FromJson(item, obj);
            objs.push_back(obj);
        }
    } else {
        T obj;
        FromJson(json, obj);
        objs.push_back(obj);
    }
}

template
void FromJson<AudioEncoder>(const std::string &key, const cJSON *jsonObject, std::vector<AudioEncoder> &objs);
template
void FromJson<AudioDecoder>(const std::string &key, const cJSON *jsonObject, std::vector<AudioDecoder> &objs);
template
void FromJson<VideoEncoder>(const std::string &key, const cJSON *jsonObject, std::vector<VideoEncoder> &objs);
template
void FromJson<VideoDecoder>(const std::string &key, const cJSON *jsonObject, std::vector<VideoDecoder> &objs);

}
}