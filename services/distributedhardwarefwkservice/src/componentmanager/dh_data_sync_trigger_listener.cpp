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

#include "dh_data_sync_trigger_listener.h"

#include <cinttypes>

#include "anonymous_string.h"
#include "component_manager.h"
#include "distributed_hardware_log.h"
#include "event_handler.h"

class ComponentManager;
namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "DHDataSyncTriggerListener"
DHDataSyncTriggerListener::DHDataSyncTriggerListener()
{
    DHLOGI("Ctor DHDataSyncTriggerListener");
}

DHDataSyncTriggerListener::~DHDataSyncTriggerListener()
{
    DHLOGI("Dtor DHDataSyncTriggerListener");
}

void DHDataSyncTriggerListener::OnDataSyncTrigger(const std::string &networkId)
{
    DHLOGI("Receive data sync trigger, networkId: %{public}s", GetAnonyString(networkId).c_str());
    if (networkId.empty()) {
        DHLOGE("OnDataSyncTrigger networkId is empty");
        return;
    }
    std::shared_ptr<std::string> networkIdPtr = std::make_shared<std::string>(networkId);
    AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(EVENT_DATA_SYNC_MANUAL, networkIdPtr);
    ComponentManager::GetInstance().GetEventHandler()->SendEvent(msgEvent,
        0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}
} // namespace DistributedHardware
} // namespace OHOS