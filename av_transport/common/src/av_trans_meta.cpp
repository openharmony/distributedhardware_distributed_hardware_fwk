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
#include "cJSON.h"

namespace OHOS {
namespace DistributedHardware {
const std::string META_DATA_TYPE = "meta_data_type";
const std::string META_TIMESTAMP = "meta_timestamp";
const std::string META_FRAME_NUMBER = "meta_frame_number";
const std::string META_EXT_TIMESTAMP = "meta_ext_timestamp";
const std::string META_EXT_FRAME_NUMBER = "meta_ext_frame_number";

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
    cJSON *metaJson = cJSON_CreateObject();
    if (metaJson == nullptr) {
        return "";
    }
    cJSON_AddNumberToObject(metaJson, META_DATA_TYPE.c_str(), static_cast<uint32_t>(dataType_));
    cJSON_AddNumberToObject(metaJson, META_TIMESTAMP.c_str(), pts_);
    cJSON_AddNumberToObject(metaJson, META_FRAME_NUMBER.c_str(), frameNum_);
    char *data = cJSON_PrintUnformatted(metaJson);
    if (data == nullptr) {
        cJSON_Delete(metaJson);
        return "";
    }
    std::string jsonstr(data);
    cJSON_free(data);
    cJSON_Delete(metaJson);
    return jsonstr;
}

bool AVTransAudioBufferMeta::UnmarshalAudioMeta(const std::string& jsonStr)
{
    cJSON *metaJson = cJSON_Parse(jsonStr.c_str());
    if (metaJson == nullptr) {
        return false;
    }
    if (!IsUInt32(metaJson, META_DATA_TYPE) || !IsInt64(metaJson, META_TIMESTAMP) ||
        !IsUInt32(metaJson, META_FRAME_NUMBER)) {
        cJSON_Delete(metaJson);
        return false;
    }
    cJSON *typeObj = cJSON_GetObjectItemCaseSensitive(metaJson, META_DATA_TYPE.c_str());
    if (typeObj == nullptr || !cJSON_IsNumber(typeObj)) {
        cJSON_Delete(metaJson);
        return false;
    }
    cJSON *ptsObj = cJSON_GetObjectItemCaseSensitive(metaJson, META_TIMESTAMP.c_str());
    if (ptsObj == nullptr || !cJSON_IsNumber(ptsObj)) {
        cJSON_Delete(metaJson);
        return false;
    }
    cJSON *frameObj = cJSON_GetObjectItemCaseSensitive(metaJson, META_FRAME_NUMBER.c_str());
    if (frameObj == nullptr || !cJSON_IsNumber(frameObj)) {
        cJSON_Delete(metaJson);
        return false;
    }
    dataType_ = static_cast<BufferDataType>(typeObj->valueint);
    pts_ = static_cast<int64_t>(ptsObj->valueint);
    frameNum_ = static_cast<uint32_t>(frameObj->valueint);
    cJSON_Delete(metaJson);
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
    bufferMeta->extPts_ = extPts_;
    bufferMeta->extFrameNum_ = extFrameNum_;
    bufferMeta->Update(*this);
    return bufferMeta;
}

std::string AVTransVideoBufferMeta::MarshalVideoMeta()
{
    cJSON *metaJson = cJSON_CreateObject();
    if (metaJson == nullptr) {
        return "";
    }
    cJSON_AddNumberToObject(metaJson, META_DATA_TYPE.c_str(), static_cast<uint32_t>(dataType_));
    cJSON_AddNumberToObject(metaJson, META_TIMESTAMP.c_str(), pts_);
    cJSON_AddNumberToObject(metaJson, META_FRAME_NUMBER.c_str(), frameNum_);
    if (extPts_ > 0) {
        cJSON_AddNumberToObject(metaJson, META_EXT_TIMESTAMP.c_str(), extPts_);
    }
    if (extFrameNum_ > 0) {
        cJSON_AddNumberToObject(metaJson, META_EXT_FRAME_NUMBER.c_str(), extFrameNum_);
    }
    char *data = cJSON_PrintUnformatted(metaJson);
    if (data == nullptr) {
        cJSON_Delete(metaJson);
        return "";
    }
    std::string jsonstr(data);
    cJSON_Delete(metaJson);
    cJSON_free(data);
    return jsonstr;
}

bool AVTransVideoBufferMeta::UnmarshalVideoMeta(const std::string& jsonStr)
{
    cJSON *metaJson = cJSON_Parse(jsonStr.c_str());
    if (metaJson == nullptr) {
        return false;
    }

    cJSON *typeObj = cJSON_GetObjectItemCaseSensitive(metaJson, META_DATA_TYPE.c_str());
    if (typeObj != nullptr && IsUInt32(metaJson, META_DATA_TYPE)) {
        dataType_ = static_cast<BufferDataType>(typeObj->valueint);
    }

    cJSON *timeStampObj = cJSON_GetObjectItemCaseSensitive(metaJson, META_TIMESTAMP.c_str());
    if (timeStampObj != nullptr && IsInt64(metaJson, META_TIMESTAMP)) {
        pts_ = static_cast<int64_t>(timeStampObj->valueint);
    }

    cJSON *numberObj = cJSON_GetObjectItemCaseSensitive(metaJson, META_FRAME_NUMBER.c_str());
    if (numberObj != nullptr && IsUInt32(metaJson, META_FRAME_NUMBER)) {
        frameNum_ = static_cast<uint32_t>(numberObj->valueint);
    }

    cJSON *extTimeStampObj = cJSON_GetObjectItemCaseSensitive(metaJson, META_EXT_TIMESTAMP.c_str());
    if (extTimeStampObj != nullptr && IsInt64(metaJson, META_EXT_TIMESTAMP)) {
        extPts_ = static_cast<int64_t>(extTimeStampObj->valueint);
    }

    cJSON *extNumberObj = cJSON_GetObjectItemCaseSensitive(metaJson, META_EXT_FRAME_NUMBER.c_str());
    if (extNumberObj != nullptr && IsUInt32(metaJson, META_EXT_FRAME_NUMBER)) {
        extFrameNum_ = static_cast<uint32_t>(extNumberObj->valueint);
    }
    cJSON_Delete(metaJson);
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS