/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "capabilityinfomanager_fuzzer.h"

#include "capability_info_manager.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {

void CapabilityInfoManagerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= sizeof(DistributedKv::ChangeNotification))) {
        return;
    }

    DistributedKv::Entry insert;
    DistributedKv::Entry update;
    DistributedKv::Entry del;
    insert.key = std::string(reinterpret_cast<const char*>(data), size);
    update.key = std::string(reinterpret_cast<const char*>(data), size);
    del.key = std::string(reinterpret_cast<const char*>(data), size);
    insert.value = std::string(reinterpret_cast<const char*>(data), size);
    update.value = std::string(reinterpret_cast<const char*>(data), size);
    del.value = std::string(reinterpret_cast<const char*>(data), size);
    std::vector<DistributedKv::Entry> inserts;
    std::vector<DistributedKv::Entry> updates;
    std::vector<DistributedKv::Entry> deleteds;
    inserts.push_back(insert);
    updates.push_back(update);
    deleteds.push_back(del);
    std::string deviceId(reinterpret_cast<const char*>(data), size);

    DistributedKv::ChangeNotification changeIn(std::move(inserts), std::move(updates), std::move(deleteds),
        deviceId, true);
    CapabilityInfoManager::GetInstance()->OnChange(changeIn);
}

void RemoveCapabilityInfoInMemFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::string deviceId(reinterpret_cast<const char*>(data), size);
    CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoInMem(deviceId);
}

void RemoveCapabilityInfoByKeyFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::string key(reinterpret_cast<const char*>(data), size);
    CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(key);
}

void RemoveCapabilityInfoInDBFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::string deviceId(reinterpret_cast<const char*>(data), size);
    CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoInDB(deviceId);
}

void SyncDeviceInfoFromDBFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::string deviceId(reinterpret_cast<const char*>(data), size);
    CapabilityInfoManager::GetInstance()->Init();
    CapabilityInfoManager::GetInstance()->SyncDeviceInfoFromDB(deviceId);
    CapabilityInfoManager::GetInstance()->SyncRemoteCapabilityInfos();
    CapabilityInfoManager::GetInstance()->UnInit();
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::CapabilityInfoManagerFuzzTest(data, size);
    OHOS::DistributedHardware::RemoveCapabilityInfoInMemFuzzTest(data, size);
    OHOS::DistributedHardware::RemoveCapabilityInfoByKeyFuzzTest(data, size);
    OHOS::DistributedHardware::RemoveCapabilityInfoInDBFuzzTest(data, size);
    OHOS::DistributedHardware::SyncDeviceInfoFromDBFuzzTest(data, size);
    return 0;
}

