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

void FromJson(const nlohmann::json &jsonObject, AudioEncoderIn &audioEncoderIn)
{
    if (!IsString(jsonObject, MIME)) {
        DHLOGE("AudioEncoderIn MIME is invalid!");
        return;
    }
    audioEncoderIn.mime = jsonObject.at(MIME).get<std::string>();
    if (jsonObject.find(SAMPLE_RATE) == jsonObject.end()) {
        DHLOGE("AudioEncoderIn SAMPLE_RATE is invalid");
    }
    audioEncoderIn.sample_rate = jsonObject.at(SAMPLE_RATE).get<std::vector<uint32_t>>();
}

void FromJson(const nlohmann::json &jsonObject, AudioEncoderOut &audioEncoderOut)
{
    if (!IsString(jsonObject, MIME)) {
        DHLOGE("AudioEncoderOut MIME is invalid!");
        return;
    }
    audioEncoderOut.mime = jsonObject.at(MIME).get<std::string>();

    if (!IsUInt32(jsonObject, AD_MPEG_VER)) {
        DHLOGE("AudioEncoderOut AD_MPEG_VER is invalid");
        return;
    }
    audioEncoderOut.ad_mpeg_ver = jsonObject.at(AD_MPEG_VER).get<uint32_t>();

    if (!IsUInt8(jsonObject, AUDIO_AAC_PROFILE)) {
        DHLOGE("AudioEncoderOut AUDIO_AAC_PROFILE is invalid");
        return;
    }
    audioEncoderOut.aac_profile = (AudioAacProfile)jsonObject.at(AUDIO_AAC_PROFILE).get<uint8_t>();

    if (!IsUInt8(jsonObject, AUDIO_AAC_STREAM_FORMAT)) {
        DHLOGE("AudioEncoderOut AUDIO_AAC_STREAM_FORMAT is invalid");
        return;
    }
    audioEncoderOut.aac_stm_fmt = (AudioAacStreamFormat)jsonObject.at(AUDIO_AAC_STREAM_FORMAT).get<uint8_t>();
}

void FromJson(const nlohmann::json &jsonObject, AudioEncoder &audioEncoder)
{
    if (!IsString(jsonObject, NAME)) {
        DHLOGE("AudioEncoder NAME is invalid");
        return;
    }
    audioEncoder.name = jsonObject.at(NAME).get<std::string>();

    if (jsonObject.find(INS) == jsonObject.end()) {
        DHLOGE("AudioEncoder INS is invalid");
        return;
    }

    nlohmann::json audioEncoderInsJson = jsonObject[INS];
    for (auto &inJson : audioEncoderInsJson) {
        AudioEncoderIn in;
        FromJson(inJson, in);
        audioEncoder.ins.push_back(in);
    }

    if (jsonObject.find(OUTS) == jsonObject.end()) {
        DHLOGE("AudioEncoder OUTS is invalid");
        return;
    }
    nlohmann::json audioEncoderOutsJson = jsonObject[OUTS];
    for (auto &outJson : audioEncoderOutsJson) {
        AudioEncoderOut out;
        FromJson(outJson, out);
        audioEncoder.outs.push_back(out);
    }
}

void FromJson(const nlohmann::json &jsonObject, AudioDecoderIn &audioDecoderIn)
{
    if (!IsString(jsonObject, MIME)) {
        DHLOGE("AudioDecoderIn MIME is invalid");
        return;
    }
    audioDecoderIn.mime = jsonObject.at(MIME).get<std::string>();

    if (jsonObject.find(AUDIO_CHANNEL_LAYOUT) == jsonObject.end()) {
        DHLOGE("AudioEncoder AUDIO_CHANNEL_LAYOUT is invalid");
        return;
    }
    nlohmann::json channelLayoutJson = jsonObject[AUDIO_CHANNEL_LAYOUT];
    for (auto layout : channelLayoutJson) {
        audioDecoderIn.channel_layout.push_back((AudioChannelLayout)layout);
    }
}

