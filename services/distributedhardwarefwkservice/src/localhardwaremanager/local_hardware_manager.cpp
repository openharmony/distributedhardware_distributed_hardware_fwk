/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "local_hardware_manager.h"

#include <unistd.h>

#include "capability_info_manager.h"
#include "component_loader.h"
#include "constants.h"
#include "device_type.h"
#include "dh_context.h"
#include "dh_utils_hitrace.h"
#include "distributed_hardware_errno.h"
#include "plugin_listener_impl.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr int32_t QUERY_INTERVAL_TIME = 1000 * 1000; // 1s
    constexpr int32_t QUERY_RETRY_MAX_TIMES = 30;
}
#undef DH_LOG_TAG
#define DH_LOG_TAG "LocalHardwareManager"

IMPLEMENT_SINGLE_INSTANCE(LocalHardwareManager)

LocalHardwareManager::LocalHardwareManager() {}
LocalHardwareManager::~LocalHardwareManager() {}

void LocalHardwareManager::Init()
{
    DHLOGI("start");
    std::vector<DHType> allCompTypes = ComponentLoader::GetInstance().GetAllCompTypes();
    for (auto dhType : allCompTypes) {
        IHardwareHandler *hardwareHandler = nullptr;
        int32_t status = ComponentLoader::GetInstance().GetHardwareHandler(dhType, hardwareHandler);
        if (status != DH_FWK_SUCCESS || hardwareHandler == nullptr) {
            DHLOGE("GetHardwareHandler %#X failed", dhType);
            continue;
        }
        if (hardwareHandler->Initialize() != DH_FWK_SUCCESS) {
            DHLOGE("Initialize %#X failed", dhType);
            continue;
        }

        DHQueryTraceStart(dhType);
        QueryLocalHardware(dhType, hardwareHandler);
        DHTraceEnd();
        if (!hardwareHandler->IsSupportPlugin()) {
            DHLOGI("hardwareHandler is not support hot swap plugin, release!");
            ComponentLoader::GetInstance().ReleaseHardwareHandler(dhType);
            hardwareHandler = nullptr;
        } else {
            compToolFuncsMap_[dhType] = hardwareHandler;
            std::shared_ptr<PluginListener> listener = std::make_shared<PluginListenerImpl>(dhType);
            pluginListenerMap_[dhType] = listener;
            hardwareHandler->RegisterPluginListener(listener);
        }
    }
}

void LocalHardwareManager::UnInit()
{
    DHLOGI("start");
    compToolFuncsMap_.clear();
    pluginListenerMap_.clear();
}

void LocalHardwareManager::QueryLocalHardware(const DHType dhType, IHardwareHandler *hardwareHandler)
{
    std::vector<DHItem> dhItems;
    int32_t retryTimes = QUERY_RETRY_MAX_TIMES;
    while (retryTimes > 0) {
        DHLOGI("Query hardwareHandler retry times left: %d, dhType: %#X", retryTimes, dhType);
        dhItems = hardwareHandler->Query();
        if (dhItems.empty()) {
            DHLOGE("Query hardwareHandler and obtain empty, dhType: %#X", dhType);
            usleep(QUERY_INTERVAL_TIME);
        } else {
            DHLOGI("Query hardwareHandler success, dhType: %#X!", dhType);

            /*
             * Failed to delete data when the device restarts or other exception situation.
             * So check and remove the non-exist local capabilityInfo.
             */
            CheckNonExistCapabilityInfo(dhItems, dhType);
            AddLocalCapabilityInfo(dhItems, dhType);
            break;
        }
        retryTimes--;
    }
}

void LocalHardwareManager::AddLocalCapabilityInfo(const std::vector<DHItem> &dhItems, const DHType dhType)
{
    DHLOGI("start!");
    std::vector<std::shared_ptr<CapabilityInfo>> capabilityInfos;
    std::string deviceId = DHContext::GetInstance().GetDeviceInfo().deviceId;
    std::string devName = DHContext::GetInstance().GetDeviceInfo().deviceName;
    uint16_t devType = DHContext::GetInstance().GetDeviceInfo().deviceType;
    for (auto dhItem : dhItems) {
        std::shared_ptr<CapabilityInfo> dhCapabilityInfo =
            std::make_shared<CapabilityInfo>(dhItem.dhId, deviceId, devName, devType, dhType, dhItem.attrs);
        capabilityInfos.push_back(dhCapabilityInfo);
    }
    CapabilityInfoManager::GetInstance()->AddCapability(capabilityInfos);
}

void LocalHardwareManager::CheckNonExistCapabilityInfo(const std::vector<DHItem> &dhItems, const DHType dhType)
{
    DHLOGI("start");
    if (dhType != DHType::INPUT) {
        DHLOGI("This dhType is not input and no need check!");
        return;
    }
    CapabilityInfoMap allLocalCapabilityInfos;
    GetLocalCapabilityMapByPrefix(dhType, allLocalCapabilityInfos);
    for (auto capabilityInfo : allLocalCapabilityInfos) {
        std::shared_ptr<CapabilityInfo> capabilityValue = capabilityInfo.second;
        if (capabilityValue == nullptr) {
            DHLOGE("capabilityInfo value is nullptr");
            continue;
        }
        DHLOGI("The key in allLocalCapabilityInfos is %s", capabilityValue->GetAnonymousKey().c_str());
        bool isExist = false;
        for (auto dhItem : dhItems) {
            DHLOGI("This data key is: %s, dhItem: %s", capabilityValue->GetAnonymousKey().c_str(),
                GetAnonyString(dhItem.dhId).c_str());
            if (capabilityValue->GetDHId() == dhItem.dhId) {
                DHLOGI("This data is exist, no need removed key: %s", capabilityValue->GetAnonymousKey().c_str());
                isExist = true;
                break;
            }
        }
        if (!isExist) {
            DHLOGI("This data is non-exist, it should be removed, key: %s", capabilityValue->GetAnonymousKey().c_str());
            CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoByKey(capabilityValue->GetKey());
        }
    }
    DHLOGI("end");
}

void LocalHardwareManager::GetLocalCapabilityMapByPrefix(const DHType dhType, CapabilityInfoMap &capabilityInfoMap)
{
    std::string localDeviceId = DHContext::GetInstance().GetDeviceInfo().deviceId;
    if (localDeviceId.size() == 0 || localDeviceId.size() > MAX_ID_LEN) {
        DHLOGE("LocalDeviceId is invalid");
        return;
    }
    if (DHTypePrefixMap.find(dhType) == DHTypePrefixMap.end()) {
        DHLOGE("DHTypePrefixMap can not find dhType: %#X", dhType);
        return;
    }
    std::string prefix = DHTypePrefixMap.find(dhType)->second;
    std::string localCapabilityPrefix = localDeviceId + RESOURCE_SEPARATOR + prefix;
    CapabilityInfoManager::GetInstance()->GetDataByKeyPrefix(localCapabilityPrefix, capabilityInfoMap);
}
} // namespace DistributedHardware
} // namespace OHOS
