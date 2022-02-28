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

#include "online_task.h"

#include "anonymous_string.h"
#include "capability_info_manager.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "task_board.h"
#include "task_executor.h"
#include "task_factory.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "OnLineTask"

OnLineTask::OnLineTask(const std::string &networkId, const std::string &devId, const std::string &dhId)
    : Task(networkId, devId, dhId)
{
    SetTaskType(TaskType::ON_LINE);
    SetTaskSteps(std::vector<TaskStep> { TaskStep::SYNC_ONLINE_INFO, TaskStep::REGISTER_ONLINE_DISTRIBUTED_HARDWARE });
    DHLOGD("id = %s, devId = %s", GetId().c_str(), GetAnonyString(devId).c_str());
}

OnLineTask::~OnLineTask()
{
    DHLOGD("id = %s, devId = %s", GetId().c_str(), GetAnonyString(GetDevId()).c_str());
}

void OnLineTask::DoTask()
{
    DHLOGD("start online task, id = %s, devId = %s", GetId().c_str(), GetAnonyString(GetDevId()).c_str());
    this->SetTaskState(TaskState::RUNNING);
    for (auto& step : this->GetTaskSteps()) {
        switch (step) {
            case TaskStep::SYNC_ONLINE_INFO: {
                DoSyncInfo();
                break;
            }
            case TaskStep::REGISTER_ONLINE_DISTRIBUTED_HARDWARE: {
                CreateEnableTask();
                break;
            }
            default: {
                break;
            }
        }
    }
    SetTaskState(TaskState::SUCCESS);
    TaskBoard::GetInstance().RemoveTask(this->GetId());
    DHLOGD("finish online task, remove it, id = %s.", GetId().c_str());
}

void OnLineTask::DoSyncInfo()
{
    DHLOGI("start sync resource when device online, devId = %s", GetAnonyString(GetDevId()).c_str());
    auto ret = CapabilityInfoManager::GetInstance()->ManualSync(GetNetworkId());
    if (ret != DH_FWK_SUCCESS) {
        DHLOGW("ManualSync failed, devId = %s, errCode = %d", GetAnonyString(GetDevId()).c_str(), ret);
    }
    ret = CapabilityInfoManager::GetInstance()->SyncDeviceInfoFromDB(GetDevId());
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("SyncDeviceInfoFromDB failed, devId = %s, errCode = %d", GetAnonyString(GetDevId()).c_str(), ret);
        return;
    }
}

void OnLineTask::CreateEnableTask()
{
    DHLOGI("networkId = %s, devId = %s", GetAnonyString(GetNetworkId()).c_str(), GetAnonyString(GetDevId()).c_str());
    std::vector<std::shared_ptr<CapabilityInfo>> capabilityInfos;
    CapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(GetDevId(), capabilityInfos);
    if (capabilityInfos.empty()) {
        DHLOGE("capabilityInfos is empty, can not create enableTask, devId = %s", GetAnonyString(GetDevId()).c_str());
        return;
    }
    for (const auto &iter : capabilityInfos) {
        if (iter == nullptr) {
            DHLOGE("capabilityInfo is null");
            continue;
        }
        auto task = TaskFactory::GetInstance().CreateTask(TaskType::ENABLE, GetNetworkId(), GetDevId(),
            iter->GetDHId(), shared_from_this());
        TaskExecutor::GetInstance().PushTask(task);
    }
}
}
}