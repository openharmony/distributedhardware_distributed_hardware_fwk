/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "mock_device_param_mgr.h"

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<IDeviceParamMgr> IDeviceParamMgr::ParamMgrInstance_;

std::shared_ptr<IDeviceParamMgr> IDeviceParamMgr::GetOrCreateInstance()
{
    if (!ParamMgrInstance_) {
        ParamMgrInstance_ = std::make_shared<MockIDeviceParamMgr>();
    }
    return ParamMgrInstance_;
}

void IDeviceParamMgr::ReleaseInstance()
{
    ParamMgrInstance_.reset();
    ParamMgrInstance_ = nullptr;
}

bool GetDeviceSyncDataMode()
{
    return IDeviceParamMgr::GetOrCreateInstance()->GetDeviceSyncDataMode();
}
} // namespace DistributedHardware
} // namespace OHOS
