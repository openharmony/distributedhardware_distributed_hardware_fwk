/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "task_executor.h"

#include <pthread.h>
#include <thread>

#include "constants.h"
#include "dh_context.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    const uint32_t MAX_TASK_QUEUE_LENGTH = 256;
}
IMPLEMENT_SINGLE_INSTANCE(TaskExecutor);
TaskExecutor::TaskExecutor() : taskThreadFlag_(true)
{
    DHLOGI("Ctor TaskExecutor");
    std::thread(&TaskExecutor::TriggerTask, this).detach();
}

TaskExecutor::~TaskExecutor()
{
    DHLOGI("Dtor TaskExecutor");
    taskThreadFlag_ = false;
}

void TaskExecutor::PushTask(const std::shared_ptr<Task>& task)
{
    if (task == nullptr) {
        DHLOGE("Task is null");
        return;
    }

    {
        DHLOGI("Push task: %s", task->GetId().c_str());
        std::unique_lock<std::mutex> lock(taskQueueMtx_);
        if (taskQueue_.size() > MAX_TASK_QUEUE_LENGTH) {
            DHLOGE("Task queue is full");
            return;
        }
        taskQueue_.push(task);
    }

    condVar_.notify_one();
}

std::shared_ptr<Task> TaskExecutor::PopTask()
{
    std::shared_ptr<Task> task = nullptr;

    std::unique_lock<std::mutex> lock(taskQueueMtx_);
    condVar_.wait(lock, [this] {
        return !(this->taskQueue_.empty());
    });

    if (!taskQueue_.empty()) {
        task = taskQueue_.front();
        taskQueue_.pop();
        DHLOGI("Pop task: %s", task->GetId().c_str());
    }

    return task;
}

void TaskExecutor::TriggerTask()
{
    int32_t ret = pthread_setname_np(pthread_self(), TRIGGER_TASK);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("TriggerTask setname failed.");
    }
    while (taskThreadFlag_) {
        std::shared_ptr<Task> task = PopTask();
        if (task == nullptr) {
            DHLOGE("Pop a null task, error");
            continue;
        }

        auto taskFunc = [task]() {
            task->DoTask();
        };

        DHLOGI("Post task to EventBus: %s", task->GetId().c_str());
        DHContext::GetInstance().GetEventHandler()->PostTask(taskFunc, task->GetId());
    }
}
} // namespace DistributedHardware
} // namespace OHOS
