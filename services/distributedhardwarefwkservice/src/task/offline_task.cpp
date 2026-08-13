/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "ffrt.h"

#include "anonymous_string.h"
#include "capability_info_manager.h"
#include "dh_context.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "distributed_hardware_manager_factory.h"
#include "local_capability_info_manager.h"
#include "meta_info_manager.h"
#include "task_board.h"
#include "task_executor.h"
#include "task_factory.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr uint16_t PHONE_TYPE = 14;
    constexpr const char *OFFLINE_TASK_INNER = "OffLineTask";
    constexpr int32_t USLEEP_TIME = 1000 * 100;
    constexpr int32_t RETRY_MAX_TIMES = 30;
}
#undef DH_LOG_TAG
#define DH_LOG_TAG "OffLineTask"

OffLineTask::OffLineTask(const std::string &networkId, const std::string &uuid, const std::string &udid,
    const std::string &dhId, const DHType dhType) : Task(networkId, uuid, udid, dhId, dhType)
{
    this->SetTaskType(TaskType::OFF_LINE);
    this->SetTaskSteps({TaskStep::META_DISABLE_TASK, TaskStep::UNREGISTER_OFFLINE_DISTRIBUTED_HARDWARE,
        TaskStep::DISABLE_SINK, TaskStep::WAIT_UNREGISTGER_COMPLETE, TaskStep::CLEAR_OFFLINE_INFO});
    DHLOGD("OffLineTask id: %{public}s, networkId: %{public}s, uuid: %{public}s, udid: %{public}s",
        GetId().c_str(), GetAnonyString(GetNetworkId()).c_str(), GetAnonyString(GetUUID()).c_str(),
        GetAnonyString(GetUDID()).c_str());
}

OffLineTask::~OffLineTask()
{
    DHLOGD("id = %{public}s, uuid = %{public}s", GetId().c_str(), GetAnonyString(GetUUID()).c_str());
}

void OffLineTask::DoTask()
{
    ffrt::submit([this]() { this->DoTaskInner(); });
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
            case TaskStep::DISABLE_SINK: {
                CreateDisableSinkTask();
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
            case TaskStep::META_DISABLE_TASK: {
                CreateMetaDisableTask();
                break;
            }
            default: {
                break;
            }
        }
    }

    this->SetTaskState(TaskState::SUCCESS);
    DHLOGI("Finish OffLine task, remove it, id: %{public}s", GetId().c_str());
    TaskBoard::GetInstance().RemoveTask(this->GetId());
    int32_t retryCount = 0;
    if (DHContext::GetInstance().GetRealTimeOnlineDeviceCount() == 0 &&
        DHContext::GetInstance().GetIsomerismConnectCount() == 0) {
        while (!TaskBoard::GetInstance().IsAllDisableTaskFinish() && retryCount < RETRY_MAX_TIMES) {
            retryCount++;
            usleep(USLEEP_TIME);
            DHLOGI("judge disable task is finished, retryCount = %{public}d", retryCount);
        }
        DHLOGI("all devices are offline and all disable tasks are finished, start to free the resource");
        DistributedHardwareManagerFactory::GetInstance().UnInit();
    }
}

