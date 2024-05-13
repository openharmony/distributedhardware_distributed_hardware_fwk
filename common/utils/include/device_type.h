/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
    SCREEN = 0x08,            // Display
    GPS = 0x10,               // GPS
    INPUT = 0x20,             // Key board
    HFP = 0x40,               // HFP External device
    A2D = 0x80,               // A2DP External device
    VIRMODEM_AUDIO = 0x100,     // Cellular call AUDIO
    MAX_DH = 0x80000000
};

enum class DHSubtype : uint32_t {
    CAMERA = 0x01,            // Camera
    SCREEN = 0x08,            // Display
    INPUT = 0x20,             // Key board
    AUDIO_MIC = 0x400,        // AUDIO Mic
    AUDIO_SPEAKER = 0x800     // AUDIO Speaker
};

const std::unordered_map<DHType, std::string> DHTypeStrMap = {
    { DHType::CAMERA, "CAMERA" },
    { DHType::AUDIO, "AUDIO" },
    { DHType::SCREEN, "SCREEN" },
    { DHType::GPS, "GPS" },
    { DHType::INPUT, "INPUT" },
    { DHType::HFP, "HFP" },
    { DHType::A2D, "A2D" },
    { DHType::VIRMODEM_AUDIO, "VIRMODEM_AUDIO" },
};

struct DeviceInfo {
    std::string networkId;
    std::string uuid;
    std::string deviceId;
    std::string deviceName;
    uint16_t deviceType;

    explicit DeviceInfo(std::string networkId, std::string uuid, std::string deviceId, std::string deviceName,
        uint16_t deviceType) : networkId(networkId), uuid(uuid), deviceId(deviceId), deviceName(deviceName),
        deviceType(deviceType) {}
};

/* The key is DHType, the value is the prefix of DHId */
const std::unordered_map<DHType, std::string> DHTypePrefixMap = {
    {DHType::CAMERA, "Camera"},
    {DHType::SCREEN, "Screen"},
    {DHType::INPUT, "Input"},
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
