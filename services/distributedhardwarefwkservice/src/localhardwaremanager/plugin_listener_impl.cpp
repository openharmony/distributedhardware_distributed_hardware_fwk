/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "anonymous_string.h"
#include "capability_info.h"
#include "capability_info_manager.h"
#include "constants.h"
#include "dh_context.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "publisher.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "PluginListenerImpl"

void PluginListenerImpl::PluginHardware(const std::string &dhId, const std::string &attrs, const std::string &subtype)
{
    if (dhId.size() == 0 || dhId.size() > MAX_ID_LEN || attrs.size() == 0 || attrs.size() > MAX_MESSAGE_LEN) {
        DHLOGE("Param is invalid!");
        return;
    }
    DHLOGI("plugin start, dhId: %{public}s", GetAnonyString(dhId).c_str());
    if (DHContext::GetInstance().IsSleeping()) {
        DHLOGI("System is in sleeping, drop it");
        return;
    }
    std::vector<std::shared_ptr<CapabilityInfo>> capabilityInfos;
    std::string deviceId = DHContext::GetInstance().GetDeviceInfo().deviceId;
    std::string devName = DHContext::GetInstance().GetDeviceInfo().deviceName;
    uint16_t devType = DHContext::GetInstance().GetDeviceInfo().deviceType;
    std::shared_ptr<CapabilityInfo> dhCapabilityInfo =
        std::make_shared<CapabilityInfo>(dhId, deviceId, devName, devType, dhType_, attrs, subtype);
    capabilityInfos.push_back(dhCapabilityInfo);

    CapabilityInfoManager::GetInstance()->AddCapability(capabilityInfos);
    Publisher::GetInstance().PublishMessage(DHTopic::TOPIC_PHY_DEV_PLUGIN, dhId);
    DHLOGI("plugin end, dhId: %{public}s", GetAnonyString(dhId).c_str());
}

void PluginListenerImpl::UnPluginHardware(const std::string &dhId)
{
    if (dhId.size() == 0 || dhId.size() > MAX_ID_LEN) {
        DHLOGE("DhId is invalid!");
        return;
    }
    DHLOGI("unplugin start, dhId: %{public}s", GetAnonyString(dhId).c_str());
    if (DHContext::GetInstance().IsSleeping()) {
        DHLOGI("System is in sleeping, drop it");
        return;
    }
    std::string deviceId = DHContext::GetInstance().GetDeviceInfo().deviceId;
    std::shared_ptr<CapabilityInfo> capability = nullptr;
    auto ret = CapabilityInfoManager::GetInstance()->GetCapability(deviceId, dhId, capability);
    if ((ret != DH_FWK_SUCCESS) || (capability == nullptr)) {
        DHLOGE("GetCapability failed, deviceId =%{public}s, dhId = %{public}s, errCode = %{public}d",
            GetAnonyString(deviceId).c_str(), GetAnonyString(dhId).c_str(), ret);
        return;
    }
    CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(capability->GetKey());
    DHLOGI("unplugin end, dhId: %{public}s", GetAnonyString(dhId).c_str());
}
} // namespace DistributedHardware
} // namespace OHOS
