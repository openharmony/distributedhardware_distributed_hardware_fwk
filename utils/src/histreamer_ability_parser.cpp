/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

void FromJson(const cJSON *jsonObject, AudioEncoderIn &audioEncoderIn)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    if (!IsString(jsonObject, MIME)) {
        DHLOGE("AudioEncoderIn MIME is invalid!\n");
        return;
    }
    audioEncoderIn.mime = cJSON_GetObjectItem(jsonObject, MIME.c_str())->valuestring;
    if (!IsArray(jsonObject, SAMPLE_RATE)) {
        DHLOGE("AudioEncoderIn SAMPLE_RATE is invalid\n");
        return;
    }
    cJSON *sampleRate = cJSON_GetObjectItem(jsonObject, SAMPLE_RATE.c_str());
    if (sampleRate == NULL) {
        DHLOGE("AudioEncoderIn SAMPLE_RATE is invalid\n");
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
    if (!IsString(jsonObject, MIME)) {
        DHLOGE("AudioEncoderOut MIME is invalid!");
        return;
    }
    audioEncoderOut.mime = cJSON_GetObjectItem(jsonObject, MIME.c_str())->valuestring;
    if (!IsUInt32(jsonObject, AD_MPEG_VER)) {
        DHLOGE("AudioEncoderOut AD_MPEG_VER is invalid!");
        return;
    }
    audioEncoderOut.ad_mpeg_ver = (uint32_t)cJSON_GetObjectItem(jsonObject, AD_MPEG_VER.c_str())->valuedouble;

    if (!IsUInt8(jsonObject, AUDIO_AAC_PROFILE)) {
        DHLOGE("AudioEncoderOut AUDIO_AAC_PROFILE is invalid!");
        return;
    }
    audioEncoderOut.aac_profile =
        (AudioAacProfile)cJSON_GetObjectItem(jsonObject, AUDIO_AAC_PROFILE.c_str())->valuedouble;

    if (!IsUInt8(jsonObject, AUDIO_AAC_STREAM_FORMAT)) {
        DHLOGE("AudioEncoderOut AUDIO_AAC_STREAM_FORMAT is invalid!");
        return;
    }
    audioEncoderOut.aac_stm_fmt =
        (AudioAacStreamFormat)cJSON_GetObjectItem(jsonObject, AUDIO_AAC_STREAM_FORMAT.c_str())->valuedouble;
}

void FromJson(const cJSON *jsonObject, AudioEncoder &audioEncoder)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    if (!IsString(jsonObject, NAME)) {
        DHLOGE("AudioEncoder NAME is invalid!");
        return;
    }
    audioEncoder.name = cJSON_GetObjectItem(jsonObject, NAME.c_str())->valuestring;

    if (!IsArray(jsonObject, INS)) {
        DHLOGE("AudioEncoder INS is invalid!");
        return;
    }
    cJSON *insJson = cJSON_GetObjectItem(jsonObject, INS.c_str());
    cJSON *inJson = nullptr;
    cJSON_ArrayForEach(inJson, insJson) {
        AudioEncoderIn in;
        FromJson(inJson, in);
        audioEncoder.ins.push_back(in);
    }

    if (!IsArray(jsonObject, OUTS)) {
        DHLOGE("AudioEncoder OUTS is invalid!");
        return;
    }
    cJSON *outsJson = cJSON_GetObjectItem(jsonObject, OUTS.c_str());
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
    if (!IsString(jsonObject, MIME)) {
        DHLOGE("AudioDecoderIn MIME is invalid!");
        return;
    }
    audioDecoderIn.mime = cJSON_GetObjectItem(jsonObject, MIME.c_str())->valuestring;

    if (!IsArray(jsonObject, AUDIO_CHANNEL_LAYOUT)) {
        DHLOGE("AudioDecoder AUDIO_CHANNEL_LAYOUT is invalid!");
        return;
    }
    const cJSON *channelLayoutJson = cJSON_GetObjectItem(jsonObject, AUDIO_CHANNEL_LAYOUT.c_str());
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
    if (!IsString(jsonObject, MIME)) {
        DHLOGE("AudioDecoderOut MIME is invalid!");
        return;
    }
    audioDecoderOut.mime = cJSON_GetObjectItem(jsonObject, MIME.c_str())->valuestring;
    if (!IsArray(jsonObject, AUDIO_SAMPLE_FORMAT)) {
        DHLOGE("AudioDecoderOut AUDIO_SAMPLE_FORMAT is invalid!");
        return;
    }
    cJSON *sampleFormatJson = cJSON_GetObjectItem(jsonObject, AUDIO_SAMPLE_FORMAT.c_str());
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
    if (!IsString(jsonObject, NAME)) {
        DHLOGE("AudioDecoderOut MIME is invalid!");
        return;
    }
    audioDecoder.name = cJSON_GetObjectItem(jsonObject, NAME.c_str())->valuestring;

    if (!IsArray(jsonObject, INS)) {
        DHLOGE("AudioDecoder OUTS is invalid!");
        return;
    }
    const cJSON *insJson = cJSON_GetObjectItem(jsonObject, INS.c_str());
    cJSON *inJson = nullptr;
    cJSON_ArrayForEach(inJson, insJson) {
        AudioDecoderIn in;
        FromJson(inJson, in);
        audioDecoder.ins.push_back(in);
    }
    if (!IsArray(jsonObject, OUTS)) {
        DHLOGE("AudioDecoder OUTS is invalid!");
        return;
    }
    cJSON *outsJson = cJSON_GetObjectItem(jsonObject, OUTS.c_str());
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
    if (!IsString(jsonObject, MIME)) {
        DHLOGE("VideoEncoderIn MIME is invalid!");
        return;
    }
    videoEncoderIn.mime = cJSON_GetObjectItem(jsonObject, MIME.c_str())->valuestring;

    if (!IsArray(jsonObject, VIDEO_PIXEL_FMT)) {
        DHLOGE("VideoEncoderIn VIDEO_PIXEL_FMT is invalid!");
        return;
    }
    cJSON *videoPixelFmt = cJSON_GetObjectItem(jsonObject, VIDEO_PIXEL_FMT.c_str());
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
    if (!IsString(jsonObject, MIME)) {
        DHLOGE("VideoEncoderIn MIME is invalid!");
        return;
    }
    videoEncoderOut.mime = cJSON_GetObjectItem(jsonObject, MIME.c_str())->valuestring;
}

