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

#include "dh_utils_tool.h"

#include <algorithm>
#include <iomanip>
#include <random>
#include <sstream>
#include <sys/time.h>

#include "openssl/sha.h"
#include "softbus_common.h"
#include "softbus_bus_center.h"

#include "constants.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr int32_t MS_ONE_SECOND = 1000;
    constexpr int32_t WORD_WIDTH_8 = 8;
    constexpr int32_t WORD_WIDTH_4 = 4;
    constexpr int32_t WIDTH = 4;
    constexpr unsigned char MASK = 0x0F;
    constexpr int32_t DOUBLE_TIMES = 2;
}

int64_t GetCurrentTime()
{
    struct timeval tv {
        0
    };
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * MS_ONE_SECOND + tv.tv_usec / MS_ONE_SECOND;
}

std::string GetRandomID()
{
    static std::random_device rd;
    static std::uniform_int_distribution<uint64_t> dist(0ULL, 0xFFFFFFFFFFFFFFFFULL);
    uint64_t ab = dist(rd);
    uint64_t cd = dist(rd);
    uint32_t a, b, c, d;
    std::stringstream ss;
    ab = (ab & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
    cd = (cd & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;
    a = (ab >> 32U);
    b = (ab & 0xFFFFFFFFU);
    c = (cd >> 32U);
    d = (cd & 0xFFFFFFFFU);
    ss << std::hex << std::nouppercase << std::setfill('0');
    ss << std::setw(WORD_WIDTH_8) << (a);
    ss << std::setw(WORD_WIDTH_4) << (b >> 16U);
    ss << std::setw(WORD_WIDTH_4) << (b & 0xFFFFU);
    ss << std::setw(WORD_WIDTH_4) << (c >> 16U);
    ss << std::setw(WORD_WIDTH_4) << (c & 0xFFFFU);
    ss << std::setw(WORD_WIDTH_8) << d;

    return ss.str();
}

std::string GetUUIDBySoftBus(const std::string &networkId)
{
    if (networkId.empty()) {
        return "";
    }
    char uuid[UUID_BUF_LEN] = {0};
    auto ret = GetNodeKeyInfo(DH_FWK_PKG_NAME.c_str(), networkId.c_str(), NodeDeviceInfoKey::NODE_KEY_UUID,
        reinterpret_cast<uint8_t *>(uuid), UUID_BUF_LEN);
    return (ret == DH_FWK_SUCCESS) ? std::string(uuid) : "";
}

std::string GetDeviceIdByUUID(const std::string &uuid)
{
    if (uuid.size() == 0 || uuid.size() > MAX_ID_LEN) {
        DHLOGE("uuid is invalid!");
        return "";
    }
    return Sha256(uuid);
}

std::string Sha256(const std::string& in)
{
    unsigned char out[SHA256_DIGEST_LENGTH * DOUBLE_TIMES + 1] = {0};
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, in.data(), in.size());
    SHA256_Final(&out[SHA256_DIGEST_LENGTH], &ctx);
    // here we translate sha256 hash to hexadecimal. each 8-bit char will be presented by two characters([0-9a-f])
    const char* hexCode = "0123456789abcdef";
    for (int32_t i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        unsigned char value = out[SHA256_DIGEST_LENGTH + i];
        // uint8_t is 2 digits in hexadecimal.
        out[i * DOUBLE_TIMES] = hexCode[(value >> WIDTH) & MASK];
        out[i * DOUBLE_TIMES + 1] = hexCode[value & MASK];
    }
    out[SHA256_DIGEST_LENGTH * DOUBLE_TIMES] = 0;
    return reinterpret_cast<char*>(out);
}

DeviceInfo GetLocalDeviceInfo()
{
    DeviceInfo devInfo { "", "", "", 0 };
    auto info = std::make_unique<NodeBasicInfo>();
    auto ret = GetLocalNodeDeviceInfo(DH_FWK_PKG_NAME.c_str(), info.get());
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetLocalNodeDeviceInfo failed, errCode = %{public}d", ret);
        return devInfo;
    }
    devInfo.uuid = GetUUIDBySoftBus(info->networkId);
    devInfo.deviceId = GetDeviceIdByUUID(devInfo.uuid);
    devInfo.deviceName = info->deviceName;
    devInfo.deviceType = info->deviceTypeId;
    return devInfo;
}

bool IsUInt8(const cJSON* jsonObj, const std::string& key)
{
    const cJSON* value = cJSON_GetObjectItem(jsonObj, key.c_str());
    if (value == NULL || !cJSON_IsNumber(value)) {
        return false;
    }
    return (value->valuedouble >= 0 && value->valuedouble <= UINT8_MAX);
}

bool IsUInt16(const cJSON* jsonObj, const std::string& key)
{
    const cJSON* value = cJSON_GetObjectItem(jsonObj, key.c_str());
    if (value == NULL || !cJSON_IsNumber(value)) {
        return false;
    }
    return (value->valuedouble >= 0 && value->valuedouble <= UINT16_MAX);
}

bool IsInt32(const cJSON* jsonObj, const std::string& key)
{
    const cJSON* value = cJSON_GetObjectItem(jsonObj, key.c_str());
    if (value == NULL || !cJSON_IsNumber(value)) {
        return false;
    }
    return (value->valuedouble >= INT32_MIN && value->valuedouble <= INT32_MAX);
}

bool IsUInt32(const cJSON* jsonObj, const std::string& key)
{
    const cJSON* value = cJSON_GetObjectItem(jsonObj, key.c_str());
    if (value == NULL || !cJSON_IsNumber(value)) {
        return false;
    }
    return (value->valuedouble >= 0 && value->valuedouble <= UINT32_MAX);
}

bool IsBool(const cJSON* jsonObj, const std::string& key)
{
    const cJSON* value = cJSON_GetObjectItem(jsonObj, key.c_str());
    return (value != NULL && cJSON_IsBool(value));
}

bool IsString(const cJSON* jsonObj, const std::string& key)
{
    const cJSON* value = cJSON_GetObjectItem(jsonObj, key.c_str());
    if (value == NULL || !cJSON_IsString(value)) {
        return false;
    }
    return (strlen(value->valuestring) > MIN_MESSAGE_LEN && strlen(value->valuestring) <= MAX_MESSAGE_LEN);
}

bool IsArray(const cJSON* jsonObj, const std::string& key)
{
    const cJSON* value = cJSON_GetObjectItem(jsonObj, key.c_str());
    if (value == NULL || !cJSON_IsArray(value)) {
        return false;
    }
    return ((uint32_t)cJSON_GetArraySize(value) > 0 && (uint32_t)cJSON_GetArraySize(value) <= MAX_COMP_SIZE);
}
} // namespace DistributedHardware
} // namespace OHOS
