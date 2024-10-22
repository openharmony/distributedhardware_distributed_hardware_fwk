/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "meta_disable_task.h"

#include <pthread.h>

#include "ffrt.h"

#include "anonymous_string.h"
#include "component_manager.h"
#include "dh_modem_context_ext.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "offline_task.h"
#include "task_board.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "MetaDisableTask"

MetaDisableTask::MetaDisableTask(const std::string &networkId, const std::string &uuid, const std::string &udid,
    const std::string &dhId, const DHType dhType) : Task(networkId, uuid, udid, dhId, dhType)
{
    SetTaskType(TaskType::META_DISABLE);
    SetTaskSteps(std::vector<TaskStep> { TaskStep::DO_MODEM_META_DISABLE });
    DHLOGD("DisableTask id: %{public}s, networkId: %{public}s, dhId: %{public}s",
        GetId().c_str(), GetAnonyString(networkId).c_str(), GetAnonyString(dhId).c_str());
}

MetaDisableTask::~MetaDisableTask()
{
    DHLOGD("id = %{public}s, uuid = %{public}s", GetId().c_str(), GetAnonyString(GetUUID()).c_str());
}

void MetaDisableTask::DoTask()
{
    ffrt::submit([this]() { this->DoTaskInner(); });
}

void MetaDisableTask::DoTaskInner()
{
    int32_t ret = pthread_setname_np(pthread_self(), META_DISABLE_TASK_INNER);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("DoTaskInner setname failed.");
    }
    DHLOGD("id = %{public}s, uuid = %{public}s, dhId = %{public}s", GetId().c_str(), GetAnonyString(GetUUID()).c_str(),
        GetAnonyString(GetDhId()).c_str());
    SetTaskState(TaskState::RUNNING);

    auto result = Disable();
    auto state = (result == DH_FWK_SUCCESS) ? TaskState::SUCCESS : TaskState::FAIL;
    SetTaskState(state);
    /* if finish task, notify father finish */
    std::shared_ptr<Task> father = GetFatherTask().lock();
    if (father != nullptr) {
        auto offLineTask = std::static_pointer_cast<OffLineTask>(father);
        offLineTask->NotifyFatherFinish(GetId());
    }
    DHLOGD("finish meta disable task, remove it, id = %{public}s", GetId().c_str());
    TaskBoard::GetInstance().RemoveTask(GetId());
}

int32_t MetaDisableTask::Disable()
{
    IDistributedHardwareSource *sourcePtr = ComponentManager::GetInstance().GetDHSourceInstance(DHType::MODEM);
    if (sourcePtr == nullptr) {
        DHLOGW("GetDHSourceInstance is nullptr.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    std::shared_ptr<IDistributedModemExt> distributedModemExt_ =
            DHModemContextExt::GetInstance().GetModemExtInstance();
    if (distributedModemExt_ == nullptr) {
        DHLOGE("GetModemExtInstance is nullptr.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    DHLOGI("Meta disable, networkId = %{public}s", GetAnonyString(GetNetworkId()).c_str());
    if (distributedModemExt_->Disable(GetNetworkId(), sourcePtr) != DH_FWK_SUCCESS) {
        DHLOGW("Meta disable failed, dhId = %{public}s.", GetAnonyString(GetDhId()).c_str());
        return ERR_DH_FWK_PARA_INVALID;
    }
    return DH_FWK_SUCCESS;
}

} // namespace DistributedHardware
} // namespace OHOS
