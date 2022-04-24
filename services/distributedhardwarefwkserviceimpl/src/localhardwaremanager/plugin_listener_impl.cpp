/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "plugin_listener_impl.h"

#include "capability_info_manager.h"
#include "dh_context.h"
#include "distributed_hardware_errno.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "PluginListenerImpl"

void PluginListenerImpl::PluginHardware(const std::string &dhId, const std::string &attrs)
{
    DHLOGI("plugin start, dhId: %s", GetAnonyString(dhId).c_str());
    std::vector<std::shared_ptr<CapabilityInfo>> capabilityInfos;
    std::string deviceId = DHContext::GetInstance().GetDeviceInfo().deviceId;
    std::string devName = DHContext::GetInstance().GetDeviceInfo().deviceName;
    uint16_t devType = DHContext::GetInstance().GetDeviceInfo().deviceType;
    std::shared_ptr<CapabilityInfo> dhCapabilityInfo =
        std::make_shared<CapabilityInfo>(dhId, deviceId, devName, devType, dhType_, attrs);
    capabilityInfos.push_back(dhCapabilityInfo);

    CapabilityInfoManager::GetInstance()->AddCapability(capabilityInfos);
    DHLOGI("plugin end, dhId: %s", GetAnonyString(dhId).c_str());
}

void PluginListenerImpl::UnPluginHardware(const std::string &dhId)
{
    DHLOGI("unplugin start, dhId: %s", GetAnonyString(dhId).c_str());
    std::string deviceId = DHContext::GetInstance().GetDeviceInfo().deviceId;
    std::shared_ptr<CapabilityInfo> capability = nullptr;
    auto ret = CapabilityInfoManager::GetInstance()->GetCapability(deviceId, dhId, capability);
    if ((ret != DH_FWK_SUCCESS) || (capability == nullptr)) {
        DHLOGE("GetCapability failed, deviceId =%s, dhId = %s, errCode = %d",
            GetAnonyString(deviceId).c_str(), GetAnonyString(dhId).c_str(), ret);
        return;
    }
    CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(capability->GetKey());
    DHLOGI("unplugin end, dhId: %s", GetAnonyString(dhId).c_str());
}
} // namespace DistributedHardware
} // namespace OHOS
