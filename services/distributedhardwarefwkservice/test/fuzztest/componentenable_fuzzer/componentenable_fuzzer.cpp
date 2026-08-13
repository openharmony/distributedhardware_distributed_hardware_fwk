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

#include "componentenable_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "component_enable.h"
#include "distributed_hardware_errno.h"

namespace OHOS {
namespace DistributedHardware {

int32_t FuzzDistributedHardwareSource::InitSource(const std::string& networkId)
{
    (void)networkId;
    return DH_FWK_SUCCESS;
}

int32_t FuzzDistributedHardwareSource::ReleaseSource()
{
    return DH_FWK_SUCCESS;
}

int32_t FuzzDistributedHardwareSource::RegisterDistributedHardware(
    const std::string& networkId, const std::string& dhId, const EnableParam& param,
    std::shared_ptr<RegisterCallback> callback)
{
    (void)networkId;
    (void)dhId;
    (void)param;
    (void)callback;
    return DH_FWK_SUCCESS;
}

int32_t FuzzDistributedHardwareSource::UnregisterDistributedHardware(
    const std::string& networkId, const std::string& dhId, std::shared_ptr<UnregisterCallback> callback)
{
    (void)networkId;
    (void)dhId;
    (void)callback;
    return DH_FWK_SUCCESS;
}

int32_t FuzzDistributedHardwareSource::ConfigDistributedHardware(
    const std::string& networkId, const std::string& dhId, const std::string& key, const std::string& value)
{
    (void)networkId;
    (void)dhId;
    (void)key;
    (void)value;
    return DH_FWK_SUCCESS;
}

void FuzzDistributedHardwareSource::RegisterDistributedHardwareStateListener(
    std::shared_ptr<DistributedHardwareStateListener> listener)
{
    (void)listener;
}

void FuzzDistributedHardwareSource::RegisterDataSyncTriggerListener(std::shared_ptr<DataSyncTriggerListener> listener)
{
    (void)listener;
}

void ComponentEnableFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    auto compEnable = std::make_shared<ComponentEnable>();
    FuzzedDataProvider fdp(data, size);
    int32_t status = fdp.ConsumeIntegral<int32_t>();
    std::string uuid = fdp.ConsumeRandomLengthString();
    std::string dhId = fdp.ConsumeRandomLengthString();
    std::string enableData = fdp.ConsumeRandomLengthString();
    compEnable->OnRegisterResult(uuid, dhId, status, enableData);
}

void EnableFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    auto compEnable = std::make_shared<ComponentEnable>();
    FuzzedDataProvider fdp(data, size);
    std::string networkId = fdp.ConsumeRandomLengthString();
    std::string dhId = fdp.ConsumeRandomLengthString();
    EnableParam param;
    param.sourceVersion = fdp.ConsumeRandomLengthString();
    param.sourceAttrs = fdp.ConsumeRandomLengthString();
    param.sinkVersion = fdp.ConsumeRandomLengthString();
    param.sinkAttrs = fdp.ConsumeRandomLengthString();
    param.subtype = fdp.ConsumeRandomLengthString();
    auto handler = std::make_shared<FuzzDistributedHardwareSource>();
    compEnable->Enable(networkId, dhId, param, handler.get());
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::ComponentEnableFuzzTest(data, size);
    OHOS::DistributedHardware::EnableFuzzTest(data, size);
    return 0;
}

