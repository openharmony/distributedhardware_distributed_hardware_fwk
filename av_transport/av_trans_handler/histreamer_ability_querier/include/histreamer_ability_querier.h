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

#ifndef OHOS_HISTREAMER_ABILITY_QUERIER_H
#define OHOS_HISTREAMER_ABILITY_QUERIER_H

#include <cstdint>
#include <string>
#include <vector>

#include "cJSON.h"

#include "plugin_audio_tags.h"
#include "plugin_video_tags.h"

using OHOS::Media::Plugin::AudioAacProfile;
using OHOS::Media::Plugin::AudioAacStreamFormat;
using OHOS::Media::Plugin::AudioChannelLayout;
using OHOS::Media::Plugin::AudioSampleFormat;
using OHOS::Media::Plugin::VideoBitStreamFormat;
using OHOS::Media::Plugin::VideoPixelFormat;

namespace OHOS {
namespace DistributedHardware {

/******************* AudioEncoder Begin *****************/
struct AudioEncoderIn {
    std::string mime;
    std::vector<uint32_t> sample_rate;
};

struct AudioEncoderOut {
    std::string mime;
    uint32_t ad_mpeg_ver;
    AudioAacProfile aac_profile;
    AudioAacStreamFormat aac_stm_fmt;
};

struct AudioEncoder {
    std::string name;
    std::vector<AudioEncoderIn> ins;
    std::vector<AudioEncoderOut> outs;
};

void ToJson(cJSON *jsonObject, const AudioEncoderIn &audioEncoderIn);
void FromJson(const cJSON *jsonObject, AudioEncoderIn &audioEncoderIn);

void ToJson(cJSON *jsonObject, const AudioEncoderOut &audioEncoderOut);
void FromJson(const cJSON *jsonObject, AudioEncoderOut &audioEncoderOut);

void ToJson(cJSON *jsonObject, const AudioEncoder &audioEncoder);
void FromJson(const cJSON *jsonObject, AudioEncoder &audioEncoder);
/******************* AudioEncoder End *******************/

/******************* AudioDecoder Begin *****************/
struct AudioDecoderIn {
    std::string mime;
    std::vector<AudioChannelLayout> channel_layout;
};

struct AudioDecoderOut {
    std::string mime;
    std::vector<AudioSampleFormat> sample_fmt;
};

struct AudioDecoder {
    std::string name;
    std::vector<AudioDecoderIn> ins;
    std::vector<AudioDecoderOut> outs;
};

void ToJson(cJSON *jsonObject, const AudioDecoderIn &audioDecoderIn);
void FromJson(const cJSON *jsonObject, AudioDecoderIn &audioDecoderIn);

void ToJson(cJSON *jsonObject, const AudioDecoderOut &audioDecoderOut);
void FromJson(const cJSON *jsonObject, AudioDecoderOut &audioDecoderOut);

void ToJson(cJSON *jsonObject, const AudioDecoder &audioDecoder);
void FromJson(const cJSON *jsonObject, AudioDecoder &audioDecoder);
/******************* AudioDecoder End *******************/

/******************* VideoEncoder Begin *****************/
struct VideoEncoderIn {
    std::string mime;
    std::vector<VideoPixelFormat> pixel_fmt;
};

struct VideoEncoderOut {
    std::string mime;
};

struct VideoEncoder {
    std::string name;
    std::vector<VideoEncoderIn> ins;
    std::vector<VideoEncoderOut> outs;
};

void ToJson(cJSON *jsonObject, const VideoEncoderIn &videoEncoderIn);
void FromJson(const cJSON *jsonObject, VideoEncoderIn &videoEncoderIn);

void ToJson(cJSON *jsonObject, const VideoEncoderOut &videoEncoderOut);
void FromJson(const cJSON *jsonObject, VideoEncoderOut &videoEncoderOut);

void ToJson(cJSON *jsonObject, const VideoEncoder &videoEncoder);
void FromJson(const cJSON *jsonObject, VideoEncoder &videoEncoder);
/******************* VideoEncoder End *******************/

/******************* VideoDecoder Begin *****************/
struct VideoDecoderIn {
    std::string mime;
    std::vector<VideoBitStreamFormat> vd_bit_stream_fmt;
};

struct VideoDecoderOut {
    std::string mime;
    std::vector<VideoPixelFormat> pixel_fmt;
};

struct VideoDecoder {
    std::string name;
    std::vector<VideoDecoderIn> ins;
    std::vector<VideoDecoderOut> outs;
};

void ToJson(cJSON *jsonObject, const VideoDecoderIn &videoDecoderIn);
void FromJson(const cJSON *jsonObject, VideoDecoderIn &videoDecoderIn);

void ToJson(cJSON *jsonObject, const VideoDecoderOut &videoDecoderOut);
void FromJson(const cJSON *jsonObject, VideoDecoderOut &videoDecoderOut);

void ToJson(cJSON *jsonObject, const VideoDecoder &videoDecoder);
void FromJson(const cJSON *jsonObject, VideoDecoder &videoDecoder);
/******************* VideoDecoder End *******************/

std::vector<AudioEncoder> QueryAudioEncoderAbility();
std::vector<AudioDecoder> QueryAudioDecoderAbility();
std::vector<VideoEncoder> QueryVideoEncoderAbility();
std::vector<VideoDecoder> QueryVideoDecoderAbility();

template<typename T>
void ToJson(const std::string &key, cJSON *jsonObject, std::vector<T> &objs);
template<typename T>
void FromJson(const std::string &key, const cJSON *jsonObject, std::vector<T> &objs);

#ifdef __cplusplus
extern "C" {
#endif
__attribute__((visibility("default"))) int32_t QueryAudioEncoderAbilityStr(char* res);
__attribute__((visibility("default"))) int32_t QueryAudioDecoderAbilityStr(char* res);
__attribute__((visibility("default"))) int32_t QueryVideoEncoderAbilityStr(char* res);
__attribute__((visibility("default"))) int32_t QueryVideoDecoderAbilityStr(char* res);
#ifdef __cplusplus
}
#endif
}
}
#endif