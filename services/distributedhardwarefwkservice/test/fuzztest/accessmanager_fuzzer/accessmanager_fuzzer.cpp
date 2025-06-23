/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "accessmanager_fuzzer.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <securec.h>
#include <string>
#include <unistd.h>

#include "device_manager.h"

#include "access_manager.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_manager_factory.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr uint32_t SLEEP_TIME_US = 10 * 1000;
}

void OnDeviceReadyFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size > DM_MAX_DEVICE_ID_LEN)) {
        return;
    }

    AccessManager::GetInstance()->Init();
    DmDeviceInfo deviceInfo;
    int32_t ret = memcpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, (reinterpret_cast<const char *>(data)), size);
    if (ret != EOK) {
        return;
    }
    AccessManager::GetInstance()->OnDeviceReady(deviceInfo);

    usleep(SLEEP_TIME_US);
}

void OnDeviceOfflineFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size > DM_MAX_DEVICE_ID_LEN)) {
        return;
    }

    AccessManager::GetInstance()->Init();
    DmDeviceInfo deviceInfo;
    int32_t ret = memcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, (reinterpret_cast<const char *>(data)), size);
    if (ret != EOK) {
        return;
    }
    AccessManager::GetInstance()->OnDeviceOffline(deviceInfo);
    usleep(SLEEP_TIME_US);
}

void OnDeviceChangedFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size > DM_MAX_DEVICE_ID_LEN)) {
        return;
    }

    AccessManager::GetInstance()->Init();
    DmDeviceInfo deviceInfo;
    int32_t ret = memcpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, (reinterpret_cast<const char *>(data)), size);
    if (ret != EOK) {
        return;
    }
    AccessManager::GetInstance()->OnDeviceChanged(deviceInfo);
    usleep(SLEEP_TIME_US);
}

void UnInitFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size > DM_MAX_DEVICE_ID_LEN)) {
        return;
    }

    AccessManager::GetInstance()->UnInit();
    usleep(SLEEP_TIME_US);
}

void UnInitDeviceManagerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size > DM_MAX_DEVICE_ID_LEN)) {
        return;
    }

    AccessManager::GetInstance()->UnInitDeviceManager();
    usleep(SLEEP_TIME_US);
}

void UnRegisterDevStateCallbackFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size > DM_MAX_DEVICE_ID_LEN)) {
        return;
    }

    AccessManager::GetInstance()->UnRegisterDevStateCallback();
    usleep(SLEEP_TIME_US);
}

void OnRemoteDiedFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size > DM_MAX_DEVICE_ID_LEN)) {
        return;
    }

    AccessManager::GetInstance()->OnRemoteDied();
    usleep(SLEEP_TIME_US);
}

void CheckTrustedDeviceOnlineFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size > DM_MAX_DEVICE_ID_LEN)) {
        return;
    }

    AccessManager::GetInstance()->CheckTrustedDeviceOnline();
    usleep(SLEEP_TIME_US);
}

void DumpFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size > DM_MAX_DEVICE_ID_LEN)) {
        return;
    }

    std::vector<std::string> argsStr = {reinterpret_cast<const char *>(data)};
    std::string result = {reinterpret_cast<const char *>(data)};
    AccessManager::GetInstance()->Dump(argsStr, result);
    usleep(SLEEP_TIME_US);
}

void OnDeviceOnlineFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size > DM_MAX_DEVICE_ID_LEN)) {
        return;
    }

    AccessManager::GetInstance()->Init();
    DmDeviceInfo deviceInfo;
    int32_t ret = memcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, (reinterpret_cast<const char *>(data)), size);
    if (ret != EOK) {
        return;
    }
    AccessManager::GetInstance()->OnDeviceOnline(deviceInfo);
    usleep(SLEEP_TIME_US);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::OnDeviceReadyFuzzTest(data, size);
    OHOS::DistributedHardware::OnDeviceOnlineFuzzTest(data, size);
    OHOS::DistributedHardware::OnDeviceChangedFuzzTest(data, size);
    OHOS::DistributedHardware::UnInitFuzzTest(data, size);
    OHOS::DistributedHardware::UnInitDeviceManagerFuzzTest(data, size);
    OHOS::DistributedHardware::UnRegisterDevStateCallbackFuzzTest(data, size);
    OHOS::DistributedHardware::OnRemoteDiedFuzzTest(data, size);
    OHOS::DistributedHardware::CheckTrustedDeviceOnlineFuzzTest(data, size);
    OHOS::DistributedHardware::DumpFuzzTest(data, size);
    OHOS::DistributedHardware::OnDeviceOfflineFuzzTest(data, size);
    return 0;
}

