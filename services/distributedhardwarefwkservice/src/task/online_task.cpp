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

#include "online_task.h"

#include "anonymous_string.h"
#include "capability_info_manager.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "local_capability_info_manager.h"
#include "meta_info_manager.h"
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
    DHLOGD("id = %{public}s, uuid = %{public}s", GetId().c_str(), GetAnonyString(uuid).c_str());
}

OnLineTask::~OnLineTask()
{
    DHLOGD("id = %{public}s, uuid = %{public}s", GetId().c_str(), GetAnonyString(GetUUID()).c_str());
}

void OnLineTask::DoTask()
{
    DHLOGD("start online task, id = %{public}s, uuid = %{public}s", GetId().c_str(), GetAnonyString(GetUUID()).c_str());
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
    DHLOGD("finish online task, remove it, id = %{public}s.", GetId().c_str());
    TaskBoard::GetInstance().RemoveTask(this->GetId());
}

void OnLineTask::DoSyncInfo()
{
    std::string deviceId = GetDeviceIdByUUID(GetUUID());
    auto ret = CapabilityInfoManager::GetInstance()->SyncDeviceInfoFromDB(deviceId);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("SyncDeviceInfoFromDB failed, deviceId = %{public}s, uuid = %{public}s, errCode = %{public}d",
            GetAnonyString(deviceId).c_str(), GetAnonyString(GetUUID()).c_str(), ret);
    }

    ret = VersionInfoManager::GetInstance()->SyncVersionInfoFromDB(deviceId);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("SyncVersionInfoFromDB failed, deviceId = %{public}s, uuid = %{public}s, errCode = %{public}d",
            GetAnonyString(deviceId).c_str(), GetAnonyString(GetUUID()).c_str(), ret);
    }

    ret = LocalCapabilityInfoManager::GetInstance()->SyncDeviceInfoFromDB(deviceId);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("SyncLocalCapabilityInfoFromDB failed, deviceId = %{public}s, uuid = %{public}s, errCode = %{public}d",
            GetAnonyString(deviceId).c_str(), GetAnonyString(GetUUID()).c_str(), ret);
    }

    ret = MetaInfoManager::GetInstance()->SyncMetaInfoFromDB(deviceId);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("SyncMetaInfoFromDB failed, deviceId = %{public}s, uuid = %{public}s, errCode = %{public}d",
            GetAnonyString(deviceId).c_str(), GetAnonyString(GetUUID()).c_str(), ret);
    }
}

void OnLineTask::CreateEnableTask()
{
    DHLOGI("networkId = %{public}s, uuid = %{public}s", GetAnonyString(GetNetworkId()).c_str(),
        GetAnonyString(GetUUID()).c_str());
    std::string deviceId = GetDeviceIdByUUID(GetUUID());
    std::vector<std::pair<std::string, DHType>> devDhInfos;
    std::vector<std::shared_ptr<CapabilityInfo>> capabilityInfos;
    CapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(deviceId, capabilityInfos);
    std::for_each(capabilityInfos.begin(), capabilityInfos.end(), [&](std::shared_ptr<CapabilityInfo> cap) {
        devDhInfos.push_back({cap->GetDHId(), cap->GetDHType()});
    });

    if (devDhInfos.empty()) {
        DHLOGW("Can not get cap info from CapabilityInfo, try use local Capability info");
        LocalCapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(deviceId, capabilityInfos);
        std::for_each(capabilityInfos.begin(), capabilityInfos.end(), [&](std::shared_ptr<CapabilityInfo> cap) {
            devDhInfos.push_back({cap->GetDHId(), cap->GetDHType()});
        });
    }

    if (devDhInfos.empty()) {
        DHLOGW("Can not get cap info from local Capbility, try use meta info");
        std::vector<std::shared_ptr<MetaCapabilityInfo>> metaCapInfos;
        MetaInfoManager::GetInstance()->GetMetaCapInfosByDeviceId(deviceId, metaCapInfos);
        std::for_each(metaCapInfos.begin(), metaCapInfos.end(), [&](std::shared_ptr<MetaCapabilityInfo> cap) {
            devDhInfos.push_back({cap->GetDHId(), cap->GetDHType()});
        });
    }

    if (devDhInfos.empty()) {
        DHLOGE("Can not get cap info, uuid = %{public}s, deviceId = %{public}s", GetAnonyString(GetUUID()).c_str(),
            GetAnonyString(deviceId).c_str());
    }

    for (const auto &info : devDhInfos) {
        TaskParam taskParam = {
            .networkId = GetNetworkId(),
            .uuid = GetUUID(),
            .dhId = info.first,
            .dhType = info.second
        };
        auto task = TaskFactory::GetInstance().CreateTask(TaskType::ENABLE, taskParam, shared_from_this());
        TaskExecutor::GetInstance().PushTask(task);
    }
}
} // namespace DistributedHardware
} // namespace OHOS