void FromJson(const nlohmann::json &jsonObject, AudioDecoderOut &audioDecoderOut)
{
    if (!IsString(jsonObject, MIME)) {
        DHLOGE("AudioDecoderOut MIME is invalid");
        return;
    }
    audioDecoderOut.mime = jsonObject.at(MIME).get<std::string>();

    if (jsonObject.find(AUDIO_SAMPLE_FORMAT) == jsonObject.end()) {
        DHLOGE("AudioDecoderOut AUDIO_SAMPLE_FORMAT is invalid");
        return;
    }

    for (auto sampleFormatJson : jsonObject[AUDIO_SAMPLE_FORMAT]) {
        audioDecoderOut.sample_fmt.push_back((AudioSampleFormat)sampleFormatJson);
    }
}

void FromJson(const nlohmann::json &jsonObject, AudioDecoder &audioDecoder)
{
    if (!IsString(jsonObject, NAME)) {
        DHLOGE("AudioDecoder NAME is invalid");
        return;
    }
    audioDecoder.name = jsonObject.at(NAME).get<std::string>();

    if (jsonObject.find(INS) == jsonObject.end()) {
        DHLOGE("AudioDecoder INS is invalid");
        return;
    }

    nlohmann::json audioDecoderInsJson = jsonObject[INS];
    for (auto &inJson : audioDecoderInsJson) {
        AudioDecoderIn in;
        FromJson(inJson, in);
        audioDecoder.ins.push_back(in);
    }

    if (jsonObject.find(OUTS) == jsonObject.end()) {
        DHLOGE("AudioDecoder OUTS is invalid");
        return;
    }
    nlohmann::json audioDecoderOutsJson = jsonObject[OUTS];
    for (auto &outJson : audioDecoderOutsJson) {
        AudioDecoderOut out;
        FromJson(outJson, out);
        audioDecoder.outs.push_back(out);
    }
}

void FromJson(const nlohmann::json &jsonObject, VideoEncoderIn &videoEncoderIn)
{
    if (!IsString(jsonObject, MIME)) {
        DHLOGE("VideoEncoderIn MIME is invalid");
        return;
    }
    videoEncoderIn.mime = jsonObject.at(MIME).get<std::string>();

    if (jsonObject.find(VIDEO_PIXEL_FMT) == jsonObject.end()) {
        DHLOGE("VideoEncoderIn VIDEO_PIXEL_FMT is invalid");
        return;
    }
    for (auto fmt : jsonObject[VIDEO_PIXEL_FMT]) {
        videoEncoderIn.pixel_fmt.push_back((VideoPixelFormat)fmt);
    }
}

void FromJson(const nlohmann::json &jsonObject, VideoEncoderOut &videoEncoderOut)
{
    if (!IsString(jsonObject, MIME)) {
        DHLOGE("VideoEncoderOut MIME is invalid");
        return;
    }
    videoEncoderOut.mime = jsonObject[MIME].get<std::string>();
}

void FromJson(const nlohmann::json &jsonObject, VideoEncoder &videoEncoder)
{
    if (!IsString(jsonObject, NAME)) {
        DHLOGE("VideoEncoder NAME is invalid");
        return;
    }
    videoEncoder.name = jsonObject.at(NAME).get<std::string>();

    if (jsonObject.find(INS) == jsonObject.end()) {
        DHLOGE("VideoEncoder INS is invalid");
        return;
    }

    nlohmann::json videoEncoderInsJson = jsonObject[INS];
    for (auto &inJson : videoEncoderInsJson) {
        VideoEncoderIn in;
        FromJson(inJson, in);
        videoEncoder.ins.push_back(in);
    }

    if (jsonObject.find(OUTS) == jsonObject.end()) {
        DHLOGE("VideoEncoder OUTS is invalid");
        return;
    }
    nlohmann::json videoEncoderOutsJson = jsonObject[OUTS];
    for (auto &outJson : videoEncoderOutsJson) {
        VideoEncoderOut out;
        FromJson(outJson, out);
        videoEncoder.outs.push_back(out);
    }
}


