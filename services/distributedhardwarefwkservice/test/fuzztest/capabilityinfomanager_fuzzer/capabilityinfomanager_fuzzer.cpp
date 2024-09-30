/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "capability_info.h"
#include "capability_utils.h"
#include "capability_info_manager.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr uint16_t TEST_DEV_TYPE_PAD = 0x11;
}

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
    std::vector<std::shared_ptr<CapabilityInfo>> resInfos;
    CapabilityInfoManager::GetInstance()->AddCapabilityInMem(resInfos);

    std::string dhId1(reinterpret_cast<const char*>(data), size);
    std::string deviceId1(reinterpret_cast<const char*>(data), size);
    std::string deviceName1(reinterpret_cast<const char*>(data), size);
    std::string dhAttrs1(reinterpret_cast<const char*>(data), size);
    std::string dhSubtype1(reinterpret_cast<const char*>(data), size);
    std::shared_ptr<CapabilityInfo> capInfo1 = std::make_shared<CapabilityInfo>(
        dhId1, deviceId1, deviceName1, TEST_DEV_TYPE_PAD, DHType::AUDIO, dhAttrs1, dhSubtype1);

    std::string dhId2(reinterpret_cast<const char*>(data), size);
    std::string deviceId2(reinterpret_cast<const char*>(data), size);
    std::string deviceName2(reinterpret_cast<const char*>(data), size);
    std::string dhAttrs2(reinterpret_cast<const char*>(data), size);
    std::string dhSubtype2(reinterpret_cast<const char*>(data), size);
    std::shared_ptr<CapabilityInfo> capInfo2 = std::make_shared<CapabilityInfo>(
        dhId2, deviceId2, deviceName2, TEST_DEV_TYPE_PAD, DHType::CAMERA, dhAttrs2, dhSubtype2);
    std::vector<std::shared_ptr<CapabilityInfo>> resInfos1 { capInfo1, capInfo2 };
    CapabilityInfoManager::GetInstance()->AddCapabilityInMem(resInfos1);
}

void IsCapabilityMatchFilterFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::shared_ptr<CapabilityInfo> cap = nullptr;
    CapabilityInfoFilter filter = CapabilityInfoFilter::FILTER_DH_ID;
    std::string value(reinterpret_cast<const char*>(data), size);
    CapabilityInfoManager::GetInstance()->IsCapabilityMatchFilter(cap, filter, value);

    std::string dhId(reinterpret_cast<const char*>(data), size);
    std::string deviceId(reinterpret_cast<const char*>(data), size);
    std::string deviceName(reinterpret_cast<const char*>(data), size);
    std::string dhAttrs(reinterpret_cast<const char*>(data), size);
    std::string dhSubtype(reinterpret_cast<const char*>(data), size);
    std::shared_ptr<CapabilityInfo> capInfo = std::make_shared<CapabilityInfo>(
        dhId, deviceId, deviceName, TEST_DEV_TYPE_PAD, DHType::AUDIO, dhAttrs, dhSubtype);
    CapabilityInfoManager::GetInstance()->IsCapabilityMatchFilter(capInfo, filter, value);
    filter = CapabilityInfoFilter::FILTER_DEVICE_ID;
    CapabilityInfoManager::GetInstance()->IsCapabilityMatchFilter(capInfo, filter, value);
    filter = CapabilityInfoFilter::FILTER_DEVICE_NAME;
    CapabilityInfoManager::GetInstance()->IsCapabilityMatchFilter(capInfo, filter, value);
    filter = CapabilityInfoFilter::FILTER_DH_ATTRS;
    CapabilityInfoManager::GetInstance()->IsCapabilityMatchFilter(capInfo, filter, value);
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
    OHOS::DistributedHardware::AddCapabilityInMemFuzzTest(data, size);
    OHOS::DistributedHardware::IsCapabilityMatchFilterFuzzTest(data, size);
    OHOS::DistributedHardware::GetDataByKeyFuzzTest(data, size);
    return 0;
}

