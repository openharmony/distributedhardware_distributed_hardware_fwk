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

#ifndef OHOS_AV_TRANSPORT_UTILS_H
#define OHOS_AV_TRANSPORT_UTILS_H

#include <fstream>
#include <string>
#include <sys/time.h>
#include <unistd.h>

#include "av_trans_buffer.h"
#include "av_trans_types.h"
#include "nlohmann/json.hpp"

// follwing head files depends on histreamer
#include "event.h"
#include "plugin_event.h"
#include "plugin_buffer.h"
#include "plugin_source_tags.h"

namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::Media;
using namespace OHOS::Media::Plugin;
using AVBuffer = OHOS::Media::Plugin::Buffer;
constexpr int32_t MS_ONE_SECOND = 1000;

SrcInputType TransName2InputType(const std::string &ownerName);
SrcInputType TransName2SoftbusInputType(const std::string &ownerName);
MediaType TransName2MediaType(const std::string &ownerName);

std::shared_ptr<AVBuffer> TransBuffer2HiSBuffer(const std::shared_ptr<AVTransBuffer> &transBuffer);
std::shared_ptr<AVTransBuffer> HiSBuffer2TransBuffer(const std::shared_ptr<AVBuffer> &hisBuffer);
void Convert2HiSBufferMeta(std::shared_ptr<AVTransBuffer> transBuffer, std::shared_ptr<AVBuffer> hisBuffer);
void Convert2TransBufferMeta(std::shared_ptr<AVBuffer> hisBuffer, std::shared_ptr<AVTransBuffer> transBuffer);

std::string BuildChannelDescription(const std::string &ownerName, const std::string &peerDevId);
void ParseChannelDescription(const std::string &descJsonStr, std::string &ownerName, std::string &peerDevId);

EventType CastEventType(Plugin::PluginEventType type);
void DumpBufferToFile(std::string fileName, uint8_t *buffer, int32_t bufSize);

bool IsUInt32(const nlohmann::json &jsonObj, const std::string &key);
bool IsInt64(const nlohmann::json &jsonObj, const std::string &key);
bool IsString(const nlohmann::json &jsonObj, const std::string &key);

int64_t GetCurrentTime();

void GenerateAdtsHeader(unsigned char* adtsHeader, int packetLen, int profile, int sampleRate, int channels);

template<typename T, typename U>
inline std::shared_ptr<T> ReinterpretCastPointer(const std::shared_ptr<U> &ptr) noexcept
{
    return std::shared_ptr<T>(ptr, reinterpret_cast<T*>(ptr.get()));
}
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AV_TRANSPORT_UTILS_H