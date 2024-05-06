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

#ifndef OHOS_DISTRIBUTED_HARDWARE_TASK_H
#define OHOS_DISTRIBUTED_HARDWARE_TASK_H

#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "impl_utils.h"

namespace OHOS {
namespace DistributedHardware {
class Task : public std::enable_shared_from_this<Task> {
public:
    Task() = delete;
    Task(const std::string &networkId, const std::string &uuid, const std::string &dhId, const DHType dhType);
    virtual ~Task();
    virtual void DoTask() = 0;

    std::string GetId();
    std::string GetNetworkId();
    std::string GetUUID();
    std::string GetDhId();
    DHType GetDhType();
    TaskType GetTaskType();
    void SetTaskType(TaskType taskType);
    void SetTaskSteps(std::vector<TaskStep> taskSteps);
    const std::vector<TaskStep> GetTaskSteps();

    TaskState GetTaskState();
    void SetTaskState(TaskState taskState);

    virtual void AddChildrenTask(std::shared_ptr<Task> childrenTask);
    const std::vector<std::shared_ptr<Task>> GetChildrenTasks();

    const std::weak_ptr<Task> GetFatherTask();
    void SetFatherTask(std::shared_ptr<Task> fatherTask);

private:
    std::string id_;
    // the remote device networkid
    std::string networkId_;
    // the remote device uuid
    std::string uuid_;
    // the remote device dhid
    std::string dhId_;
    // the remote device dh type
    DHType dhType_;
    TaskType taskType_ { TaskType::UNKNOWN };
    std::vector<TaskStep> taskSteps_;
    std::weak_ptr<Task> fatherTask_;

    std::mutex taskMtx_;
    std::vector<std::shared_ptr<Task>> childrenTasks_;

    TaskState taskState_ { TaskState::INIT };
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
