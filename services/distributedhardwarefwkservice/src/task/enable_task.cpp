/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "enable_task.h"

#include <pthread.h>

#include "ffrt.h"

#include "anonymous_string.h"
#include "capability_utils.h"
#include "component_manager.h"
#include "constants.h"
#include "dh_utils_hitrace.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "task_board.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "EnableTask"

EnableTask::EnableTask(const std::string &networkId, const std::string &uuid, const std::string &udid,
    const std::string &dhId, const DHType dhType) : Task(networkId, uuid, udid, dhId, dhType)
{
    SetTaskType(TaskType::ENABLE);
    SetTaskSteps(std::vector<TaskStep> { TaskStep::DO_ENABLE });
    DHLOGD("EnableTask id: %{public}s, networkId: %{public}s, dhId: %{public}s",
        GetId().c_str(), GetAnonyString(networkId).c_str(), GetAnonyString(dhId).c_str());
}

EnableTask::~EnableTask()
{
    DHLOGD("id = %{public}s, uuid = %{public}s", GetId().c_str(), GetAnonyString(GetUUID()).c_str());
}

void EnableTask::DoTask()
{
    ffrt::submit([this]() { this->DoTaskInner(); });
}

void EnableTask::DoTaskInner()
{
    int32_t ret = pthread_setname_np(pthread_self(), ENABLE_TASK_INNER);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("DoTaskInner setname failed.");
    }
    DHLOGD("DoTaskInner id = %{public}s, uuid = %{public}s, dhId = %{public}s", GetId().c_str(),
        GetAnonyString(GetUUID()).c_str(), GetAnonyString(GetDhId()).c_str());
    SetTaskState(TaskState::RUNNING);
    auto result = RegisterHardware();
    auto state = (result == DH_FWK_SUCCESS) ? TaskState::SUCCESS : TaskState::FAIL;
    SetTaskState(state);
    DHLOGD("finish enable task, remove it, id = %{public}s", GetId().c_str());
    if (result == DH_FWK_SUCCESS) {
        TaskParam taskParam = {
            .networkId = GetNetworkId(),
            .uuid = GetUUID(),
            .udid = GetUDID(),
            .dhId = GetDhId(),
            .dhType = GetDhType()
        };
        std::string enabledDeviceKey = GetCapabilityKey(GetDeviceIdByUUID(GetUUID()), GetDhId());
        TaskBoard::GetInstance().SaveEnabledDevice(enabledDeviceKey, taskParam);
    }
    TaskBoard::GetInstance().RemoveTask(GetId());
}

int32_t EnableTask::RegisterHardware()
{
    DHCompMgrTraceStart(GetAnonyString(GetNetworkId()), GetAnonyString(GetDhId()), DH_ENABLE_START);
    auto result = ComponentManager::GetInstance().Enable(GetNetworkId(), GetUUID(), GetDhId(), GetDhType());
    DHLOGI("enable task %{public}s, id = %{public}s, uuid = %{public}s, dhId = %{public}s",
        (result == DH_FWK_SUCCESS) ? "success" : "failed", GetId().c_str(), GetAnonyString(GetUUID()).c_str(),
        GetAnonyString(GetDhId()).c_str());
    DHTraceEnd();
    return result;
}
} // namespace DistributedHardware
} // namespace OHOS
