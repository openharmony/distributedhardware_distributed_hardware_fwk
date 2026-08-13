/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "disable_task.h"

#include <pthread.h>

#include "ffrt.h"

#include "anonymous_string.h"
#include "capability_utils.h"
#include "component_manager.h"
#include "dh_context.h"
#include "dh_utils_hitrace.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "offline_task.h"
#include "task_board.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr const char *DISABLE_TASK_INNER = "DisableTask";
}

#undef DH_LOG_TAG
#define DH_LOG_TAG "DisableTask"

DisableTask::DisableTask(const std::string &networkId, const std::string &uuid, const std::string &udid,
    const std::string &dhId, const DHType dhType) : Task(networkId, uuid, udid, dhId, dhType)
{
    SetTaskType(TaskType::DISABLE);
    SetTaskSteps(std::vector<TaskStep> { TaskStep::DO_DISABLE });
    DHLOGD("DisableTask id: %{public}s, networkId: %{public}s, dhId: %{public}s",
        GetId().c_str(), GetAnonyString(networkId).c_str(), GetAnonyString(dhId).c_str());
}

DisableTask::~DisableTask()
{
    DHLOGD("id = %{public}s, uuid = %{public}s", GetId().c_str(), GetAnonyString(GetUUID()).c_str());
}

void DisableTask::DoTask()
{
    ffrt::submit([this]() { this->DoTaskInner(); });
}

void DisableTask::DoTaskInner()
{
    int32_t ret = pthread_setname_np(pthread_self(), DISABLE_TASK_INNER);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("DoTaskInner setname failed.");
    }
    DHLOGD("id = %{public}s, uuid = %{public}s, dhId = %{public}s", GetId().c_str(), GetAnonyString(GetUUID()).c_str(),
        GetAnonyString(GetDhId()).c_str());
    SetTaskState(TaskState::RUNNING);

    /* trigger Unregister Distributed Hardware Task, sync function */
    auto result = UnRegisterHardware();
    if (result == DH_FWK_SUCCESS) {
        std::string enabledDeviceKey = GetCapabilityKey(GetDeviceIdByUUID(GetUUID()), GetDhId());
        TaskBoard::GetInstance().RemoveEnabledDevice(enabledDeviceKey);
    }
    auto state = (result == DH_FWK_SUCCESS) ? TaskState::SUCCESS : TaskState::FAIL;
    SetTaskState(state);

    DHLOGD("finish disable task, remove it, id = %{public}s", GetId().c_str());
    std::string taskId = GetId();
    std::shared_ptr<Task> father = GetFatherTask().lock();
    TaskBoard::GetInstance().RemoveTask(taskId);
    /* if finish task, notify father finish */
    if (father != nullptr) {
        auto offLineTask = std::static_pointer_cast<OffLineTask>(father);
        offLineTask->NotifyFatherFinish(taskId);
    }
}

int32_t DisableTask::UnRegisterHardware()
{
    DHCompMgrTraceStart(GetAnonyString(GetNetworkId()), GetAnonyString(GetDhId()), DH_DISABLE_START);

    int32_t ret = DH_FWK_SUCCESS;

    // Determine whether it is an active disable
    if (GetCallingUid() || GetCallingPid()) {
        // It is an active disable
        ret = DoActiveDisable();
    } else {
        // It is an auto disable
        ret = DoAutoDisable();
    }
    DHLOGI("disable task %{public}s, id = %{public}s, uuid = %{public}s, dhId = %{public}s.",
        (ret == DH_FWK_SUCCESS) ? "success" : "failed", GetId().c_str(), GetAnonyString(GetUUID()).c_str(),
        GetDhId().c_str());
    DHTraceEnd();
    return ret;
}

void DisableTask::SetEffectSink(bool isEffect)
{
    effectSink_ = isEffect;
}

bool DisableTask::GetEffectSink()
{
    return effectSink_;
}

void DisableTask::SetEffectSource(bool isEffect)
{
    effectSource_ = isEffect;
}

bool DisableTask::GetEffectSource()
{
    return effectSource_;
}

void DisableTask::SetCallingUid(int32_t callingUid)
{
    callingUid_ = callingUid;
}

int32_t DisableTask::GetCallingUid()
{
    return callingUid_;
}

void DisableTask::SetCallingPid(int32_t callingPid)
{
    callingPid_ = callingPid;
}

int32_t DisableTask::GetCallingPid()
{
    return callingPid_;
}

int32_t DisableTask::DoAutoDisable()
{
    std::string localUdid = GetLocalUdid();
    if (localUdid == GetUDID()) {
        auto ret = DH_FWK_SUCCESS;
        if (DHContext::GetInstance().GetRealTimeOnlineDeviceCount() == 0 &&
            DHContext::GetInstance().GetIsomerismConnectCount() == 0) {
            DHDescriptor dhDescriptor {
                .id = GetDhId(),
                .dhType = GetDhType()
            };
            DHLOGI("DisableSinkTask DhType = %{public}#X, id= %{public}s", GetDhType(),
                GetAnonyString(GetDhId()).c_str());
            ret = ComponentManager::GetInstance().ForceDisableSink(dhDescriptor);
            if (ret != DH_FWK_SUCCESS) {
                DHLOGE("DisableTask DhType = %{public}#X, failed!", GetDhType());
            }
        }
        return ret;
    }

    DHDescriptor dhDescriptor {
        .id = GetDhId(),
        .dhType = GetDhType()
    };
    DHLOGI("DisableSourceTask DhType = %{public}#X, id= %{public}s", GetDhType(), GetDhId().c_str());
    auto ret = ComponentManager::GetInstance().ForceDisableSource(GetNetworkId(), dhDescriptor);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("DisableSource failed!");
    }
    return ret;
}

int32_t DisableTask::DoActiveDisable()
{
    int32_t ret = DH_FWK_SUCCESS;
    DHDescriptor dhDescriptor {
        .id = GetDhId(),
        .dhType = GetDhType()
    };
    if (GetEffectSink()) {
        ret = ComponentManager::GetInstance().DisableSink(dhDescriptor, GetCallingUid(), GetCallingPid());
        if (ret != DH_FWK_SUCCESS) {
            DHLOGE("DisableSink failed!");
        }
    }
    if (GetEffectSource()) {
        ret = ComponentManager::GetInstance().DisableSource(
            GetNetworkId(), dhDescriptor, GetCallingUid(), GetCallingPid());
        if (ret != DH_FWK_SUCCESS) {
            DHLOGE("DisableSource failed!");
        }
    }
    return ret;
}
} // namespace DistributedHardware
} // namespace OHOS
