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

#ifndef OHOS_AV_TRANSPORT_EXTEND_META_H
#define OHOS_AV_TRANSPORT_EXTEND_META_H

#include "av_trans_types.h"

// follwing head files depends on histreamer
#include "plugin_buffer.h"

namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::Media;
using namespace OHOS::Media::Plugin;

/**
 * @brief av transport audio buffer metadata.
 * Buffer metadata describing how data is laid out inside the buffer
 */
class AVTransAudioBufferMeta : public OHOS::Media::Plugin::BufferMeta {
public:
    AVTransAudioBufferMeta() : OHOS::Media::Plugin::BufferMeta(BufferMetaType::AUDIO) {}
    ~AVTransAudioBufferMeta() override = default;

    std::shared_ptr<OHOS::Media::Plugin::BufferMeta> Clone() override;

    std::string MarshalAudioMeta();
    bool UnmarshalAudioMeta(const std::string &jsonStr);

public:
    int64_t pts_ {0};

    int64_t cts_ {0};

    uint32_t frameNum_ {0};

    uint32_t channels_ {0};

    uint32_t sampleRate_ {0};

    BufferDataType dataType_ {BufferDataType::AUDIO};

    AudioSampleFormat format_ {AudioSampleFormat::S8};

private:
    friend class Buffer;
};

/**
 * @brief av transport video buffer metadata.
 *  Extra buffer metadata describing video properties.
 */
class AVTransVideoBufferMeta : public OHOS::Media::Plugin::BufferMeta {
public:
    AVTransVideoBufferMeta() : OHOS::Media::Plugin::BufferMeta(BufferMetaType::VIDEO) {}
    ~AVTransVideoBufferMeta() override = default;

    std::shared_ptr<OHOS::Media::Plugin::BufferMeta> Clone() override;

    std::string MarshalVideoMeta();
    bool UnmarshalVideoMeta(const std::string &jsonStr);

    int64_t pts_ {0};

    int64_t cts_ {0};

    uint32_t width_ {0};

    uint32_t height_ {0};

    uint32_t frameNum_ {0};

    VideoPixelFormat format_ {VideoPixelFormat::UNKNOWN};

    BufferDataType dataType_ {BufferDataType::VIDEO_STREAM};

private:
    friend class Buffer;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AV_TRANSPORT_EXTEND_META_H