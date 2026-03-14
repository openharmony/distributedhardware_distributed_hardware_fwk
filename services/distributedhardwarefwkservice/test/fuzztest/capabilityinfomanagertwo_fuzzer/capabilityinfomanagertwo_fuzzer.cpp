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

#include "capabilityinfomanagertwo_fuzzer.h"

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

void CapabilityInfoManagerOnChangeInsertFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    cJSON *insertJson = cJSON_CreateObject();
    if (insertJson == nullptr) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string networkId = fdp.ConsumeRandomLengthString();
    std::string udId = fdp.ConsumeRandomLengthString();
    std::string uuId = fdp.ConsumeRandomLengthString();
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
    insert.key = fdp.ConsumeRandomLengthString();
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
    FuzzedDataProvider fdp(data, size);
    std::string networkId = fdp.ConsumeRandomLengthString();
    std::string udId = fdp.ConsumeRandomLengthString();
    std::string uuId = fdp.ConsumeRandomLengthString();
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
    update.key = fdp.ConsumeRandomLengthString();
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
    FuzzedDataProvider fdp(data, size);
    std::string networkId = fdp.ConsumeRandomLengthString();
    std::string udId = fdp.ConsumeRandomLengthString();
    std::string uuId = fdp.ConsumeRandomLengthString();
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
    del.key = fdp.ConsumeRandomLengthString();
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
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::CapabilityInfoManagerOnChangeInsertFuzzTest(data, size);
    OHOS::DistributedHardware::CapabilityInfoManagerOnChangeUpdateFuzzTest(data, size);
    OHOS::DistributedHardware::CapabilityInfoManagerOnChangeDeleteFuzzTest(data, size);
    OHOS::DistributedHardware::DumpCapabilityInfosFuzzTest(data, size);
    return 0;
}

