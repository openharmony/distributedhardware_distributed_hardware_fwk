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

#include "offline_task.h"

#include <thread>

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
#define DH_LOG_TAG "OffLineTask"

OffLineTask::OffLineTask(const std::string &networkId, const std::string &devId, const std::string &dhId)
    : Task(networkId, devId, dhId)
{
    this->SetTaskType(TaskType::OFF_LINE);
    this->SetTaskSteps({TaskStep::UNREGISTER_OFFLINE_DISTRIBUTED_HARDWARE, TaskStep::WAIT_UNREGISTGER_COMPLETE,
        TaskStep::CLEAR_OFFLINE_INFO});
    DHLOGD("id = %s, devId = %s", GetId().c_str(), GetAnonyString(devId).c_str());
}

OffLineTask::~OffLineTask()
{
    DHLOGD("id = %s, devId = %s", GetId().c_str(), GetAnonyString(GetDevId()).c_str());
}

void OffLineTask::DoTask()
{
    std::thread(&OffLineTask::DoTaskInner, this).detach();
}

void OffLineTask::DoTaskInner()
{
    DHLOGD("start offline task, id = %s, devId = %s", GetId().c_str(), GetAnonyString(GetDevId()).c_str());
    this->SetTaskState(TaskState::RUNNING);
    for (auto& step : this->GetTaskSteps()) {
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
    TaskBoard::GetInstance().RemoveTask(this->GetId());
    DHLOGD("Finish OffLine task, remove it, id: %s", GetId().c_str());
}

void OffLineTask::CreateDisableTask()
{
    DHLOGI("networkId = %s, devId = %s", GetAnonyString(GetNetworkId()).c_str(), GetAnonyString(GetDevId()).c_str());
    std::vector<std::shared_ptr<CapabilityInfo>> capabilityInfos;
    CapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(GetDevId(), capabilityInfos);
    if (capabilityInfos.empty()) {
        DHLOGE("capabilityInfos is empty, can not create disableTask, devId = %s", GetAnonyString(GetDevId()).c_str());
        return;
    }
    for (const auto &iter : capabilityInfos) {
        if (iter == nullptr) {
            DHLOGE("capabilityInfo is null");
            continue;
        }
        auto task = TaskFactory::GetInstance().CreateTask(TaskType::DISABLE, GetNetworkId(), GetDevId(),
            iter->GetDHId(), shared_from_this());
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
    DHLOGI("start clear resource when device offline, devId = %s", GetAnonyString(GetDevId()).c_str());
    auto ret = CapabilityInfoManager::GetInstance()->RemoveCapabilityInfoInMem(GetDevId());
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("RemoveCapabilityInfoInMem failed, devId = %s, errCode = %d", GetAnonyString(GetDevId()).c_str(), ret);
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
}
}