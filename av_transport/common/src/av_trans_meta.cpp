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

#include "av_trans_meta.h"

#include "av_trans_utils.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace DistributedHardware {
const std::string META_DATA_TYPE = "meta_data_type";
const std::string META_TIMESTAMP = "meta_timestamp";
const std::string META_FRAME_NUMBER = "meta_frame_number";

std::shared_ptr<OHOS::Media::Plugin::BufferMeta> AVTransAudioBufferMeta::Clone()
{
    auto bufferMeta = std::make_shared<AVTransAudioBufferMeta>();
    bufferMeta->pts_ = pts_;
    bufferMeta->cts_ = cts_;
    bufferMeta->format_ = format_;
    bufferMeta->dataType_ = dataType_;
    bufferMeta->frameNum_ = frameNum_;
    bufferMeta->channels_ = channels_;
    bufferMeta->sampleRate_ = sampleRate_;
    bufferMeta->Update(*this);
    return bufferMeta;
}

std::string AVTransAudioBufferMeta::MarshalAudioMeta()
{
    nlohmann::json metaJson;
    metaJson[META_DATA_TYPE] = dataType_;
    metaJson[META_TIMESTAMP] = pts_;
    metaJson[META_FRAME_NUMBER] = frameNum_;
    return metaJson.dump();
}

bool AVTransAudioBufferMeta::UnmarshalAudioMeta(const std::string& jsonStr)
{
    nlohmann::json metaJson = nlohmann::json::parse(jsonStr, nullptr, false);
    if (metaJson.is_discarded()) {
        return false;
    }
    if (!IsUInt32(metaJson, META_DATA_TYPE) || !IsInt64(metaJson, META_TIMESTAMP) ||
        !IsUInt32(metaJson, META_FRAME_NUMBER)) {
        return false;
    }
    dataType_ = metaJson[META_DATA_TYPE].get<BufferDataType>();
    pts_ = metaJson[META_TIMESTAMP].get<int64_t>();
    frameNum_ = metaJson[META_FRAME_NUMBER].get<uint32_t>();
    return true;
}

std::shared_ptr<OHOS::Media::Plugin::BufferMeta> AVTransVideoBufferMeta::Clone()
{
    auto bufferMeta = std::make_shared<AVTransVideoBufferMeta>();
    bufferMeta->pts_ = pts_;
    bufferMeta->cts_ = cts_;
    bufferMeta->width_ = width_;
    bufferMeta->height_ = height_;
    bufferMeta->format_ = format_;
    bufferMeta->dataType_ = dataType_;
    bufferMeta->frameNum_ = frameNum_;
    bufferMeta->Update(*this);
    return bufferMeta;
}

std::string AVTransVideoBufferMeta::MarshalVideoMeta()
{
    nlohmann::json metaJson;
    metaJson[META_DATA_TYPE] = dataType_;
    metaJson[META_TIMESTAMP] = pts_;
    metaJson[META_FRAME_NUMBER] = frameNum_;
    return metaJson.dump();
}

bool AVTransVideoBufferMeta::UnmarshalVideoMeta(const std::string& jsonStr)
{
    nlohmann::json metaJson = nlohmann::json::parse(jsonStr, nullptr, false);
    if (metaJson.is_discarded()) {
        return false;
    }
    if (!IsUInt32(metaJson, META_DATA_TYPE) || !IsInt64(metaJson, META_TIMESTAMP) ||
        !IsUInt32(metaJson, META_FRAME_NUMBER)) {
        return false;
    }
    dataType_ = metaJson[META_DATA_TYPE].get<BufferDataType>();
    pts_ = metaJson[META_TIMESTAMP].get<int64_t>();
    frameNum_ = metaJson[META_FRAME_NUMBER].get<uint32_t>();
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS