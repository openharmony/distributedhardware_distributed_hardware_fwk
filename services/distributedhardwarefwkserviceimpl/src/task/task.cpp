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

#include "task.h"

#include "constants.h"
#include "dh_utils_tool.h"

namespace OHOS {
namespace DistributedHardware {
Task::Task(const std::string &networkId, const std::string &devId, const std::string &dhId)
    : id_(DH_TASK_NAME_PREFIX + GetRandomID()), networkId_(networkId), devId_(devId), dhId_(dhId)
{}

Task::~Task()
{
    this->childrenTasks_.clear();
}

std::string Task::GetId()
{
    return this->id_;
}

std::string Task::GetNetworkId()
{
    return this->networkId_;
}

std::string Task::GetDevId()
{
    return this->devId_;
}

std::string Task::GetDhId()
{
    return this->dhId_;
}

TaskType Task::GetTaskType()
{
    return this->taskType_;
}

void Task::SetTaskType(TaskType taskType)
{
    this->taskType_ = taskType;
}

void Task::SetTaskSteps(std::vector<TaskStep> taskSteps)
{
    this->taskSteps_.swap(taskSteps);
}

const std::vector<TaskStep> Task::GetTaskSteps()
{
    return this->taskSteps_;
}

TaskState Task::GetTaskState()
{
    return this->taskState_;
}

void Task::SetTaskState(TaskState taskState)
{
    this->taskState_ = taskState;
}

void Task::AddChildrenTask(std::shared_ptr<Task> childrenTask)
{
    std::lock_guard<std::mutex> lock(taskMtx_);
    this->childrenTasks_.push_back(childrenTask);
}

const std::vector<std::shared_ptr<Task>> Task::GetChildrenTasks()
{
    std::lock_guard<std::mutex> lock(taskMtx_);
    return this->childrenTasks_;
}

const std::weak_ptr<Task> Task::GetFatherTask()
{
    return this->fatherTask_;
}

void Task::SetFatherTask(std::shared_ptr<Task> fatherTask)
{
    this->fatherTask_ = fatherTask;
}
}
}