void FromJson(const nlohmann::json &jsonObject, VideoDecoderIn &videoDecoderIn)
{
    if (!IsString(jsonObject, MIME)) {
        DHLOGE("VideoDecoderIn MIME is invalid");
        return;
    }
    videoDecoderIn.mime = jsonObject.at(MIME).get<std::string>();

    if (jsonObject.find(VIDEO_BIT_STREAM_FMT) == jsonObject.end()) {
        DHLOGE("VideoDecoderIn VIDEO_BIT_STREAM_FMT is invalid");
        return;
    }
    for (auto fmt : jsonObject[VIDEO_BIT_STREAM_FMT]) {
        videoDecoderIn.vd_bit_stream_fmt.push_back((VideoBitStreamFormat)fmt);
    }
}

void FromJson(const nlohmann::json &jsonObject, VideoDecoderOut &videoDecoderOut)
{
    if (!IsString(jsonObject, MIME)) {
        DHLOGE("VideoDecoderOut MIME is invalid");
        return;
    }
    videoDecoderOut.mime = jsonObject.at(MIME).get<std::string>();

    if (jsonObject.find(VIDEO_PIXEL_FMT) == jsonObject.end()) {
        DHLOGE("VideoDecoderOut VIDEO_PIXEL_FMT is invalid");
        return;
    }
    for (auto fmt : jsonObject[VIDEO_PIXEL_FMT]) {
        videoDecoderOut.pixel_fmt.push_back((VideoPixelFormat)fmt);
    }
}

void FromJson(const nlohmann::json &jsonObject, VideoDecoder &videoDecoder)
{
    if (!IsString(jsonObject, NAME)) {
        DHLOGE("VideoDecoder NAME is invalid");
        return;
    }
    videoDecoder.name = jsonObject.at(NAME).get<std::string>();

    if (jsonObject.find(INS) == jsonObject.end()) {
        DHLOGE("VideoDecoder INS is invalid");
        return;
    }

    nlohmann::json videoDecoderInsJson = jsonObject[INS];
    for (auto &inJson : videoDecoderInsJson) {
        VideoDecoderIn in;
        FromJson(inJson, in);
        videoDecoder.ins.push_back(in);
    }

    if (jsonObject.find(OUTS) == jsonObject.end()) {
        DHLOGE("VideoDecoder OUTS is invalid");
        return;
    }
    nlohmann::json videoDecoderOutsJson = jsonObject[OUTS];
    for (auto &outJson : videoDecoderOutsJson) {
        VideoDecoderOut out;
        FromJson(outJson, out);
        videoDecoder.outs.push_back(out);
    }
}

template<typename T>
void FromJson(const std::string &key, const nlohmann::json &jsonObject, std::vector<T> &objs)
{
    if (jsonObject.find(key) == jsonObject.end()) {
        DHLOGE("JSONObject key invalid, key: %s", key.c_str());
        return;
    }
    for (auto &json : jsonObject[key]) {
        T obj;
        FromJson(json, obj);
        objs.push_back(obj);
    }
}
template
void FromJson<AudioEncoder>(const std::string &key, const nlohmann::json &jsonObject, std::vector<AudioEncoder> &objs);
template
void FromJson<AudioDecoder>(const std::string &key, const nlohmann::json &jsonObject, std::vector<AudioDecoder> &objs);
template
void FromJson<VideoEncoder>(const std::string &key, const nlohmann::json &jsonObject, std::vector<VideoEncoder> &objs);
template
void FromJson<VideoDecoder>(const std::string &key, const nlohmann::json &jsonObject, std::vector<VideoDecoder> &objs);
}
}