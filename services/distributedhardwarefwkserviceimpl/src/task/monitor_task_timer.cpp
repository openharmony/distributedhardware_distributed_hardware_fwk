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

#include "monitor_task_timer.h"

#include "anonymous_string.h"
#include "capability_info_manager.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "task_board.h"
#include "task_executor.h"
#include "task_factory.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(MonitorTaskTimer);
namespace {
    const std::string MONITOR_TASK_TIMER_HANDLER = "monitor_task_timer_handler";
    const std::string MONITOR_TASK_TIMER_ID = "monitor_task_timer_id";
    constexpr int32_t DELAY_TIME_MS = 5000;
}
#undef DH_LOG_TAG
#define DH_LOG_TAG "MonitorTaskTimer"

MonitorTaskTimer::MonitorTaskTimer()
{
    DHLOGI("MonitorTaskTimer construction");
}

MonitorTaskTimer::~MonitorTaskTimer()
{
    DHLOGI("MonitorTaskTimer destruction");
}

void MonitorTaskTimer::StartTimer()
{
    DHLOGI("start");
    auto busRunner = OHOS::AppExecFwk::EventRunner::Create(MONITOR_TASK_TIMER_HANDLER);
    auto eventHandler = std::make_shared<OHOS::AppExecFwk::EventHandler>(busRunner);
    if (eventHandler == nullptr) {
        DHLOGI("eventHandler construction, this point is empty");
        return;
    }
    monitorTaskTimerThread_ = std::thread(&MonitorTaskTimer::Execute, this, eventHandler);
}

void MonitorTaskTimer::StopTimer()
{
    DHLOGI("start");
    if (monitorTaskTimerThread_.joinable()) {
        monitorTaskTimerThread_.join();
    }
    DHLOGI("end");
}

void MonitorTaskTimer::Execute(const std::shared_ptr<OHOS::AppExecFwk::EventHandler> eventHandler)
{
    DHLOGI("start");
    if (eventHandler == nullptr) {
        DHLOGE("eventHandler is nullptr!");
        return;
    }
    auto enabledDevices = TaskBoard::GetInstance().GetEnabledDevice();
    std::string capabilityKey;
    std::shared_ptr<CapabilityInfo> capInfoPtr = nullptr;
    TaskParam taskParam;
    for (auto item : enabledDevices) {
        capabilityKey = item.first;
        taskParam = item.second;
        if (taskParam.dhType != DHType::INPUT) {
            continue;
        }
        if (CapabilityInfoManager::GetInstance()->GetDataByKey(capabilityKey, capInfoPtr) != DH_FWK_SUCCESS) {
            DHLOGI("CapabilityInfoManager can not find this key in DB, key: %s, networkId: %s, uuid: %s, dhId: %s",
                GetAnonyString(capabilityKey).c_str(), GetAnonyString(taskParam.networkId).c_str(),
                GetAnonyString(taskParam.uuid).c_str(), GetAnonyString(taskParam.dhId).c_str());
            auto task = TaskFactory::GetInstance().CreateTask(TaskType::DISABLE, taskParam, nullptr);
            TaskExecutor::GetInstance().PushTask(task);
        }
    }
    auto monitorTaskTimer = [this, eventHandler] {Execute(eventHandler);};
    eventHandler->PostTask(monitorTaskTimer, MONITOR_TASK_TIMER_ID, DELAY_TIME_MS);
}
} // namespace DistributedHardware
} // namespace OHOS
