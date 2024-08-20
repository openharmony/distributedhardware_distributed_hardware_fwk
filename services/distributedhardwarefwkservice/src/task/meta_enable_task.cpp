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

#include "meta_enable_task.h"

#include <pthread.h>

#include "ffrt.h"

#include "anonymous_string.h"
#include "component_manager.h"
#include "device_type.h"
#include "dh_modem_context_ext.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "task_board.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "MetaEnableTask"

MetaEnableTask::MetaEnableTask(const std::string &networkId, const std::string &uuid, const std::string &udid,
    const std::string &dhId, const DHType dhType) : Task(networkId, uuid, udid, dhId, dhType)
{
    SetTaskType(TaskType::META_ENABLE);
    SetTaskSteps(std::vector<TaskStep> { TaskStep::DO_MODEM_META_ENABLE });
    DHLOGD("EnableTask id: %{public}s, networkId: %{public}s, dhId: %{public}s",
        GetId().c_str(), GetAnonyString(networkId).c_str(), GetAnonyString(dhId).c_str());
}

MetaEnableTask::~MetaEnableTask()
{
    DHLOGD("id = %{public}s, uuid = %{public}s", GetId().c_str(), GetAnonyString(GetUUID()).c_str());
}

void MetaEnableTask::DoTask()
{
    ffrt::submit([this]() { this->DoTaskInner(); });
}

void MetaEnableTask::DoTaskInner()
{
    int32_t ret = pthread_setname_np(pthread_self(), META_ENABLE_TASK_INNER);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("DoTaskInner setname failed.");
    }
    DHLOGD("DoTaskInner id = %{public}s, uuid = %{public}s, dhId = %{public}s", GetId().c_str(),
        GetAnonyString(GetUUID()).c_str(), GetAnonyString(GetDhId()).c_str());
    SetTaskState(TaskState::RUNNING);

    int32_t result = Enable();
    auto state = (result == DH_FWK_SUCCESS) ? TaskState::SUCCESS : TaskState::FAIL;
    SetTaskState(state);
    DHLOGD("finish meta enable task, remove it, id = %{public}s", GetId().c_str());
    TaskBoard::GetInstance().RemoveTask(GetId());
}

int32_t MetaEnableTask::Enable()
{
    if (DHModemContextExt::GetInstance().GetHandler() != DH_FWK_SUCCESS) {
        DHLOGE("load distributed modem_ext so failed");
    }
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
    DHLOGI("Meta enable, networkId = %{public}s", GetAnonyString(GetNetworkId()).c_str());
    if (distributedModemExt_->Enable(GetNetworkId(), sourcePtr) != DH_FWK_SUCCESS) {
        DHLOGW("Meta enable failed, dhId = %{public}s udid = %{public}s.",
            GetAnonyString(GetDhId()).c_str(), GetAnonyString(GetUDID()).c_str());
        return ERR_DH_FWK_PARA_INVALID;
    }
    return DH_FWK_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
