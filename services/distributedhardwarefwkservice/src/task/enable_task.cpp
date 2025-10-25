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

#include "enable_task.h"

#include <pthread.h>

#include "ffrt.h"

#include "anonymous_string.h"
#include "capability_utils.h"
#include "component_manager.h"
#include "dh_utils_hitrace.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "task_board.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr const char *ENABLE_TASK_INNER = "EnableTask";
}

#undef DH_LOG_TAG
#define DH_LOG_TAG "EnableTask"

EnableTask::EnableTask(const std::string &networkId, const std::string &uuid, const std::string &udid,
    const std::string &dhId, const DHType dhType) : Task(networkId, uuid, udid, dhId, dhType)
{
    SetTaskType(TaskType::ENABLE);
    SetTaskSteps(std::vector<TaskStep> { TaskStep::DO_ENABLE });
    DHLOGD("EnableTask id: %{public}s, networkId: %{public}s, dhId: %{public}s",
        GetId().c_str(), GetAnonyString(networkId).c_str(), GetAnonyString(dhId).c_str());
}

EnableTask::~EnableTask()
{
    DHLOGD("id = %{public}s, uuid = %{public}s", GetId().c_str(), GetAnonyString(GetUUID()).c_str());
}

void EnableTask::DoTask()
{
    ffrt::submit([this]() { this->DoTaskInner(); });
}

void EnableTask::DoTaskInner()
{
    int32_t ret = pthread_setname_np(pthread_self(), ENABLE_TASK_INNER);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("DoTaskInner setname failed.");
    }
    DHLOGD("DoTaskInner id = %{public}s, uuid = %{public}s, dhId = %{public}s", GetId().c_str(),
        GetAnonyString(GetUUID()).c_str(), GetAnonyString(GetDhId()).c_str());
    SetTaskState(TaskState::RUNNING);
    auto result = RegisterHardware();
    auto state = (result == DH_FWK_SUCCESS) ? TaskState::SUCCESS : TaskState::FAIL;
    SetTaskState(state);
    DHLOGD("finish enable task, remove it, id = %{public}s", GetId().c_str());
    if (result == DH_FWK_SUCCESS) {
        TaskParam taskParam = {
            .networkId = GetNetworkId(),
            .uuid = GetUUID(),
            .udid = GetUDID(),
            .dhId = GetDhId(),
            .dhType = GetDhType()
        };
        std::string enabledDeviceKey = GetCapabilityKey(GetDeviceIdByUUID(GetUUID()), GetDhId());
        TaskBoard::GetInstance().SaveEnabledDevice(enabledDeviceKey, taskParam);
    }
    TaskBoard::GetInstance().RemoveTask(GetId());
}

int32_t EnableTask::RegisterHardware()
{
    DHCompMgrTraceStart(GetAnonyString(GetNetworkId()), GetAnonyString(GetDhId()), DH_ENABLE_START);

    int32_t ret = DH_FWK_SUCCESS;

    // Determine whether it is an active enable
    if (GetCallingUid() || GetCallingPid()) {
        // It is an active enable
        ret = DoActiveEnable();
    } else {
        // It is an auto enable
        ret = DoAutoEnable();
    }
    DHLOGI("enable task %{public}s, id = %{public}s, uuid = %{public}s, dhId = %{public}s.",
        (ret == DH_FWK_SUCCESS) ? "success" : "failed", GetId().c_str(), GetAnonyString(GetUUID()).c_str(),
        GetDhId().c_str());
    DHTraceEnd();
    return ret;
}

void EnableTask::SetEffectSink(bool isEffect)
{
    effectSink_ = isEffect;
}

bool EnableTask::GetEffectSink()
{
    return effectSink_;
}

void EnableTask::SetEffectSource(bool isEffect)
{
    effectSource_ = isEffect;
}

bool EnableTask::GetEffectSource()
{
    return effectSource_;
}

void EnableTask::SetCallingUid(int32_t callingUid)
{
    callingUid_ = callingUid;
}

int32_t EnableTask::GetCallingUid()
{
    return callingUid_;
}

void EnableTask::SetCallingPid(int32_t callingPid)
{
    callingPid_ = callingPid;
}

int32_t EnableTask::GetCallingPid()
{
    return callingPid_;
}

int32_t EnableTask::DoAutoEnable()
{
    std::string localUdid = GetLocalUdid();
    if (localUdid == GetUDID()) {
        bool enableSink = false;
        auto ret = ComponentManager::GetInstance().CheckSinkConfigStart(GetDhType(), enableSink);
        if (ret != DH_FWK_SUCCESS) {
            DHLOGE("CheckSinkConfigStart failed!");
            return ret;
        }
        if (!enableSink) {
            DHLOGE("No need Enablesink.");
            return ERR_DH_FWK_COMPONENT_NO_NEED_ENABLE;
        }
        DHDescriptor dhDescriptor {
            .id = GetDhId(),
            .dhType = GetDhType()
        };
        ret = ComponentManager::GetInstance().EnableSink(dhDescriptor, GetCallingUid(), GetCallingPid());
        if (ret != DH_FWK_SUCCESS) {
            DHLOGE("EnableSink DhType = %{public}#X, failed!", GetDhType());
        }
        return ret;
    }

    bool enableSource = false;
    int32_t ret = ComponentManager::GetInstance().CheckDemandStart(GetUUID(), GetDhType(), enableSource);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("CheckDemandStart failed!");
        return ret;
    }
    if (!enableSource) {
        DHLOGE("No need Enablesource.");
        return ERR_DH_FWK_COMPONENT_NO_NEED_ENABLE;
    }
    DHDescriptor dhDescriptor {
        .id = GetDhId(),
        .dhType = GetDhType()
    };
    ret = ComponentManager::GetInstance().EnableSource(GetNetworkId(), dhDescriptor, GetCallingUid(), GetCallingPid());
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("EnableSource DhType = %{public}#X, failed!", GetDhType());
    }
    return ret;
}

int32_t EnableTask::DoActiveEnable()
{
    int32_t ret = DH_FWK_SUCCESS;
    DHDescriptor dhDescriptor {
        .id = GetDhId(),
        .dhType = GetDhType()
    };
    if (GetEffectSink()) {
        ret = ComponentManager::GetInstance().EnableSink(dhDescriptor, GetCallingUid(), GetCallingPid());
        if (ret != DH_FWK_SUCCESS) {
            DHLOGE("EnableSink failed!");
        }
    }
    if (GetEffectSource()) {
        ret = ComponentManager::GetInstance().EnableSource(
            GetNetworkId(), dhDescriptor, GetCallingUid(), GetCallingPid());
        if (ret != DH_FWK_SUCCESS) {
            DHLOGE("EnableSource failed!");
        }
    }
    return ret;
}
} // namespace DistributedHardware
} // namespace OHOS
