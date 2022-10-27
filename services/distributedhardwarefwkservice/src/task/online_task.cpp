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

#include "online_task.h"

#include "anonymous_string.h"
#include "capability_info_manager.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "task_board.h"
#include "task_executor.h"
#include "task_factory.h"
#include "version_info_manager.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "OnLineTask"

OnLineTask::OnLineTask(const std::string &networkId, const std::string &uuid, const std::string &dhId,
    const DHType dhType) : Task(networkId, uuid, dhId, dhType)
{
    SetTaskType(TaskType::ON_LINE);
    SetTaskSteps(std::vector<TaskStep> { TaskStep::SYNC_ONLINE_INFO, TaskStep::REGISTER_ONLINE_DISTRIBUTED_HARDWARE });
    DHLOGD("id = %s, uuid = %s", GetId().c_str(), GetAnonyString(uuid).c_str());
}

OnLineTask::~OnLineTask()
{
    DHLOGD("id = %s, uuid = %s", GetId().c_str(), GetAnonyString(GetUUID()).c_str());
}

void OnLineTask::DoTask()
{
    DHLOGD("start online task, id = %s, uuid = %s", GetId().c_str(), GetAnonyString(GetUUID()).c_str());
    this->SetTaskState(TaskState::RUNNING);
    for (const auto& step : this->GetTaskSteps()) {
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
    DHLOGD("finish online task, remove it, id = %s.", GetId().c_str());
    TaskBoard::GetInstance().RemoveTask(this->GetId());
}

void OnLineTask::DoSyncInfo()
{
    DHLOGI("start sync resource when device online, uuid = %s", GetAnonyString(GetUUID()).c_str());
    auto ret = CapabilityInfoManager::GetInstance()->ManualSync(GetNetworkId());
    if (ret != DH_FWK_SUCCESS) {
        DHLOGW("ManualSync failed, uuid = %s, errCode = %d", GetAnonyString(GetUUID()).c_str(), ret);
    }

    ret = VersionInfoManager::GetInstance()->ManualSync(GetNetworkId());
    if (ret != DH_FWK_SUCCESS) {
        DHLOGW("ManualSync version failed, uuid = %s, errCode = %d", GetAnonyString(GetUUID()).c_str(), ret);
    }

    ret = CapabilityInfoManager::GetInstance()->SyncDeviceInfoFromDB(GetDeviceIdByUUID(GetUUID()));
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("SyncDeviceInfoFromDB failed, uuid = %s, errCode = %d", GetAnonyString(GetUUID()).c_str(), ret);
    }

    ret = VersionInfoManager::GetInstance()->SyncVersionInfoFromDB(GetDeviceIdByUUID(GetUUID()));
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("SyncVersionInfoFromDB failed, uuid = %s, errCode = %d", GetAnonyString(GetUUID()).c_str(), ret);
    }
}

void OnLineTask::CreateEnableTask()
{
    DHLOGI("networkId = %s, uuid = %s", GetAnonyString(GetNetworkId()).c_str(), GetAnonyString(GetUUID()).c_str());
    std::vector<std::shared_ptr<CapabilityInfo>> capabilityInfos;
    CapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(GetDeviceIdByUUID(GetUUID()), capabilityInfos);
    if (capabilityInfos.empty()) {
        DHLOGE("capabilityInfos is empty, can not create enableTask, uuid = %s", GetAnonyString(GetUUID()).c_str());
        return;
    }
    for (const auto &iter : capabilityInfos) {
        if (iter == nullptr) {
            DHLOGE("capabilityInfo is null");
            continue;
        }
        TaskParam taskParam = {
            .networkId = GetNetworkId(),
            .uuid = GetUUID(),
            .dhId = iter->GetDHId(),
            .dhType = iter->GetDHType()
        };
        auto task = TaskFactory::GetInstance().CreateTask(TaskType::ENABLE, taskParam, shared_from_this());
        TaskExecutor::GetInstance().PushTask(task);
    }
}
} // namespace DistributedHardware
} // namespace OHOS
