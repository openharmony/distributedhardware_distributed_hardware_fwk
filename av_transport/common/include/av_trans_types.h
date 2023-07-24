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

#ifndef OHOS_AV_TRANSPORT_TYPES_H
#define OHOS_AV_TRANSPORT_TYPES_H

#include <string>
#include <unistd.h>

namespace OHOS {
namespace DistributedHardware {
const std::string OWNER_NAME_D_CAMERA = "ohos.dhardware.dcamera";
const std::string OWNER_NAME_D_SCREEN = "ohos.dhardware.dscreen";
const std::string OWNER_NAME_D_MIC = "ohos.dhardware.daudio.dmic";
const std::string OWNER_NAME_D_SPEAKER = "ohos.dhardware.daudio.dspeaker";

const std::string SCENE_TYPE_D_MIC = "dmic_stream";
const std::string SCENE_TYPE_D_SCREEN = "dscreen_stream";
const std::string SCENE_TYPE_D_SPEAKER = "dspeaker_stream";
const std::string SCENE_TYPE_D_CAMERA_STR = "dcamera_stream";
const std::string SCENE_TYPE_D_CAMERA_PIC = "dcamera_picture";

const std::string PKG_NAME_DH_FWK = "ohos.dhardware";
const std::string PKG_NAME_D_AUDIO = "ohos.dhardware.daudio";
const std::string PKG_NAME_D_CAMERA = "ohos.dhardware.dcamera";
const std::string PKG_NAME_D_SCREEN = "ohos.dhardware.dscreen";

const std::string MIME_VIDEO_RAW = "video/raw";
const std::string MIME_VIDEO_H264 = "video/avc";
const std::string MIME_VIDEO_H265 = "video/hevc";

const std::string VIDEO_FORMAT_NV12 = "nv12";
const std::string VIDEO_FORMAT_NV21 = "nv21";
const std::string VIDEO_FORMAT_JEPG = "jpeg";
const std::string VIDEO_FORMAT_YUVI420 = "yuvi420";
const std::string VIDEO_FORMAT_RGBA8888 = "rgba8888";

enum struct TransRole : uint32_t {
    AV_SENDER = 0,
    AV_RECEIVER = 1,
    UNKNOWN = 2
};

enum struct AvSyncFlag : uint32_t {
    MASTER = 0,
    SLAVE = 1,
    UNKNOWN = 2
};

enum struct TransStrategy : uint32_t {
    LOW_LATANCY_STRATEGY,
    LOW_JITTER_STRATEGY
};

struct ChannelAttribute {
    TransStrategy strategy;
};

enum struct BufferDataType : uint32_t {
    AUDIO = 0,
    VIDEO_STREAM,
    PICTURE,
    UNKNOW,
};

enum struct StateId : uint32_t {
    IDLE = 0,
    INITIALIZED = 1,
    CH_CREATING = 2,
    CH_CREATED = 3,
    STARTED = 4,
    PLAYING = 5,
    STOPPED = 6,
    BUTT,
};

enum struct TagSection : uint8_t {
    REGULAR = 1,
    D_AUDIO = 2,
    D_VIDEO = 3,
    MAX_SECTION = 64
};

enum struct AVTransTag : uint32_t {
    INVALID = 0,
    SECTION_REGULAR_START = static_cast<uint8_t>(TagSection::REGULAR) << 16U,
    SECTION_D_AUDIO_START = static_cast<uint8_t>(TagSection::D_AUDIO) << 16U,
    SECTION_D_VIDEO_START = static_cast<uint8_t>(TagSection::D_VIDEO) << 16U,

    /* -------------------- regular tag -------------------- */
    FRAME_NUMBER = SECTION_REGULAR_START + 1,
    BUFFER_DATA_TYPE,
    PRE_TIMESTAMP,
    CUR_TIMESTAMP,
    ENGINE_READY,
    ENGINE_PAUSE,
    ENGINE_RESUME,
    START_AV_SYNC,
    STOP_AV_SYNC,
    TIME_SYNC_RESULT,
    SHARED_MEMORY_FD,

    /* -------------------- d_audio tag -------------------- */
    AUDIO_CHANNELS = SECTION_D_AUDIO_START + 1,
    AUDIO_SAMPLE_RATE,
    AUDIO_CODEC_TYPE,
    AUDIO_CHANNEL_MASK,
    AUDIO_SAMPLE_FORMAT,
    AUDIO_FRAME_SIZE,
    AUDIO_STREAM_USAGE,
    AUDIO_RENDER_FLAGS,
    AUDIO_CONTENT_TYPE,
    AUDIO_CHANNEL_LAYOUT,
    AUDIO_BIT_RATE,

    /* -------------------- d_video tag -------------------- */
    VIDEO_WIDTH = SECTION_D_VIDEO_START + 1,
    VIDEO_HEIGHT,
    VIDEO_CODEC_TYPE,
    VIDEO_PIXEL_FORMAT,
    VIDEO_FRAME_RATE,
    VIDEO_BIT_RATE,
};

enum struct EventType : uint32_t {
    EVENT_CHANNEL_OPENED = 0,
    EVENT_CHANNEL_OPEN_FAIL = 1,
    EVENT_CHANNEL_CLOSED = 2,
    EVENT_START_SUCCESS = 3,
    EVENT_START_FAIL = 4,
    EVENT_STOP_SUCCESS = 5,
    EVENT_STOP_FAIL = 6,
    EVENT_ENGINE_ERROR = 7,
    EVENT_REMOTE_ERROR = 8,
    EVENT_DATA_RECEIVED = 9,
    EVENT_TIME_SYNC_RESULT = 10,
    EVENT_ADD_STREAM = 11,
    EVENT_REMOVE_STREAM = 12,
};

struct AVTransEvent {
    EventType type;
    std::string content;
    std::string peerDevId;
};

struct AVStreamInfo {
    std::string sceneType;
    std::string peerDevId;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AV_TRANSPORT_TYPES_H