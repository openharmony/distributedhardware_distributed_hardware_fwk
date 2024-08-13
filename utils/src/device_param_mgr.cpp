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

#include "device_param_mgr.h"

#include <parameter.h>

#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    const int32_t BUF_LENTH = 128;
    const char *SYNC_TYPE_E2E = "1";
    const char *DATA_SYNC_PARAM = "persist.distributed_scene.sys_settings_data_sync";
}
IMPLEMENT_SINGLE_INSTANCE(DeviceParamMgr);
void DeviceParamMgr::QueryDeviceDataSyncMode()
{
    char paramBuf[BUF_LENTH] = {0};
    int32_t ret = GetParameter(DATA_SYNC_PARAM, "", paramBuf, BUF_LENTH);
    DHLOGI("The device paramBuf: %{public}s", paramBuf);
    if (ret > 0 && strncmp(paramBuf, SYNC_TYPE_E2E, strlen(SYNC_TYPE_E2E)) == 0) {
        DHLOGI("Determine the e2e device success");
        isDeviceE2ESync_.store(true);
    }
}

bool DeviceParamMgr::IsDeviceE2ESync()
{
    return isDeviceE2ESync_;
}
}
}