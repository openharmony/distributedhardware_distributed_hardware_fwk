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

#include "resourcemanager_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "capability_info.h"
#include "capability_info_manager.h"
#include "dh_context.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    const uint32_t DH_TYPE_SIZE = 10;
    const DHType dhTypeFuzz[DH_TYPE_SIZE] = {
        DHType::CAMERA, DHType::AUDIO, DHType::DISPLAY, DHType::VIRMODEM_MIC,
        DHType::INPUT, DHType::A2D, DHType::GPS, DHType::HFP, DHType::VIRMODEM_SPEAKER
    };
}

void ResourceManagerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint16_t))) {
        return;
    }

    std::string dhId(reinterpret_cast<const char*>(data), size);
    std::string devId(reinterpret_cast<const char*>(data), size);
    std::string devName(reinterpret_cast<const char*>(data), size);
    uint16_t devType = *(reinterpret_cast<const uint16_t*>(data));
    DHType dhType = dhTypeFuzz[data[0] % DH_TYPE_SIZE];
    std::string dhAttrs(reinterpret_cast<const char*>(data), size);

    std::shared_ptr<CapabilityInfo> capInfo =
        std::make_shared<CapabilityInfo>(dhId, devId, devName, devType, dhType, dhAttrs);
    std::vector<std::shared_ptr<CapabilityInfo>> resInfos;
    resInfos.emplace_back(capInfo);

    CapabilityInfoManager::GetInstance()->AddCapability(resInfos);

    std::shared_ptr<CapabilityInfo> info;
    CapabilityInfoManager::GetInstance()->GetCapability(capInfo->GetDeviceId(), capInfo->GetDHId(), info);
    CapabilityInfoManager::GetInstance()->GetDataByKey(capInfo->GetKey(), info);

    CapabilityInfoMap capMap;
    CapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(capInfo->GetDeviceId(), capMap);
    CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(capInfo->GetKey());
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::ResourceManagerFuzzTest(data, size);
    return 0;
}

