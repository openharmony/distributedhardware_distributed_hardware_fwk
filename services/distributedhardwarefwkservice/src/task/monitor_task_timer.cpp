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

#include "capability_info.h"
#include "capability_info_manager.h"
#include "distributed_hardware_errno.h"
#include "dh_timer.h"
#include "task_board.h"
#include "task_executor.h"
#include "task_factory.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "MonitorTaskTimer"
MonitorTaskTimer::MonitorTaskTimer(std::string timerId, int32_t delayTimeMs) : DHTimer(timerId, delayTimeMs)
{
    DHLOGI("MonitorTaskTimer ctor!");
}

MonitorTaskTimer::~MonitorTaskTimer()
{
    DHLOGI("MonitorTaskTimer dtor!");
}

void MonitorTaskTimer::ExecuteInner()
{
    DHLOGD("ExecuteInner!");
    auto enabledDevices = TaskBoard::GetInstance().GetEnabledDevice();
    std::shared_ptr<CapabilityInfo> capInfoPtr = nullptr;
    TaskParam taskParam;
    std::string capabilityKey;
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
}

void MonitorTaskTimer::HandleStopTimer()
{
    DHLOGI("HandleStopTimer!");
}
} // namespace DistributedHardware
} // namespace OHOS
