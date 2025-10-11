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
namespace {
    constexpr uint16_t TEST_DEV_TYPE_PAD = 0x11;
    constexpr int32_t EVEN_CHECK = 2;
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

void CapabilityInfoManagerOnChangeInsertFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    cJSON *insertJson = cJSON_CreateObject();
    if (insertJson == nullptr) {
        return;
    }
    std::string networkId(reinterpret_cast<const char*>(data), size);
    std::string udId(reinterpret_cast<const char*>(data), size);
    std::string uuId(reinterpret_cast<const char*>(data), size);
    DHContext::GetInstance().AddOnlineDevice(udId, uuId, networkId);
    std::string deviceId = Sha256(uuId);
    cJSON_AddStringToObject(insertJson, DH_ID, "111111");
    cJSON_AddStringToObject(insertJson, DEV_ID, deviceId.c_str());
    cJSON_AddStringToObject(insertJson, DEV_NAME, "dev_name");
    char* cjson = cJSON_PrintUnformatted(insertJson);
    if (cjson == nullptr) {
        cJSON_Delete(insertJson);
        return;
    }
    std::string jsonStr(cjson);
    DistributedKv::Entry insert;
    insert.key = std::string(reinterpret_cast<const char*>(data), size);
    insert.value = jsonStr;

    DistributedKv::Entry update;
    DistributedKv::Entry del;
    std::vector<DistributedKv::Entry> inserts;
    std::vector<DistributedKv::Entry> updates;
    std::vector<DistributedKv::Entry> deleteds;
    inserts.push_back(insert);
    updates.push_back(update);
    deleteds.push_back(del);

    DistributedKv::ChangeNotification changeIn(std::move(inserts), std::move(updates), std::move(deleteds),
        deviceId, true);
    CapabilityInfoManager::GetInstance()->OnChange(changeIn);
    DHContext::GetInstance().RemoveOnlineDeviceIdEntryByNetworkId(networkId);
    cJSON_free(cjson);
    cJSON_Delete(insertJson);
}

void CapabilityInfoManagerOnChangeUpdateFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    cJSON *updateJson = cJSON_CreateObject();
    if (updateJson == nullptr) {
        return;
    }
    std::string networkId(reinterpret_cast<const char*>(data), size);
    std::string udId(reinterpret_cast<const char*>(data), size);
    std::string uuId(reinterpret_cast<const char*>(data), size);
    DHContext::GetInstance().AddOnlineDevice(udId, uuId, networkId);
    std::string deviceId = Sha256(uuId);
    cJSON_AddStringToObject(updateJson, DH_ID, "222222");
    cJSON_AddStringToObject(updateJson, DEV_ID, deviceId.c_str());
    cJSON_AddStringToObject(updateJson, DEV_NAME, "dev_name");
    char* cjson = cJSON_PrintUnformatted(updateJson);
    if (cjson == nullptr) {
        cJSON_Delete(updateJson);
        return;
    }
    std::string jsonStr(cjson);
    DistributedKv::Entry update;
    update.key = std::string(reinterpret_cast<const char*>(data), size);
    update.value = jsonStr;

    DistributedKv::Entry insert;
    DistributedKv::Entry del;
    std::vector<DistributedKv::Entry> inserts;
    std::vector<DistributedKv::Entry> updates;
    std::vector<DistributedKv::Entry> deleteds;
    inserts.push_back(insert);
    updates.push_back(update);
    deleteds.push_back(del);

    DistributedKv::ChangeNotification changeIn(std::move(inserts), std::move(updates), std::move(deleteds),
        deviceId, true);
    CapabilityInfoManager::GetInstance()->OnChange(changeIn);
    DHContext::GetInstance().RemoveOnlineDeviceIdEntryByNetworkId(networkId);
    cJSON_free(cjson);
    cJSON_Delete(updateJson);
}

