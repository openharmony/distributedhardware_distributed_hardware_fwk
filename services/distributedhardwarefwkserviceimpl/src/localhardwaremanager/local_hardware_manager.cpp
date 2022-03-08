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
#include "device_type.h"
#include "dh_context.h"
#include "distributed_hardware_errno.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "LocalHardwareManager"

IMPLEMENT_SINGLE_INSTANCE(LocalHardwareManager)

LocalHardwareManager::LocalHardwareManager() {}
LocalHardwareManager::~LocalHardwareManager() {}

void LocalHardwareManager::Init()
{
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
        QueryLocalHardware(dhType, hardwareHandler);
        if (!hardwareHandler->IsSupportPlugin()) {
            DHLOGI("hardwareHandler is not support hot swap plugin, release!");
            ComponentLoader::GetInstance().ReleaseHardwareHandler(dhType);
            hardwareHandler = nullptr;
        } else {
            compToolFuncsMap_[dhType] = hardwareHandler;
        }
    }
}

void LocalHardwareManager::UnInit()
{
    DHLOGI("start");
    compToolFuncsMap_.clear();
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
}
}
