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

#include "offline_task.h"

#include <pthread.h>
#include <thread>

#include "anonymous_string.h"
#include "capability_info_manager.h"
#include "constants.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "meta_info_manager.h"
#include "task_board.h"
#include "task_executor.h"
#include "task_factory.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "OffLineTask"

OffLineTask::OffLineTask(const std::string &networkId, const std::string &uuid, const std::string &dhId,
    const DHType dhType) : Task(networkId, uuid, dhId, dhType)
{
    this->SetTaskType(TaskType::OFF_LINE);
    this->SetTaskSteps({TaskStep::UNREGISTER_OFFLINE_DISTRIBUTED_HARDWARE, TaskStep::WAIT_UNREGISTGER_COMPLETE,
        TaskStep::CLEAR_OFFLINE_INFO});
    DHLOGD("id = %{public}s, uuid = %{public}s", GetId().c_str(), GetAnonyString(uuid).c_str());
}

OffLineTask::~OffLineTask()
{
    DHLOGD("id = %{public}s, uuid = %{public}s", GetId().c_str(), GetAnonyString(GetUUID()).c_str());
}

void OffLineTask::DoTask()
{
    std::thread(&OffLineTask::DoTaskInner, this).detach();
}

void OffLineTask::DoTaskInner()
{
    int32_t ret = pthread_setname_np(pthread_self(), OFFLINE_TASK_INNER);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("DoTaskInner setname failed.");
    }
    DHLOGD("start offline task, id = %{public}s, uuid = %{public}s", GetId().c_str(),
        GetAnonyString(GetUUID()).c_str());
    this->SetTaskState(TaskState::RUNNING);
    for (const auto& step : this->GetTaskSteps()) {
        switch (step) {
            case TaskStep::UNREGISTER_OFFLINE_DISTRIBUTED_HARDWARE: {
                CreateDisableTask();
                break;
            }
            case TaskStep::WAIT_UNREGISTGER_COMPLETE: {
                WaitDisableTaskFinish();
                break;
            }
            case TaskStep::CLEAR_OFFLINE_INFO: {
                ClearOffLineInfo();
                break;
            }
            default: {
                break;
            }
        }
    }

    this->SetTaskState(TaskState::SUCCESS);
    DHLOGD("Finish OffLine task, remove it, id: %{public}s", GetId().c_str());
    TaskBoard::GetInstance().RemoveTask(this->GetId());
}

void OffLineTask::CreateDisableTask()
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
        DHLOGW("Can not get cap info from CapabilityInfo, try use meta info");
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
        auto task = TaskFactory::GetInstance().CreateTask(TaskType::DISABLE, taskParam, shared_from_this());
        TaskExecutor::GetInstance().PushTask(task);
    }
}

void OffLineTask::WaitDisableTaskFinish()
{
    DHLOGI("start wait disable task finish");
    std::unique_lock<std::mutex> waitLock(unFinishTaskMtx_);
    finishCondVar_.wait(waitLock, [&] { return this->unFinishChildrenTasks_.empty(); });
    DHLOGI("all disable task finish");
}

void OffLineTask::ClearOffLineInfo()
{
    DHLOGI("start clear resource when device offline, uuid = %{public}s", GetAnonyString(GetUUID()).c_str());
    auto ret = CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoInMem(GetDeviceIdByUUID(GetUUID()));
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("RemoveCapabilityInfoInMem failed, uuid = %{public}s, errCode = %{public}d",
            GetAnonyString(GetUUID()).c_str(), ret);
    }
}

void OffLineTask::NotifyFatherFinish(std::string taskId)
{
    {
        std::lock_guard<std::mutex> lock(unFinishTaskMtx_);
        this->unFinishChildrenTasks_.erase(taskId);
    }
    finishCondVar_.notify_all();
}

void OffLineTask::AddChildrenTask(std::shared_ptr<Task> childrenTask)
{
    std::lock_guard<std::mutex> lock(unFinishTaskMtx_);
    this->unFinishChildrenTasks_.insert(childrenTask->GetId());
}
} // namespace DistributedHardware
} // namespace OHOS
