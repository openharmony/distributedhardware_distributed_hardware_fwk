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

#include "access_listener_service.h"
#include "component_manager.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "AccessListenerService"

void AccessListenerService::OnRequestHardwareAccess(const std::string &requestId, AuthDeviceInfo info,
    const DHType dhType, const std::string &pkgName)
{
    sptr<IAuthorizationResultCallback> callback = ComponentManager::GetInstance().GetAccessListener(dhType, pkgName);
    if (callback == nullptr) {
        DHLOGE("Request Hardware Access failed!");
        return;
    }
    callback->OnAuthorizationResult(info.networkId, requestId);
    return;
}
} // namespace DistributedHardware
} // namespace OHOS