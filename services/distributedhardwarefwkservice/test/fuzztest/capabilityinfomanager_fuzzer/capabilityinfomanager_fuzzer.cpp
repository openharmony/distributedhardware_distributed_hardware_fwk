/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <fuzzer/FuzzedDataProvider.h>

#include "constants.h"
#include "capability_info.h"
#include "capability_utils.h"
#include "capability_info_manager.h"
#include "distributed_hardware_log.h"
#include "dh_context.h"
#include "dh_utils_tool.h"

namespace OHOS {
namespace DistributedHardware {

void TestGetDistributedHardwareCallback::OnSuccess(const std::string &networkId,
    const std::vector<DHDescriptor> &descriptors, EnableStep enableStep)
{
    (void)networkId;
    (void)descriptors;
    (void)enableStep;
}

void TestGetDistributedHardwareCallback::OnError(const std::string &networkId, int32_t error)
{
    (void)networkId;
    (void)error;
}

void CapabilityInfoManagerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= sizeof(DistributedKv::ChangeNotification))) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    DistributedKv::Entry insert;
    DistributedKv::Entry update;
    DistributedKv::Entry del;
    insert.key = fdp.ConsumeRandomLengthString();
    update.key = fdp.ConsumeRandomLengthString();
    del.key = fdp.ConsumeRandomLengthString();
    insert.value = fdp.ConsumeRandomLengthString();
    update.value = fdp.ConsumeRandomLengthString();
    del.value = fdp.ConsumeRandomLengthString();
    std::vector<DistributedKv::Entry> inserts;
    std::vector<DistributedKv::Entry> updates;
    std::vector<DistributedKv::Entry> deleteds;
    inserts.push_back(insert);
    updates.push_back(update);
    deleteds.push_back(del);
    std::string deviceId = fdp.ConsumeRandomLengthString();

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
    CapabilityInfoManager::GetInstance()->Init();
    CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(key);
    CapabilityInfoManager::GetInstance()->UnInit();
}

void RemoveCapabilityInfoInDBFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::string deviceId(reinterpret_cast<const char*>(data), size);
    CapabilityInfoManager::GetInstance()->Init();
    CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoInDB(deviceId);
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
    return 0;
}

