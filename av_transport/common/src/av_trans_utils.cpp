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

#include "av_trans_utils.h"

#include <cstddef>
#include <securec.h>

#include "av_trans_constants.h"
#include "av_trans_log.h"
#include "av_trans_meta.h"

#include "plugin/common/share_allocator.h"

namespace OHOS {
namespace DistributedHardware {
using HiSBufferMeta = OHOS::Media::Plugin::BufferMeta;
using TransBufferMeta = OHOS::DistributedHardware::BufferMeta;

const std::string KEY_OWNER_NAME = "ownerName";
const std::string KEY_PEER_DEVID = "peerDevId";

std::string TransName2PkgName(const std::string &ownerName)
{
    const static std::pair<std::string, std::string> mapArray[] = {
        {OWNER_NAME_D_MIC, PKG_NAME_D_AUDIO},
        {OWNER_NAME_D_VIRMODEM_MIC, PKG_NAME_D_CALL},
        {OWNER_NAME_D_CAMERA, PKG_NAME_D_CAMERA},
        {OWNER_NAME_D_SCREEN, PKG_NAME_D_SCREEN},
        {OWNER_NAME_D_SPEAKER, PKG_NAME_D_AUDIO},
        {OWNER_NAME_D_VIRMODEM_SPEAKER, PKG_NAME_D_CALL},
    };
    for (const auto& item : mapArray) {
        if (item.first == ownerName) {
            return item.second;
        }
    }
    return EMPTY_STRING;
}

OHOS::Media::Plugin::MediaType TransName2MediaType(const std::string &ownerName)
{
    const static std::pair<std::string, OHOS::Media::Plugin::MediaType> mapArray[] = {
        {OWNER_NAME_D_MIC, OHOS::Media::Plugin::MediaType::AUDIO},
        {OWNER_NAME_D_VIRMODEM_MIC, OHOS::Media::Plugin::MediaType::AUDIO},
        {OWNER_NAME_D_CAMERA, OHOS::Media::Plugin::MediaType::VIDEO},
        {OWNER_NAME_D_SCREEN, OHOS::Media::Plugin::MediaType::VIDEO},
        {OWNER_NAME_D_SPEAKER, OHOS::Media::Plugin::MediaType::AUDIO},
        {OWNER_NAME_D_VIRMODEM_SPEAKER, OHOS::Media::Plugin::MediaType::AUDIO},
    };
    for (const auto& item : mapArray) {
        if (item.first == ownerName) {
            return item.second;
        }
    }
    return OHOS::Media::Plugin::MediaType::UNKNOWN;
}

std::string BuildChannelDescription(const std::string &ownerName, const std::string &peerDevId)
{
    nlohmann::json descJson;
    descJson[KEY_OWNER_NAME] = ownerName;
    descJson[KEY_PEER_DEVID] = peerDevId;
    return descJson.dump();
}

void ParseChannelDescription(const std::string &descJsonStr, std::string &ownerName, std::string &peerDevId)
{
    nlohmann::json descJson = nlohmann::json::parse(descJsonStr, nullptr, false);
    if (descJson.is_discarded()) {
        return;
    }
    if (!descJson.contains(KEY_OWNER_NAME) || !descJson.contains(KEY_PEER_DEVID)) {
        return;
    }
    if (!IsString(descJson, KEY_OWNER_NAME) || !IsString(descJson, KEY_PEER_DEVID)) {
        return;
    }
    ownerName = descJson[KEY_OWNER_NAME].get<std::string>();
    peerDevId = descJson[KEY_PEER_DEVID].get<std::string>();
}

std::shared_ptr<AVBuffer> TransBuffer2HiSBuffer(const std::shared_ptr<AVTransBuffer>& transBuffer)
{
    if ((transBuffer == nullptr) || transBuffer->IsEmpty()) {
        return nullptr;
    }

    auto data = transBuffer->GetBufferData();
    if (data == nullptr) {
        return nullptr;
    }

    auto hisBuffer = std::make_shared<AVBuffer>();
    hisBuffer->WrapMemory(data->GetAddress(), data->GetCapacity(), data->GetSize());

    Convert2HiSBufferMeta(transBuffer, hisBuffer);
    return hisBuffer;
}

std::shared_ptr<AVTransBuffer> HiSBuffer2TransBuffer(const std::shared_ptr<AVBuffer>& hisBuffer)
{
    if ((hisBuffer == nullptr) || hisBuffer->IsEmpty()) {
        return nullptr;
    }

    auto memory = hisBuffer->GetMemory();
    if (memory == nullptr) {
        return nullptr;
    }

    auto transBuffer = std::make_shared<AVTransBuffer>();
    transBuffer->WrapBufferData(memory->GetReadOnlyData(), memory->GetCapacity(), memory->GetSize());

    Convert2TransBufferMeta(hisBuffer, transBuffer);
    return transBuffer;
}

void Convert2HiSBufferMeta(std::shared_ptr<AVTransBuffer> transBuffer, std::shared_ptr<AVBuffer> hisBuffer)
{
    std::shared_ptr<TransBufferMeta> transMeta = transBuffer->GetBufferMeta();
    if ((transMeta->GetMetaType() == MetaType::AUDIO)) {
        auto hisAMeta = std::make_shared<AVTransAudioBufferMeta>();

        std::string value;
        transMeta->GetMetaItem(AVTransTag::BUFFER_DATA_TYPE, value);
        uint32_t dataType = static_cast<uint32_t>(std::atoi(value.c_str()));
        hisAMeta->dataType_ = (BufferDataType)dataType;

        transMeta->GetMetaItem(AVTransTag::AUDIO_SAMPLE_FORMAT, value);
        uint32_t format = static_cast<uint32_t>(std::atoi(value.c_str()));
        hisAMeta->format_ = (AudioSampleFormat)format;

        transMeta->GetMetaItem(AVTransTag::AUDIO_SAMPLE_RATE, value);
        hisAMeta->sampleRate_ = static_cast<uint32_t>(std::atoi(value.c_str()));

        hisBuffer->UpdateBufferMeta(*hisAMeta);
    } else {
        auto hisVMeta = std::make_shared<AVTransVideoBufferMeta>();

        std::string value;
        transMeta->GetMetaItem(AVTransTag::BUFFER_DATA_TYPE, value);
        uint32_t dataType = static_cast<uint32_t>(std::atoi(value.c_str()));
        hisVMeta->dataType_ = (BufferDataType)dataType;

        transMeta->GetMetaItem(AVTransTag::VIDEO_PIXEL_FORMAT, value);
        uint32_t format = static_cast<uint32_t>(std::atoi(value.c_str()));
        hisVMeta->format_ = (VideoPixelFormat)format;

        transMeta->GetMetaItem(AVTransTag::VIDEO_WIDTH, value);
        hisVMeta->width_ = static_cast<uint32_t>(std::atoi(value.c_str()));

        transMeta->GetMetaItem(AVTransTag::VIDEO_HEIGHT, value);
        hisVMeta->height_ = static_cast<uint32_t>(std::atoi(value.c_str()));

        TRUE_LOG_MSG(!transMeta->GetMetaItem(AVTransTag::PRE_TIMESTAMP, value), "get PRE_TIMESTAMP meta failed");
        hisVMeta->pts_ = std::stoll(value.c_str());

        hisBuffer->pts = std::stoll(value.c_str());
        hisBuffer->UpdateBufferMeta(*hisVMeta);
    }
}

void Convert2TransBufferMeta(std::shared_ptr<AVBuffer> hisBuffer, std::shared_ptr<AVTransBuffer> transBuffer)
{
    std::shared_ptr<HiSBufferMeta> hisMeta = hisBuffer->GetBufferMeta();
    if ((hisMeta->GetType() == BufferMetaType::AUDIO)) {
        std::shared_ptr<AVTransAudioBufferMeta> hisAMeta = ReinterpretCastPointer<AVTransAudioBufferMeta>(hisMeta);
        TRUE_RETURN(hisAMeta == nullptr, "hisAMeta is null");

        auto transAMeta = std::make_shared<TransBufferMeta>(MetaType::AUDIO);
        transAMeta->SetMetaItem(AVTransTag::BUFFER_DATA_TYPE, std::to_string((uint32_t)(hisAMeta->dataType_)));
        transAMeta->SetMetaItem(AVTransTag::AUDIO_SAMPLE_FORMAT, std::to_string((uint32_t)(hisAMeta->format_)));
        transAMeta->SetMetaItem(AVTransTag::AUDIO_SAMPLE_RATE, std::to_string(hisAMeta->sampleRate_));

        transBuffer->UpdateBufferMeta(transAMeta);
    } else {
        std::shared_ptr<AVTransVideoBufferMeta> hisVMeta = ReinterpretCastPointer<AVTransVideoBufferMeta>(hisMeta);
        TRUE_RETURN(hisVMeta == nullptr, "hisAMeta is null");

        auto transVMeta = std::make_shared<TransBufferMeta>(MetaType::VIDEO);
        transVMeta->SetMetaItem(AVTransTag::BUFFER_DATA_TYPE, std::to_string((uint32_t)(hisVMeta->dataType_)));
        transVMeta->SetMetaItem(AVTransTag::VIDEO_PIXEL_FORMAT, std::to_string((uint32_t)(hisVMeta->format_)));
        transVMeta->SetMetaItem(AVTransTag::VIDEO_WIDTH, std::to_string(hisVMeta->width_));
        transVMeta->SetMetaItem(AVTransTag::VIDEO_HEIGHT, std::to_string(hisVMeta->height_));
        transVMeta->SetMetaItem(AVTransTag::PRE_TIMESTAMP, std::to_string(hisVMeta->pts_));

        transBuffer->UpdateBufferMeta(transVMeta);
    }
}

EventType CastEventType(Plugin::PluginEventType type, bool isAbnormal)
{
    switch (type) {
        case Plugin::PluginEventType::EVENT_CHANNEL_OPENED:
            return EventType::EVENT_START_SUCCESS;
        case Plugin::PluginEventType::EVENT_CHANNEL_OPEN_FAIL:
            return EventType::EVENT_START_FAIL;
        case Plugin::PluginEventType::EVENT_CHANNEL_CLOSED:
            return isAbnormal ? EventType::EVENT_ENGINE_ERROR : EventType::EVENT_STOP_SUCCESS;
        default:
            AVTRANS_LOGE("unsupport plugin event type.");
    }
    return EventType::EVENT_ENGINE_ERROR;
}

void DumpBufferToFile(std::string fileName, uint8_t *buffer, int32_t bufSize)
{
    if (fileName.empty()) {
        AVTRANS_LOGE("input fileName is empty.");
        return;
    }
    std::ofstream ofs(fileName, std::ios::binary | std::ios::out | std::ios::app);
    if (!ofs.is_open()) {
        AVTRANS_LOGE("open file failed.");
        return;
    }
    ofs.write((const char*)(buffer), bufSize);
    ofs.close();
}

bool IsUInt32(const nlohmann::json &jsonObj, const std::string &key)
{
    return jsonObj.contains(key) && jsonObj[key].is_number_unsigned() && jsonObj[key] <= UINT32_MAX;
}

bool IsInt64(const nlohmann::json &jsonObj, const std::string &key)
{
    return jsonObj.contains(key) && jsonObj[key].is_number_integer() && INT64_MIN <= jsonObj[key] &&
        jsonObj[key] <= INT64_MAX;
}

bool IsString(const nlohmann::json &jsonObj, const std::string &key)
{
    return jsonObj.contains(key) && jsonObj[key].is_string() && jsonObj[key].size() <= MAX_MESSAGES_LEN;
}

int64_t GetCurrentTime()
{
    struct timespec time = { 0, 0 };
    clock_gettime(CLOCK_MONOTONIC, &time);
    return time.tv_sec * NS_ONE_S + time.tv_nsec;
}

void GenerateAdtsHeader(unsigned char* adtsHeader, uint32_t packetLen, uint32_t profile, uint32_t sampleRate,
    uint32_t channels)
{
    static std::map<int, uint32_t> mapSampleRateToFreIndex {
        {96000, 0},
        {88200, 1},
        {64000, 2},
        {48000, 3},
        {44100, 4},
        {32000, 5},
        {24000, 6},
        {16000, 8},
        {12000, 9},
        {11025, 10},
        {8000, 11},
        {7350, 12},
    };
    // profile only support AAC LC: 1
    uint32_t freqIdx = mapSampleRateToFreIndex[sampleRate]; // 48KHz : 3
    adtsHeader[0] = (unsigned char) 0xFF;
    adtsHeader[1] = (unsigned char) 0xF9;
    adtsHeader[2] = (unsigned char) (((profile - 1) << 6) + (freqIdx << 2) + (channels >> 2));
    adtsHeader[3] = (unsigned char) (((channels & 3) << 6) + (packetLen >> 11));
    adtsHeader[4] = (unsigned char) ((packetLen & 0x7FF) >> 3);
    adtsHeader[5] = (unsigned char) (((packetLen & 7) << 5) + 0x1F);
    adtsHeader[6] = (unsigned char) 0xFC;
}
} // namespace DistributedHardware
} // namespace OHOS