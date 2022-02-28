/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <iomanip>
#include <random>
#include <sstream>
#include <sys/time.h>

#include "softbus_bus_center.h"

#include "constants.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
constexpr int32_t MS_ONE_SECOND = 1000;
constexpr int32_t WORD_WIDTH_8 = 8;
constexpr int32_t WORD_WIDTH_4 = 4;

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

std::string GetUUIDByNetworkId(const std::string &networkId)
{
    if (networkId.empty()) {
        return "";
    }
    char uuid[UUID_BUF_LEN] = {0};
    auto ret = GetNodeKeyInfo(DH_FWK_PKG_NAME.c_str(), networkId.c_str(), NodeDeviceInfoKey::NODE_KEY_UUID,
        reinterpret_cast<uint8_t *>(uuid), UUID_BUF_LEN);
    return (ret == DH_FWK_SUCCESS) ? std::string(uuid) : "";
}

DeviceInfo GetLocalDeviceInfo()
{
    DeviceInfo devInfo { "", "", 0 };
    auto info = std::make_unique<NodeBasicInfo>();
    auto ret = GetLocalNodeDeviceInfo(DH_FWK_PKG_NAME.c_str(), info.get());
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetLocalNodeDeviceInfo failed, errCode = %d", ret);
        return devInfo;
    }
    devInfo.deviceId = GetUUIDByNetworkId(info->networkId);
    devInfo.deviceName = info->deviceName;
    devInfo.deviceType = info->deviceTypeId;
    return devInfo;
}
}
}
