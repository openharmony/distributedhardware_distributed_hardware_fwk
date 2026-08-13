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

#include "plugin_manager.h"
#include "plugin_caps.h"

using OHOS::Media::Plugin::PluginManager;
using OHOS::Media::Plugin::PluginType;
using OHOS::Media::Plugin::PluginInfo;
using OHOS::Media::Plugin::Capability;
using OHOS::Media::Plugin::CapabilitySet;
using OHOS::Media::Plugin::CodecMode;
using CapabilityID = OHOS::Media::Plugin::Capability::Key;

namespace {
static const uint32_t VIDEO_ENCODER_FOUR = 4;
const std::vector<std::string> AUDIO_ENCODER_NAMES = { "ffmpegAuEnc_aac" };
const std::vector<std::string> AUDIO_DECODER_NAMES = { "ffmpegAuDec_aac" };
const std::vector<std::string> VIDEO_ENCODER_NAMES = {
    "HdiCodecAdapter.OMX.rk.video_encoder.hevc"
};
const std::vector<std::string> VIDEO_DECODER_NAMES = {
    "HdiCodecAdapter.OMX.rk.video_decoder.hevc"
};
}

namespace OHOS {
namespace Media {
namespace Plugin {

PluginManager::PluginManager()
{
}

void PluginManager::Init()
{
}

std::vector<std::string> PluginManager::ListPlugins(PluginType pluginType, CodecMode preferredCodecMode)
{
    switch (pluginType) {
        case PluginType::AUDIO_ENCODER:
            return AUDIO_ENCODER_NAMES;
        case PluginType::AUDIO_DECODER:
            return AUDIO_DECODER_NAMES;
        case PluginType::VIDEO_ENCODER:
            return VIDEO_ENCODER_NAMES;
        case PluginType::VIDEO_DECODER:
            return VIDEO_DECODER_NAMES;
        default:
            return {};
    }
}

std::shared_ptr<PluginInfo> PluginManager::GetPluginInfo(PluginType type, const std::string& name)
{
    auto info = std::make_shared<PluginInfo>();
    info->name = name;
    info->pluginType = type;

    Capability inCap;
    inCap.mime = (type == PluginType::AUDIO_ENCODER || type == PluginType::AUDIO_DECODER)
        ? "audio/raw" : "video/raw";
    if (type == PluginType::AUDIO_ENCODER) {
        inCap.keys[CapabilityID::AUDIO_SAMPLE_RATE] =
            std::vector<uint32_t>{ 96000, 88200, 64000, 48000, 44100, 32000 };
    } else if (type == PluginType::AUDIO_DECODER) {
        inCap.keys[CapabilityID::AUDIO_CHANNEL_LAYOUT] =
            std::vector<AudioChannelLayout>{ AudioChannelLayout::CH_2POINT1, AudioChannelLayout::CH_2_1 };
    } else if (type == PluginType::VIDEO_ENCODER) {
        inCap.keys[CapabilityID::VIDEO_PIXEL_FORMAT] =
            std::vector<VideoPixelFormat>{ VideoPixelFormat::YUV410P, VideoPixelFormat::RGBA };
    } else if (type == PluginType::VIDEO_DECODER) {
        inCap.keys[CapabilityID::VIDEO_BIT_STREAM_FORMAT] =
            std::vector<VideoBitStreamFormat>{ VideoBitStreamFormat::AVC1, VideoBitStreamFormat::HEVC };
    }
    CapabilitySet inCaps = { inCap };
    info->inCaps = inCaps;

    Capability outCap;
    outCap.mime = (type == PluginType::AUDIO_ENCODER) ? "audio/mp4a-latm"
        : (type == PluginType::AUDIO_DECODER) ? "audio/raw"
        : (type == PluginType::VIDEO_ENCODER) ? "video/hevc"
        : "video/raw";
    if (type == PluginType::AUDIO_ENCODER) {
        outCap.keys[CapabilityID::AUDIO_MPEG_VERSION] = static_cast<uint32_t>(VIDEO_ENCODER_FOUR);
    } else if (type == PluginType::AUDIO_DECODER) {
        outCap.keys[CapabilityID::AUDIO_SAMPLE_FORMAT] =
            std::vector<AudioSampleFormat>{ AudioSampleFormat::S16 };
    } else if (type == PluginType::VIDEO_ENCODER || type == PluginType::VIDEO_DECODER) {
        outCap.keys[CapabilityID::VIDEO_PIXEL_FORMAT] =
            std::vector<VideoPixelFormat>{ VideoPixelFormat::YUV410P, VideoPixelFormat::RGBA };
    }
    CapabilitySet outCaps = { outCap };
    info->outCaps = outCaps;

    return info;
}

} // namespace Plugin
} // namespace Media
} // namespace OHOS