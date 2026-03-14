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

#include "capabilityinfomanagerthree_fuzzer.h"

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
    constexpr int32_t EVEN_CHECK = 2;
    const uint32_t MIN_DH_TYPE = 0;
    const uint32_t MAX_DH_TYPE = 10;
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

    FuzzedDataProvider fdp(data, size);
    int bufSize = fdp.ConsumeIntegralInRange<int>(MIN_DH_TYPE, MAX_DH_TYPE);

    std::vector<std::string> keys = {std::string(reinterpret_cast<const char*>(data), bufSize)};
    CapabilityInfoManager::GetInstance()->GetEntriesByKeys(keys);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::CapabilityInfoManagerEventHandlerCtorFuzzTest(data, size);
    OHOS::DistributedHardware::OnChangeFuzzTest(data, size);
    OHOS::DistributedHardware::GetEntriesByKeysFuzzTest(data, size);
    return 0;
}

