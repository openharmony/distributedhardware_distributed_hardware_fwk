/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_DEVICE_TYPE_H
#define OHOS_DISTRIBUTED_HARDWARE_DEVICE_TYPE_H

#include <string>
#include <unordered_map>

namespace OHOS {
namespace DistributedHardware {
enum class DHType : uint32_t {
    UNKNOWN = 0x0,            // unknown device
    CAMERA = 0x01,            // Camera
    AUDIO = 0x02,             // Mic
    DISPLAY = 0x08,           // Display
    GPS = 0x10,               // GPS
    INPUT = 0x20,             // Key board
    HFP = 0x40,               // HFP External device
    A2D = 0x80,               // A2DP External device
    VIRMODEM_MIC = 0x100,     // Cellular call MIC
    VIRMODEM_SPEAKER = 0x200, // Cellular call Speaker
    MAX_DH = 0x80000000
};

const std::unordered_map<DHType, std::string> DHTypeStrMap = {
    { DHType::CAMERA, "CAMERA" },
    { DHType::AUDIO, "AUDIO" },
    { DHType::DISPLAY, "DISPLAY" },
    { DHType::GPS, "GPS" },
    { DHType::INPUT, "INPUT" },
    { DHType::HFP, "HFP" },
    { DHType::A2D, "A2D" },
    { DHType::VIRMODEM_MIC, "VIRMODEM_MIC" },
    { DHType::VIRMODEM_SPEAKER, "VIRMODEM_SPEAKER" },
};

struct DeviceInfo {
    std::string uuid;
    std::string deviceId;
    std::string deviceName;
    uint16_t deviceType;

    explicit DeviceInfo(std::string uuid, std::string deviceId, std::string deviceName, uint16_t deviceType)
        : uuid(uuid), deviceId(deviceId), deviceName(deviceName), deviceType(deviceType) {}
};

/* The key is DHType, the value is the prefix of DHId */
const std::unordered_map<DHType, std::string> DHTypePrefixMap = {
    {DHType::CAMERA, "Camera"},
    {DHType::DISPLAY, "Screen"},
    {DHType::INPUT, "Input"},
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