void OffLineTask::CreateDisableTask()
{
    std::string deviceId = GetDeviceIdByUUID(GetUUID());
    DHLOGI("create disablesource task, networkId = %{public}s, uuid = %{public}s, deviceId = %{public}s",
        GetAnonyString(GetNetworkId()).c_str(), GetAnonyString(GetUUID()).c_str(), GetAnonyString(deviceId).c_str());
    std::vector<std::pair<std::string, DHType>> devDhInfos;
    std::vector<std::shared_ptr<CapabilityInfo>> capabilityInfos;
    CapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(deviceId, capabilityInfos);
    std::for_each(capabilityInfos.begin(), capabilityInfos.end(), [&](std::shared_ptr<CapabilityInfo> cap) {
        if (cap != nullptr) {
            devDhInfos.push_back({cap->GetDHId(), cap->GetDHType()});
        }
    });

    if (devDhInfos.empty()) {
        DHLOGW("Can not get cap info from CapabilityInfo, try use local info");
        std::vector<std::shared_ptr<CapabilityInfo>> localCapInfos;
        LocalCapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(deviceId, localCapInfos);
        std::for_each(localCapInfos.begin(), localCapInfos.end(), [&](std::shared_ptr<CapabilityInfo> cap) {
            if (cap != nullptr) {
                devDhInfos.push_back({cap->GetDHId(), cap->GetDHType()});
            }
        });
    }

    if (devDhInfos.empty()) {
        DHLOGW("Can not get cap info from CapabilityInfo, try use meta info");
        std::vector<std::shared_ptr<MetaCapabilityInfo>> metaCapInfos;
        std::string udidHash = DHContext::GetInstance().GetUdidHashIdByUUID(GetUUID());
        MetaInfoManager::GetInstance()->GetMetaCapInfosByUdidHash(udidHash, metaCapInfos);
        std::for_each(metaCapInfos.begin(), metaCapInfos.end(), [&](std::shared_ptr<MetaCapabilityInfo> cap) {
            if (cap != nullptr) {
                devDhInfos.push_back({cap->GetDHId(), cap->GetDHType()});
            }
        });
    }

    if (devDhInfos.empty()) {
        DHLOGE("Can not get cap info, uuid = %{public}s", GetAnonyString(GetUUID()).c_str());
        return;
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

void OffLineTask::CreateDisableSinkTask()
{
    DHLOGI("CreateDisableSinkTask start");
    DeviceInfo localDeviceInfo = GetLocalDeviceInfo();
    std::vector<std::pair<std::string, DHType>> localMetaInfos;
    std::vector<std::shared_ptr<MetaCapabilityInfo>> metaCapInfos;
    MetaInfoManager::GetInstance()->GetMetaCapInfosByUdidHash(localDeviceInfo.udidHash, metaCapInfos);
    std::for_each(metaCapInfos.begin(), metaCapInfos.end(), [&](std::shared_ptr<MetaCapabilityInfo> localMetaInfo) {
        if (localMetaInfo != nullptr) {
            localMetaInfos.push_back({localMetaInfo->GetDHId(), localMetaInfo->GetDHType()});
        }
    });
    if (localMetaInfos.empty()) {
        DHLOGE("Can not get localMetainfo.");
        return;
    }
    for (const auto &localInfo : localMetaInfos) {
        if (localInfo.second == DHType::MODEM) {
            continue;
        }
        TaskParam taskParam = {
            .networkId = localDeviceInfo.networkId,
            .uuid = localDeviceInfo.uuid,
            .udid = localDeviceInfo.udid,
            .dhId = localInfo.first,
            .dhType = localInfo.second
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
    DHContext::GetInstance().RemoveOnlineDeviceIdEntryByNetworkId(GetNetworkId());
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
    std::lock_guard<std::mutex> lock(unFinishTaskMtx_);
    this->unFinishChildrenTasks_.erase(taskId);
    if (unFinishChildrenTasks_.empty()) {
        finishCondVar_.notify_all();
    }
}

void OffLineTask::AddChildrenTask(std::shared_ptr<Task> childrenTask)
{
    std::lock_guard<std::mutex> lock(unFinishTaskMtx_);
    this->unFinishChildrenTasks_.insert(childrenTask->GetId());
}

void OffLineTask::CreateMetaDisableTask()
{
    if (!DHContext::GetInstance().IsDoubleFwkDevice(GetNetworkId())) {
        DHLOGE("online device is not double frame device or networkId not find.");
        return;
    }
    uint16_t deviceType = DHContext::GetInstance().GetDeviceTypeByNetworkId(GetNetworkId());
    if (deviceType != PHONE_TYPE) {
        DHLOGE("offline device not phone, deviceType = %{public}d", deviceType);
        return;
    }
    DHLOGI("CreateMetaDisableTask, networkId = %{public}s, uuid = %{public}s", GetAnonyString(GetNetworkId()).c_str(),
        GetAnonyString(GetUUID()).c_str());
    TaskParam taskParam = {
        .networkId = GetNetworkId(),
        .uuid = GetUUID(),
        .dhId = "Modem_1234",
        .dhType = DHType::MODEM
    };
    auto task = TaskFactory::GetInstance().CreateTask(TaskType::META_DISABLE, taskParam, shared_from_this());
    TaskExecutor::GetInstance().PushTask(task);
}
} // namespace DistributedHardware
} // namespace OHOS
