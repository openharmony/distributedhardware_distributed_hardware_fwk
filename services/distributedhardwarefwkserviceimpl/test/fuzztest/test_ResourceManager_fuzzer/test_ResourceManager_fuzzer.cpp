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

#include "test_ResourceManager_fuzzer.h"

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>

#include "capability_info.h"
#include "capability_info_manager.h"
#include "dh_context.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
constexpr uint16_t TEST_SIZE = 2;
constexpr uint16_t STR_LEN = 32;
constexpr uint16_t DHTYPE_SIZE = 5;
const DHType dhTypeFuzz[DHTYPE_SIZE] = {
    DHType::CAMERA, DHType::MIC, DHType::SPEAKER, DHType::DISPLAY, DHType::BUTTON
};

struct CapabilityInfoFuzz {
    char dhId_[STR_LEN];
    char deviceId_[STR_LEN];
    char deviceName_[STR_LEN];
    uint16_t deviceType_;
    uint8_t dhType_;
    char dhAttrs_[STR_LEN];
};

void CreateCapabilityInfo(const uint8_t* data, std::shared_ptr<CapabilityInfo> &capInfo)
{
    const CapabilityInfoFuzz *capInfoFuzz =
        reinterpret_cast<const CapabilityInfoFuzz*>(data);

    std::string dhId(capInfoFuzz->dhId_);
    std::string devId(capInfoFuzz->deviceId_);
    std::string devName(capInfoFuzz->deviceName_);
    uint16_t devType = capInfoFuzz->deviceType_;
    DHType dhType = dhTypeFuzz[capInfoFuzz->dhType_ % DHTYPE_SIZE];
    std::string dhAttrs(capInfoFuzz->dhAttrs_);

    capInfo = std::make_shared<CapabilityInfo>(dhId, devId, devName, devType, dhType, dhAttrs);
}

bool ResourceManagerFuzzTest(const uint8_t* data, size_t size)
{
    if (size > sizeof(CapabilityInfoFuzz) * TEST_SIZE) {
        std::shared_ptr<CapabilityInfo> capInfo1;
        std::shared_ptr<CapabilityInfo> capInfo2;
        std::vector<std::shared_ptr<CapabilityInfo>> resInfos;

        CreateCapabilityInfo(data, capInfo1);
        CreateCapabilityInfo(data + sizeof(CapabilityInfoFuzz), capInfo2);
        resInfos.emplace_back(capInfo1);
        resInfos.emplace_back(capInfo2);
        CapabilityInfoManager::GetInstance()->AddCapability(resInfos);

        std::shared_ptr<CapabilityInfo> info;
        CapabilityInfoManager::GetInstance()->GetCapability(capInfo1->GetDeviceId(), capInfo1->GetDHId(), info);
        CapabilityInfoManager::GetInstance()->GetCapability(capInfo2->GetDeviceId(), capInfo2->GetDHId(), info);
        CapabilityInfoManager::GetInstance()->GetDataByKey(capInfo1->GetKey(), info);
        CapabilityInfoManager::GetInstance()->GetDataByKey(capInfo2->GetKey(), info);

        CapabilityInfoMap capMap;
        CapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(capInfo1->GetDeviceId(), capMap);
        CapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(capInfo1->GetDeviceId(), capMap);

        CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(capInfo1->GetKey());
        CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(capInfo2->GetKey());
        return true;
    } else {
        return false;
    }
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

