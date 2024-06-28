/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "metainfomgr_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "meta_info_manager.h"
#include "meta_capability_info.h"

namespace OHOS {
namespace DistributedHardware {
void SyncMetaInfoFromDBFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::string deviceId(reinterpret_cast<const char*>(data), size);
    MetaInfoManager::GetInstance()->Init();
    MetaInfoManager::GetInstance()->SyncMetaInfoFromDB(deviceId);
}

void GetDataByKeyPrefixFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::string keyPrefix(reinterpret_cast<const char*>(data), size);
    MetaCapInfoMap metaCapMap;
    MetaInfoManager::GetInstance()->GetDataByKeyPrefix(keyPrefix, metaCapMap);
}

void RemoveMetaInfoByKeyFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::string key(reinterpret_cast<const char*>(data), size);
    MetaInfoManager::GetInstance()->RemoveMetaInfoByKey(key);
}

void GetMetaCapInfoFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::string deviceId(reinterpret_cast<const char*>(data), size);
    std::string udidHash(reinterpret_cast<const char*>(data), size);
    std::string dhId(reinterpret_cast<const char*>(data), size);
    uint16_t deviceType = 14;
    std::shared_ptr<MetaCapabilityInfo> metaCapPtr = std::make_shared<MetaCapabilityInfo>(
        dhId, deviceId, "devName_test", deviceType, DHType::CAMERA, "attrs_test", "subtype", udidHash, "1.0");
    std::string key = deviceId + "###" + dhId;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[key] = metaCapPtr;
    MetaInfoManager::GetInstance()->GetMetaCapInfo(deviceId, dhId, metaCapPtr);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SyncMetaInfoFromDBFuzzTest(data, size);
    OHOS::DistributedHardware::GetDataByKeyPrefixFuzzTest(data, size);
    OHOS::DistributedHardware::RemoveMetaInfoByKeyFuzzTest(data, size);
    OHOS::DistributedHardware::GetMetaCapInfoFuzzTest(data, size);
    return 0;
}

