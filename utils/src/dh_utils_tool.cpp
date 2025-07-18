/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <sys/time.h>
#include <vector>
#include <zlib.h>

#include "openssl/sha.h"
#include "parameter.h"

#include "device_manager.h"
#include "dm_device_info.h"

#include "constants.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr uint32_t MAX_MESSAGE_LEN = 40 * 1024 * 1024;
    constexpr uint32_t MIN_MESSAGE_LEN = 0;
    constexpr uint32_t MAX_ID_LEN = 256;
    constexpr uint32_t MAX_ARR_SIZE = 10000;
    constexpr uint32_t MAX_JSON_SIZE = 40 * 1024 * 1024;
    constexpr uint32_t MAX_HASH_SIZE = 64;
    constexpr uint32_t MAX_KEY_SIZE = 256;
    constexpr int32_t MS_ONE_SECOND = 1000;
    constexpr int32_t WORD_WIDTH_8 = 8;
    constexpr int32_t WORD_WIDTH_4 = 4;
    constexpr int32_t WIDTH = 4;
    constexpr unsigned char MASK = 0x0F;
    constexpr int32_t DOUBLE_TIMES = 2;
    constexpr int32_t COMPRESS_SLICE_SIZE = 1024;
    constexpr const char *PARAM_KEY_OS_TYPE = "OS_TYPE";
}

uint64_t GetCurrentTime()
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
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
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

std::string GetUUIDByDm(const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return "";
    }
    std::string uuid = "";
    auto ret = DeviceManager::GetInstance().GetUuidByNetworkId(DH_FWK_PKG_NAME, networkId, uuid);
    return (ret == DH_FWK_SUCCESS) ? uuid : "";
}

std::string GetUDIDByDm(const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return "";
    }
    std::string udid = "";
    auto ret = DeviceManager::GetInstance().GetUdidByNetworkId(DH_FWK_PKG_NAME, networkId, udid);
    return (ret == DH_FWK_SUCCESS) ? udid : "";
}

std::string GetDeviceIdByUUID(const std::string &uuid)
{
    if (!IsIdLengthValid(uuid)) {
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
    return std::string(reinterpret_cast<char*>(out));
}

DeviceInfo GetLocalDeviceInfo()
{
    DeviceInfo devInfo { "", "", "", "", "", "", 0 };
    DmDeviceInfo info;
    auto ret = DeviceManager::GetInstance().GetLocalDeviceInfo(DH_FWK_PKG_NAME, info);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetLocalNodeDeviceInfo failed, errCode = %{public}d", ret);
        return devInfo;
    }
    devInfo.networkId = info.networkId;
    devInfo.uuid = GetUUIDByDm(info.networkId);
    devInfo.deviceId = GetDeviceIdByUUID(devInfo.uuid);
    devInfo.udid = GetUDIDByDm(info.networkId);
    devInfo.udidHash = Sha256(devInfo.udid);
    devInfo.deviceName = info.deviceName;
    devInfo.deviceType = info.deviceTypeId;
    return devInfo;
}

std::string GetLocalNetworkId()
{
    DmDeviceInfo info;
    auto ret = DeviceManager::GetInstance().GetLocalDeviceInfo(DH_FWK_PKG_NAME, info);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetLocalNodeDeviceInfo failed, errCode = %{public}d", ret);
        return "";
    }
    return info.networkId;
}

std::string GetLocalUdid()
{
    DmDeviceInfo info;
    auto ret = DeviceManager::GetInstance().GetLocalDeviceInfo(DH_FWK_PKG_NAME, info);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetLocalNodeDeviceInfo failed, errCode = %{public}d", ret);
        return "";
    }
    return GetUDIDByDm(info.networkId);
}

bool IsUInt8(const cJSON* jsonObj)
{
    if (jsonObj == NULL || !cJSON_IsNumber(jsonObj)) {
        return false;
    }
    return (jsonObj->valueint >= 0 && static_cast<uint8_t>(jsonObj->valueint) <= UINT8_MAX);
}

bool IsUInt16(const cJSON* jsonObj)
{
    if (jsonObj == NULL || !cJSON_IsNumber(jsonObj)) {
        return false;
    }
    return (jsonObj->valueint >= 0 && static_cast<uint16_t>(jsonObj->valueint) <= UINT16_MAX);
}

bool IsInt32(const cJSON* jsonObj)
{
    if (jsonObj == NULL || !cJSON_IsNumber(jsonObj)) {
        return false;
    }
    return (jsonObj->valueint >= INT32_MIN && jsonObj->valueint <= INT32_MAX);
}

bool IsUInt32(const cJSON* jsonObj)
{
    if (jsonObj == NULL || !cJSON_IsNumber(jsonObj)) {
        return false;
    }
    return (jsonObj->valueint >= 0 && static_cast<uint32_t>(jsonObj->valueint) <= UINT32_MAX);
}

bool IsBool(const cJSON* jsonObj)
{
    return (jsonObj != NULL && cJSON_IsBool(jsonObj));
}

bool IsString(const cJSON* jsonObj)
{
    if (jsonObj == NULL || !cJSON_IsString(jsonObj)) {
        return false;
    }
    return (strlen(jsonObj->valuestring) > MIN_MESSAGE_LEN && strlen(jsonObj->valuestring) <= MAX_MESSAGE_LEN);
}

