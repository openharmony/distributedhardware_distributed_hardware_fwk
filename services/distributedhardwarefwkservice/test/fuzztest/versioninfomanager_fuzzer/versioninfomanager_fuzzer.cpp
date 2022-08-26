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

#include "versioninfomanager_fuzzer.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unistd.h>

#include "version_info_manager.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_manager_factory.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr uint32_t SLEEP_TIME_US = 10 * 1000;
    const uint32_t DH_TYPE_SIZE = 10;
    const DHType dhTypeFuzz[DH_TYPE_SIZE] = {
        DHType::CAMERA, DHType::AUDIO, DHType::DISPLAY, DHType::VIRMODEM_MIC,
        DHType::INPUT, DHType::A2D, DHType::GPS, DHType::HFP, DHType::VIRMODEM_SPEAKER
    };
}

void VersioninfoManagerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    VersionInfo versionInfo;
    versionInfo.deviceId = std::string(reinterpret_cast<const char*>(data), size);
    versionInfo.dhVersion = std::string(reinterpret_cast<const char*>(data), size);

    CompVersion compVer;
    compVer.dhType = dhTypeFuzz[data[0] % DH_TYPE_SIZE];
    compVer.name = std::string(reinterpret_cast<const char*>(data), size);
    compVer.handlerVersion = std::string(reinterpret_cast<const char*>(data), size);
    compVer.sourceVersion = std::string(reinterpret_cast<const char*>(data), size);
    compVer.sinkVersion = std::string(reinterpret_cast<const char*>(data), size);
    versionInfo.compVersions.insert(std::pair<DHType, CompVersion>(compVer.dhType, compVer));

    VersionInfo info;
    VersionInfoManager::GetInstance()->AddVersion(versionInfo);
    VersionInfoManager::GetInstance()->GetVersionInfoByDeviceId(versionInfo.deviceId, info);
    VersionInfoManager::GetInstance()->SyncVersionInfoFromDB(versionInfo.deviceId);
    VersionInfoManager::GetInstance()->RemoveVersionInfoByDeviceId(versionInfo.deviceId);

    usleep(SLEEP_TIME_US);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::VersioninfoManagerFuzzTest(data, size);
    return 0;
}

