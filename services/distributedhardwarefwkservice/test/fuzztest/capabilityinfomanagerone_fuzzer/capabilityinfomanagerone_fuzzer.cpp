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

#include "capabilityinfomanagerone_fuzzer.h"

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
namespace {
    constexpr uint16_t TEST_DEV_TYPE_PAD = 0x11;
}

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

void AddCapabilityInMemFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::vector<std::shared_ptr<CapabilityInfo>> resInfos;
    CapabilityInfoManager::GetInstance()->AddCapabilityInMem(resInfos);

    std::string dhId1 = fdp.ConsumeRandomLengthString();
    std::string deviceId1 = fdp.ConsumeRandomLengthString();
    std::string deviceName1 = fdp.ConsumeRandomLengthString();
    std::string dhAttrs1 = fdp.ConsumeRandomLengthString();
    std::string dhSubtype1 = fdp.ConsumeRandomLengthString();
    std::shared_ptr<CapabilityInfo> capInfo1 = std::make_shared<CapabilityInfo>(
        dhId1, deviceId1, deviceName1, TEST_DEV_TYPE_PAD, DHType::AUDIO, dhAttrs1, dhSubtype1);

    std::string dhId2 = fdp.ConsumeRandomLengthString();
    std::string deviceId2 = fdp.ConsumeRandomLengthString();
    std::string deviceName2 = fdp.ConsumeRandomLengthString();
    std::string dhAttrs2 = fdp.ConsumeRandomLengthString();
    std::string dhSubtype2 = fdp.ConsumeRandomLengthString();
    std::shared_ptr<CapabilityInfo> capInfo2 = std::make_shared<CapabilityInfo>(
        dhId2, deviceId2, deviceName2, TEST_DEV_TYPE_PAD, DHType::CAMERA, dhAttrs2, dhSubtype2);
    std::vector<std::shared_ptr<CapabilityInfo>> resInfos1 { capInfo1, capInfo2 };
    CapabilityInfoManager::GetInstance()->AddCapabilityInMem(resInfos1);
}

void GetDataByKeyFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::string key(reinterpret_cast<const char*>(data), size);
    std::shared_ptr<CapabilityInfo> capInfoPtr;
    CapabilityInfoManager::GetInstance()->GetDataByKey(key, capInfoPtr);

    CapabilityInfoManager::GetInstance()->Init();
    CapabilityInfoManager::GetInstance()->GetDataByKey(key, capInfoPtr);
    CapabilityInfoManager::GetInstance()->UnInit();
}

void HasCapabilityFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    std::string dhId = fdp.ConsumeRandomLengthString();
    std::string deviceId = fdp.ConsumeRandomLengthString();

    CapabilityInfoManager::GetInstance()->HasCapability(deviceId, dhId);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SyncDeviceInfoFromDBFuzzTest(data, size);
    OHOS::DistributedHardware::AddCapabilityInMemFuzzTest(data, size);
    OHOS::DistributedHardware::GetDataByKeyFuzzTest(data, size);
    OHOS::DistributedHardware::HasCapabilityFuzzTest(data, size);
    return 0;
}