bool IsArray(const cJSON* jsonObj)
{
    if (jsonObj == NULL || !cJSON_IsArray(jsonObj)) {
        return false;
    }
    return ((uint32_t)cJSON_GetArraySize(jsonObj) >= 0 && (uint32_t)cJSON_GetArraySize(jsonObj) <= MAX_ARR_SIZE);
}

std::string Compress(const std::string& data)
{
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    deflateInit(&strm, Z_DEFAULT_COMPRESSION);
 
    strm.next_in = reinterpret_cast<Bytef*>(const_cast<char *>(data.data()));
    strm.avail_in = data.size();
    std::string out;
    std::vector<Bytef> temp_out(COMPRESS_SLICE_SIZE, 0);
 
    do {
        strm.next_out = temp_out.data();
        strm.avail_out = COMPRESS_SLICE_SIZE;
        deflate(&strm, Z_FINISH);
        out.append(reinterpret_cast<char*>(temp_out.data()), COMPRESS_SLICE_SIZE - strm.avail_out);
    } while (strm.avail_out == 0);
 
    deflateEnd(&strm);
    return out;
}
 
std::string Decompress(const std::string& data)
{
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    inflateInit(&strm);
 
    strm.next_in = reinterpret_cast<Bytef*>(const_cast<char *>(data.data()));
    strm.avail_in = data.size();
    std::string out;
    std::vector<Bytef> temp_out(COMPRESS_SLICE_SIZE, 0);
 
    do {
        strm.next_out = temp_out.data();
        strm.avail_out = COMPRESS_SLICE_SIZE;
        inflate(&strm, Z_NO_FLUSH);
        out.append(reinterpret_cast<char*>(temp_out.data()), COMPRESS_SLICE_SIZE - strm.avail_out);
    } while (strm.avail_out == 0);
 
    inflateEnd(&strm);
    return out;
}

bool GetSysPara(const char *key, bool &value)
{
    if (key == nullptr) {
        DHLOGE("GetSysPara: key is nullptr");
        return false;
    }
    char paraValue[20] = {0}; // 20 for system parameter
    auto res = GetParameter(key, "false", paraValue, sizeof(paraValue));
    if (res <= 0) {
        DHLOGD("GetSysPara fail, key:%{public}s res:%{public}d", key, res);
        return false;
    }
    DHLOGI("GetSysPara: key:%{public}s value:%{public}s", key, paraValue);
    std::stringstream valueStr;
    valueStr << paraValue;
    valueStr >> std::boolalpha >> value;
    return true;
}

bool IsIdLengthValid(const std::string &inputID)
{
    if (inputID.empty() || inputID.length() > MAX_ID_LEN) {
        DHLOGE("On parameter length error, maybe empty or beyond MAX_ID_LEN!");
        return false;
    }
    return true;
}

bool IsMessageLengthValid(const std::string &inputMessage)
{
    if (inputMessage.empty() || inputMessage.length() > MAX_MESSAGE_LEN) {
        DHLOGE("On parameter error, maybe empty or beyond MAX_MESSAGE_LEN!");
        return false;
    }
    return true;
}

bool IsJsonLengthValid(const std::string &inputJsonStr)
{
    if (inputJsonStr.empty() || inputJsonStr.length() > MAX_JSON_SIZE) {
        DHLOGE("On parameter error, maybe empty or beyond MAX_JSON_SIZE");
        return false;
    }
    return true;
}

bool IsArrayLengthValid(const std::vector<std::string> &inputArray)
{
    if (inputArray.empty() || inputArray.size() > MAX_ARR_SIZE) {
        DHLOGE("On parameter error, maybe empty or beyond MAX_ARR_SIZE");
        return false;
    }
    return true;
}

bool IsKeySizeValid(const std::string &inputKey)
{
    if (inputKey.empty() || inputKey.length() > MAX_KEY_SIZE) {
        DHLOGE("On parameter error, maybe empty or beyond MAX_KEY_SIZE");
        return false;
    }
    return true;
}

bool IsHashSizeValid(const std::string &inputHashValue)
{
    if (inputHashValue.empty() || inputHashValue.length() > MAX_HASH_SIZE) {
        DHLOGE("On parameter error, maybe empty or beyond MAX_HASH_SIZE");
        return false;
    }
    return true;
}

int32_t GetDeviceSystemType(const std::string &extraData)
{
    cJSON *jsonObj = cJSON_Parse(extraData.c_str());
    if (jsonObj == NULL) {
        DHLOGE("jsonStr parse failed");
        return ERR_DH_FWK_INVALID_OSTYPE;
    }
    cJSON *paramKey = cJSON_GetObjectItem(jsonObj, PARAM_KEY_OS_TYPE);
    if (paramKey == NULL || !cJSON_IsNumber(paramKey)) {
        DHLOGE("paramKey is null or paramKey is invaild type!");
        cJSON_Delete(jsonObj);
        return ERR_DH_FWK_INVALID_OSTYPE;
    }
    int32_t osType = paramKey->valueint;
    cJSON_Delete(jsonObj);
    return osType;
}
} // namespace DistributedHardware
} // namespace OHOS
