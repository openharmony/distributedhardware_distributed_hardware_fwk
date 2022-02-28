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

#include "mock_task_factory.h"

#include "distributed_hardware_log.h"
#include "mock_disable_task.h"
#include "mock_enable_task.h"
#include "mock_offline_task.h"
#include "mock_online_task.h"
#include "task_board.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(MockTaskFactory);
std::shared_ptr<Task> MockTaskFactory::CreateTask(TaskType taskType, const std::string &networkId,
    const std::string &devId, const std::string &dhId, std::shared_ptr<Task> fatherTask)
{
    std::shared_ptr<Task> task = nullptr;
    switch (taskType) {
        case TaskType::ENABLE: {
            task = std::make_shared<MockEnableTask>(networkId, devId, dhId);
            break;
        }
        case TaskType::DISABLE: {
            task = std::make_shared<MockDisableTask>(networkId, devId, dhId);
            break;
        }
        case TaskType::ON_LINE: {
            task = std::make_shared<MockOnLineTask>(networkId, devId, dhId);
            break;
        }
        case TaskType::OFF_LINE: {
            task = std::make_shared<MockOffLineTask>(networkId, devId, dhId);
            break;
        }
        default: {
            DHLOGE("CreateTask type invalid, type: %d", taskType);
            return nullptr;
        }
    }

    if (fatherTask != nullptr) {
        task->SetFatherTask(fatherTask);
        fatherTask->AddChildrenTask(task);
    }
    TaskBoard::GetInstance().AddTask(task);

    return task;
}
}
}