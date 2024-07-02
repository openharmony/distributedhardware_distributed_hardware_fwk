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

#include "disable_task.h"

#include <pthread.h>

#include "anonymous_string.h"
#include "capability_utils.h"
#include "component_manager.h"
#include "constants.h"
#include "dh_utils_hitrace.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "offline_task.h"
#include "task_board.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "DisableTask"

DisableTask::DisableTask(const std::string &networkId, const std::string &uuid, const std::string &udid,
    const std::string &dhId, const DHType dhType) : Task(networkId, uuid, udid, dhId, dhType)
{
    SetTaskType(TaskType::DISABLE);
    SetTaskSteps(std::vector<TaskStep> { TaskStep::DO_DISABLE });
    DHLOGD("DisableTask id: %{public}s, networkId: %{public}s, dhId: %{public}s",
        GetId().c_str(), GetAnonyString(networkId).c_str(), GetAnonyString(dhId).c_str());
}

DisableTask::~DisableTask()
{
    DHLOGD("id = %{public}s, uuid = %{public}s", GetId().c_str(), GetAnonyString(GetUUID()).c_str());
}

void DisableTask::DoTask()
{
    std::thread([this]() { this->DoTaskInner(); }).detach();
}

void DisableTask::DoTaskInner()
{
    int32_t ret = pthread_setname_np(pthread_self(), DISABLE_TASK_INNER);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("DoTaskInner setname failed.");
    }
    DHLOGD("id = %{public}s, uuid = %{public}s, dhId = %{public}s", GetId().c_str(), GetAnonyString(GetUUID()).c_str(),
        GetAnonyString(GetDhId()).c_str());
    SetTaskState(TaskState::RUNNING);

    /* trigger Unregister Distributed Hardware Task, sync function */
    auto result = UnRegisterHardware();
    auto state = (result == DH_FWK_SUCCESS) ? TaskState::SUCCESS : TaskState::FAIL;
    SetTaskState(state);

    /* if finish task, notify father finish */
    std::shared_ptr<Task> father = GetFatherTask().lock();
    if (father != nullptr) {
        auto offLineTask = std::static_pointer_cast<OffLineTask>(father);
        offLineTask->NotifyFatherFinish(GetId());
    }
    DHLOGD("finish disable task, remove it, id = %{public}s", GetId().c_str());
    TaskBoard::GetInstance().RemoveTask(GetId());
    if (result == DH_FWK_SUCCESS) {
        std::string enabledDeviceKey = GetCapabilityKey(GetDeviceIdByUUID(GetUUID()), GetDhId());
        TaskBoard::GetInstance().RemoveEnabledDevice(enabledDeviceKey);
    }
}

int32_t DisableTask::UnRegisterHardware()
{
    DHCompMgrTraceStart(GetAnonyString(GetNetworkId()), GetAnonyString(GetDhId()), DH_DISABLE_START);
    auto result = ComponentManager::GetInstance().Disable(GetNetworkId(), GetUUID(), GetDhId(), GetDhType());
    DHLOGI("disable task %{public}s, id = %{public}s, uuid = %{public}s, dhId = %{public}s",
        (result == DH_FWK_SUCCESS) ? "success" : "failed", GetId().c_str(), GetAnonyString(GetUUID()).c_str(),
        GetAnonyString(GetDhId()).c_str());
    DHTraceEnd();
    return result;
}
} // namespace DistributedHardware
} // namespace OHOS
