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

#ifndef OHOS_DISTRIBUTED_HARDWARE_TASK_BOARD_H
#define OHOS_DISTRIBUTED_HARDWARE_TASK_BOARD_H

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "single_instance.h"
#include "task.h"

namespace OHOS {
namespace DistributedHardware {
class TaskBoard {
DECLARE_SINGLE_INSTANCE(TaskBoard);
public:
    bool IsAllTaskFinish();
    void AddTask(std::shared_ptr<Task> task);
    void RemoveTask(std::string taskId);
    int32_t WaitForALLTaskFinish();

private:
    void RemoveTaskInner(std::string taskId);

private:
    std::condition_variable conVar_;
    std::mutex tasksMtx_;
    std::unordered_map<std::string, std::shared_ptr<Task>> tasks_;
};
}
}
#endif