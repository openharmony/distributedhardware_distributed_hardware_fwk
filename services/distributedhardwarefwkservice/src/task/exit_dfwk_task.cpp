/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "exit_dfwk_task.h"

#include <pthread.h>
#include <thread>

#include "ffrt.h"

#include "anonymous_string.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "distributed_hardware_manager_factory.h"
#include "task_board.h"

namespace OHOS {
namespace DistributedHardware {

#undef DH_LOG_TAG
#define DH_LOG_TAG "ExitDfwkTask"

namespace {
constexpr int32_t WAIT_TIME_SEC = 5;
}

ExitDfwkTask::ExitDfwkTask(const std::string &networkId, const std::string &uuid, const std::string &udid,
    const std::string &dhId, const DHType dhType) : Task(networkId, uuid, udid, dhId, dhType)
{
    SetTaskType(TaskType::EXIT_DFWK);
    DHLOGD("ExitDfwkTask id: %{public}s", GetId().c_str());
}

ExitDfwkTask::~ExitDfwkTask()
{
    DHLOGD("id = %{public}s", GetId().c_str());
}

void ExitDfwkTask::DoTask()
{
    ffrt::submit([this]() { this->DoTaskInner(); });
}

void ExitDfwkTask::DoTaskInner()
{
    DHLOGI("do exit dfwk task bebin!");
    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME_SEC));
    DistributedHardwareManagerFactory::GetInstance().CheckExitSAOrNot();
    DHLOGI("do exit dfwk task end!");
}
} // namespace DistributedHardware
} // namespace OHOS
