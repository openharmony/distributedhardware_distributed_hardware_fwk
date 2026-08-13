/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_AV_PIPELINE_EVENT_H
#define OHOS_AV_PIPELINE_EVENT_H

#include <map>
#include <string>

#include "meta/any.h"

namespace OHOS {
namespace DistributedHardware {
namespace Pipeline {
enum struct EventType : uint32_t {
    EVENT_READY = 0,
    EVENT_AUDIO_PROGRESS, // unit is HST_TIME_BASE
    EVENT_VIDEO_PROGRESS, // unit is HST_TIME_BASE
    EVENT_COMPLETE,
    EVENT_ERROR,
    EVENT_PLUGIN_ERROR,
    EVENT_PLUGIN_EVENT,
    EVENT_BUFFERING,
    EVENT_BUFFER_PROGRESS,
    EVENT_DECODER_ERROR,
    EVENT_RESOLUTION_CHANGE,
    EVENT_VIDEO_RENDERING_START,
    EVENT_IS_LIVE_STREAM,
    EVENT_DRM_INFO_UPDATED,
    EVENT_AUDIO_INTERRUPT,
    EVENT_AUDIO_STATE_CHANGE,
    EVENT_AUDIO_FIRST_FRAME,
    EVENT_AUDIO_DEVICE_CHANGE,
    EVENT_AUDIO_SERVICE_DIED,
    BUFFERING_START,
    BUFFERING_END,
    EVENT_CACHED_DURATION,
    EVENT_SOURCE_BITRATE_START,
    EVENT_SUBTITLE_TEXT_UPDATE,
    EVENT_AUDIO_TRACK_CHANGE,
    EVENT_VIDEO_TRACK_CHANGE,
    EVENT_SUBTITLE_TRACK_CHANGE,
    EVENT_VIDEO_LAG, // player lag event detected by video sink
    EVENT_AUDIO_LAG, // player lag event detected by audio sink
    EVENT_STREAM_LAG, // player lag event detected by sync manager
};

struct Event {
    std::string srcFilter;
    EventType type;
    Media::Any param;
};

typedef enum {
    AUDIO_CODEC_AAC = 0,
    AUDIO_CODEC_FLAC = 1,
    AUDIO_CODEC_AAC_EN = 2,
    AUDIO_CODEC_OPUS = 3
} AudioCodecType;

const char* GetEventName(EventType type);
} // namespace Pipeline
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AV_PIPELINE_EVENT_H