void FromJson(const cJSON *jsonObject, VideoEncoder &videoEncoder)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    if (!IsString(jsonObject, NAME)) {
        DHLOGE("VideoEncoder NAME is invalid!");
        return;
    }
    videoEncoder.name = cJSON_GetObjectItem(jsonObject, NAME.c_str())->valuestring;

    if (!IsArray(jsonObject, INS)) {
        DHLOGE("VideoEncoder INS is invalid!");
        return;
    }
    cJSON *videoEncoderInsJson = cJSON_GetObjectItem(jsonObject, INS.c_str());
    cJSON *inJson = nullptr;
    cJSON_ArrayForEach(inJson, videoEncoderInsJson) {
        VideoEncoderIn in;
        FromJson(inJson, in);
        videoEncoder.ins.push_back(in);
    }

    if (!IsArray(jsonObject, OUTS)) {
        DHLOGE("VideoEncoder OUTS is invalid!");
        return;
    }
    cJSON *videoEncoderOutsJson = cJSON_GetObjectItem(jsonObject, OUTS.c_str());
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
    if (!IsString(jsonObject, MIME)) {
        DHLOGE("VideoDecoderIn MIME is invalid!");
        return;
    }
    videoDecoderIn.mime = cJSON_GetObjectItem(jsonObject, MIME.c_str())->valuestring;

    if (!IsArray(jsonObject, VIDEO_BIT_STREAM_FMT)) {
        DHLOGE("VideoDecoderIn VIDEO_BIT_STREAM_FMT is invalid!");
        return;
    }
    cJSON *videoBitStreamFmtJson = cJSON_GetObjectItem(jsonObject, VIDEO_BIT_STREAM_FMT.c_str());
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
    if (!IsString(jsonObject, MIME)) {
        DHLOGE("VideoDecoderOut MIME is invalid!");
        return;
    }
    videoDecoderOut.mime = cJSON_GetObjectItem(jsonObject, MIME.c_str())->valuestring;

    if (!IsArray(jsonObject, VIDEO_BIT_STREAM_FMT)) {
        DHLOGE("videoDecoderOut VIDEO_PIXEL_FMT is invalid!");
        return;
    }
    cJSON *videoPixelFmtJson = cJSON_GetObjectItem(jsonObject, VIDEO_PIXEL_FMT.c_str());
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
    if (!IsString(jsonObject, NAME)) {
        DHLOGE("VideoDecoder NAME is invalid!");
        return;
    }
    videoDecoder.name = cJSON_GetObjectItem(jsonObject, NAME.c_str())->valuestring;

    if (!IsArray(jsonObject, INS)) {
        DHLOGE("VideoDecoder INS is invalid!");
        return;
    }
    cJSON *videoDecoderInsJson = cJSON_GetObjectItem(jsonObject, INS.c_str());
    cJSON *inJson = nullptr;
    cJSON_ArrayForEach(inJson, videoDecoderInsJson) {
        VideoDecoderIn in;
        FromJson(inJson, in);
        videoDecoder.ins.push_back(in);
    }

    if (!IsArray(jsonObject, OUTS)) {
        DHLOGE("VideoDecoder OUTS is invalid!");
        return;
    }
    cJSON *videoDecoderOutsJson =  cJSON_GetObjectItem(jsonObject, OUTS.c_str());
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