void CapabilityInfoManagerOnChangeDeleteFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    cJSON *deleteJson = cJSON_CreateObject();
    if (deleteJson == nullptr) {
        return;
    }
    std::string networkId(reinterpret_cast<const char*>(data), size);
    std::string udId(reinterpret_cast<const char*>(data), size);
    std::string uuId(reinterpret_cast<const char*>(data), size);
    DHContext::GetInstance().AddOnlineDevice(udId, uuId, networkId);
    std::string deviceId = Sha256(uuId);
    cJSON_AddStringToObject(deleteJson, DH_ID, "333333");
    cJSON_AddStringToObject(deleteJson, DEV_ID, deviceId.c_str());
    cJSON_AddStringToObject(deleteJson, DEV_NAME, "dev_name");
    char* cjson = cJSON_PrintUnformatted(deleteJson);
    if (cjson == nullptr) {
        cJSON_Delete(deleteJson);
        return;
    }
    std::string jsonStr(cjson);
    DistributedKv::Entry del;
    del.key = std::string(reinterpret_cast<const char*>(data), size);
    del.value = jsonStr;

    DistributedKv::Entry insert;
    DistributedKv::Entry update;
    std::vector<DistributedKv::Entry> inserts;
    std::vector<DistributedKv::Entry> updates;
    std::vector<DistributedKv::Entry> deleteds;
    inserts.push_back(insert);
    updates.push_back(update);
    deleteds.push_back(del);

    DistributedKv::ChangeNotification changeIn(std::move(inserts), std::move(updates), std::move(deleteds),
        deviceId, true);
    CapabilityInfoManager::GetInstance()->OnChange(changeIn);
    DHContext::GetInstance().RemoveOnlineDeviceIdEntryByNetworkId(networkId);
    cJSON_free(cjson);
    cJSON_Delete(deleteJson);
}

void HasCapabilityFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    std::string dhId(reinterpret_cast<const char*>(data), size);
    std::string deviceId(reinterpret_cast<const char*>(data), size);

    CapabilityInfoManager::GetInstance()->HasCapability(deviceId, dhId);
}

void DumpCapabilityInfosFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::vector<CapabilityInfo> capInfos;
    std::string dhId = fdp.ConsumeRandomLengthString();
    std::string deviceId = fdp.ConsumeRandomLengthString();
    std::string deviceName = fdp.ConsumeRandomLengthString();
    std::string dhAttrs = fdp.ConsumeRandomLengthString();
    std::string dhSubtype = fdp.ConsumeRandomLengthString();
    CapabilityInfo capInfo(dhId, deviceId, deviceName, TEST_DEV_TYPE_PAD, DHType::CAMERA, dhAttrs, dhSubtype);
    capInfos.push_back(capInfo);
    CapabilityInfoManager::GetInstance()->DumpCapabilityInfos(capInfos);
}

void CapabilityInfoManagerEventHandlerCtorFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    auto runner = AppExecFwk::EventRunner::Create(true);
    std::shared_ptr<CapabilityInfoManager> mgrPtr;
    if (data[0] % EVEN_CHECK == 0) {
        mgrPtr = nullptr;
    } else {
        mgrPtr = std::make_shared<CapabilityInfoManager>();
    }

    CapabilityInfoManager::CapabilityInfoManagerEventHandler handler(runner, mgrPtr);
    CapabilityInfoManager::GetInstance()->Init();
    CapabilityInfoManager::CapabilityInfoManagerEventHandler handler2(runner, mgrPtr);
    CapabilityInfoManager::GetInstance()->UnInit();
}

void OnChangeFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    std::string uuId(reinterpret_cast<const char*>(data), size);
    std::string deviceId = Sha256(uuId);
    DistributedKv::Entry insert;
    DistributedKv::Entry update;
    DistributedKv::Entry del;
    std::vector<DistributedKv::Entry> inserts;
    std::vector<DistributedKv::Entry> updates;
    std::vector<DistributedKv::Entry> deleteds;
    inserts.push_back(insert);
    updates.push_back(update);
    deleteds.push_back(del);

    DistributedKv::ChangeNotification changeIn(std::move(inserts), std::move(updates), std::move(deleteds),
        deviceId, true);
    CapabilityInfoManager::GetInstance()->OnChange(changeIn);
}

void GetEntriesByKeysFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    std::vector<std::string> keys = {std::string(reinterpret_cast<const char*>(data), size)};
    CapabilityInfoManager::GetInstance()->GetEntriesByKeys(keys);
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
    OHOS::DistributedHardware::GetDataByKeyFuzzTest(data, size);
    OHOS::DistributedHardware::CapabilityInfoManagerOnChangeInsertFuzzTest(data, size);
    OHOS::DistributedHardware::CapabilityInfoManagerOnChangeUpdateFuzzTest(data, size);
    OHOS::DistributedHardware::CapabilityInfoManagerOnChangeDeleteFuzzTest(data, size);
    OHOS::DistributedHardware::HasCapabilityFuzzTest(data, size);
    OHOS::DistributedHardware::DumpCapabilityInfosFuzzTest(data, size);
    OHOS::DistributedHardware::CapabilityInfoManagerEventHandlerCtorFuzzTest(data, size);
    OHOS::DistributedHardware::OnChangeFuzzTest(data, size);
    OHOS::DistributedHardware::GetEntriesByKeysFuzzTest(data, size);
    return 0;
}

