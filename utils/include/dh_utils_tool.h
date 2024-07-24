/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_DHUTILS_TOOL_H
#define OHOS_DISTRIBUTED_HARDWARE_DHUTILS_TOOL_H

#include <cstdint>

#include "cJSON.h"

#include "device_type.h"

namespace OHOS {
namespace DistributedHardware {
/**
 * return current time in millisecond.
 */
int64_t GetCurrentTime();

/**
 * return a random string id.
 */
std::string GetRandomID();

std::string GetUUIDByDm(const std::string &networkId);

std::string GetUDIDByDm(const std::string &networkId);

DeviceInfo GetLocalDeviceInfo();

std::string GetLocalNetworkId();

/* Convert uuid to deviceId by sha256 encode */
std::string GetDeviceIdByUUID(const std::string &uuid);

std::string Sha256(const std::string& string);

bool IsUInt8(const cJSON* jsonObj, const std::string& key);

bool IsUInt16(const cJSON* jsonObj, const std::string& key);

bool IsInt32(const cJSON* jsonObj, const std::string& key);

bool IsUInt32(const cJSON* jsonObj, const std::string& key);

bool IsBool(const cJSON* jsonObj, const std::string& key);

bool IsString(const cJSON* jsonObj, const std::string& key);

bool IsArray(const cJSON* jsonObj, const std::string& key);

std::string Compress(const std::string& data);
std::string Decompress(const std::string& data);

bool GetSysPara(const char *key, bool &value);

bool IsIdLengthValid(const std::string &input);

bool IsMessageLengthValid(const std::string &input);

bool IsJsonLengthValid(const std::string &jsonStr);

bool IsArrayLengthValid(const std::vector<std::string> &array);

bool IsKeySizeValid(const std::string &key);

bool IsHashSizeValid(const std::string &hashValue);
} // namespace DistributedHardware
} // namespace OHOS
#endif
