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

#include "mock_online_task.h"

#include <chrono>
#include <thread>

#include "distributed_hardware_log.h"
#include "mock_task_factory.h"
#include "task_executor.h"

namespace OHOS {
namespace DistributedHardware {
MockOnLineTask::MockOnLineTask(const std::string &networkId, const std::string &uuid, const std::string &dhId,
    const DHType dhType) : OnLineTask(networkId, uuid, dhId, dhType)
{
    DHLOGI("Ctor MockOnLineTask: %{public}s, type: %{public}d", this->GetId().c_str(), this->GetTaskType());
}

void MockOnLineTask::DoSyncInfo()
{
    DHLOGI("DoSyncInfo");
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(200ms);
}

void MockOnLineTask::CreateEnableTask()
{
    for (const auto& devInfo : onLineDevInfos) {
        TaskParam taskParam = {
            .networkId = devInfo.networkId,
            .uuid = devInfo.uuid,
            .dhId = devInfo.dhId,
            .dhType = devInfo.dhType
        };
        std::shared_ptr<Task> enableTask = MockTaskFactory::GetInstance().CreateTask(TaskType::ENABLE,
            taskParam, shared_from_this());
        TaskExecutor::GetInstance().PushTask(enableTask);
    }
}

void MockOnLineTask::SetOnLineDevInfos(std::vector<MockDevInfo> devInfos)
{
    this->onLineDevInfos = devInfos;
}
} // namespace DistributedHardware
} // namespace OHOS
