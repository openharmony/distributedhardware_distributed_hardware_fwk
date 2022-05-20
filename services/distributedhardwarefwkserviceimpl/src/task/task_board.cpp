/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "task_board.h"

#include "dh_context.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "TaskBoard"

constexpr int32_t TASK_TIMEOUT_MS = 5000;

IMPLEMENT_SINGLE_INSTANCE(TaskBoard);

int32_t TaskBoard::WaitForALLTaskFinish()
{
    // wait for all task finish until timeout
    std::unique_lock<std::mutex> lock(tasksMtx_);
    auto status = conVar_.wait_for(lock, std::chrono::milliseconds(TASK_TIMEOUT_MS),
        [this]() { return tasks_.empty(); });
    if (!status) {
        DHLOGE("wait for all task finish timeout");
        return ERR_DH_FWK_TASK_TIMEOUT;
    }
    DHLOGI("all task finished");

    return DH_FWK_SUCCESS;
}

bool TaskBoard::IsAllTaskFinish()
{
    std::lock_guard<std::mutex> lock(tasksMtx_);
    return this->tasks_.empty();
}

void TaskBoard::AddTask(std::shared_ptr<Task> task)
{
    if (task == nullptr) {
        DHLOGE("task is null, error");
        return;
    }

    std::lock_guard<std::mutex> lock(tasksMtx_);
    DHLOGI("Add task, id: %s", task->GetId().c_str());
    if (this->tasks_.find(task->GetId()) != this->tasks_.end()) {
        DHLOGE("Task id duplicate, id: %d", task->GetId().c_str());
        return;
    }
    this->tasks_.emplace(task->GetId(), task);
}

void TaskBoard::RemoveTask(std::string taskId)
{
    std::lock_guard<std::mutex> lock(tasksMtx_);
    DHLOGI("Remove task, id: %s", taskId.c_str());
    RemoveTaskInner(taskId);
    if (tasks_.empty()) {
        conVar_.notify_one();
    }
}

void TaskBoard::RemoveTaskInner(std::string taskId)
{
    if (tasks_.find(taskId) == tasks_.end()) {
        DHLOGE("Can not find removed task");
        return;
    }

    tasks_.erase(taskId);
}

void TaskBoard::DumpAllTasks(std::vector<TaskDump> &taskInfos)
{
    std::lock_guard<std::mutex> lock(tasksMtx_);
    for (auto t : tasks_) {
        TaskDump taskInfo = {
            .id = t.second->GetId(),
            .taskType = t.second->GetTaskType(),
            .taskParm.networkId = t.second->GetNetworkId(),
            .taskParm.uuid = t.second->GetUUID(),
            .taskParm.dhId = t.second->GetDhId(),
            .taskParm.dhType = t.second->GetDhType(),
            .taskSteps = t.second->GetTaskSteps()
        };
        taskInfos.emplace_back(taskInfo);
    }
}
} // namespace DistributedHardware
} // namespace OHOS
