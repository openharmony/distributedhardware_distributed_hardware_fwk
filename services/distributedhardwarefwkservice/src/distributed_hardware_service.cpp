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

#include "distributed_hardware_service.h"

#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"

#include "access_manager.h"
#include "dh_utils_hisysevent.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "distributed_hardware_manager_factory.h"
#include "publisher.h"

namespace OHOS {
namespace DistributedHardware {
REGISTER_SYSTEM_ABILITY_BY_ID(DistributedHardwareService, DISTRIBUTED_HARDWARE_SA_ID, true);

DistributedHardwareService::DistributedHardwareService(int32_t saId, bool runOnCreate)
    : SystemAbility(saId, runOnCreate)
{
}

void DistributedHardwareService::OnStart()
{
    DHLOGI("DistributedHardwareService::OnStart start");
    HiSysEventWriteMsg(DHFWK_INIT_BEGIN, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "dhfwk sa start on demand.");

    if (state_ == ServiceRunningState::STATE_RUNNING) {
        DHLOGI("DistributedHardwareService has already started.");
        return;
    }
    if (!Init()) {
        DHLOGE("failed to init DistributedHardwareService");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    DHLOGI("DistributedHardwareService::OnStart start service success.");
}

bool DistributedHardwareService::Init()
{
    DHLOGI("DistributedHardwareService::Init ready to init.");
    if (!registerToService_) {
        bool ret = Publish(this);
        if (!ret) {
            DHLOGE("DistributedHardwareService::Init Publish failed!");
            HiSysEventWriteMsg(DHFWK_INIT_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
                "dhfwk sa init publish failed.");
            return false;
        }
        registerToService_ = true;
    }
    auto ret = AccessManager::GetInstance()->Init();
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("DistributedHardwareService::Init failed.");
        HiSysEventWriteErrCodeMsg(DHFWK_INIT_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            ret, "dhfwk sa AccessManager init fail.");
        return false;
    }
    DHLOGI("DistributedHardwareService::Init init success.");
    HiSysEventWriteMsg(DHFWK_INIT_END, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "dhfwk sa init success.");
    return true;
}

void DistributedHardwareService::OnStop()
{
    DHLOGI("DistributedHardwareService::OnStop ready to stop service.");
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToService_ = false;
}

int32_t DistributedHardwareService::RegisterPublisherListener(const DHTopic topic,
    const sptr<IPublisherListener> &listener)
{
    Publisher::GetInstance().RegisterListener(topic, listener);
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::UnregisterPublisherListener(const DHTopic topic,
    const sptr<IPublisherListener> &listener)
{
    Publisher::GetInstance().UnregisterListener(topic, listener);
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::PublishMessage(const DHTopic topic, const std::string &msg)
{
    Publisher::GetInstance().PublishMessage(topic, msg);
    return DH_FWK_SUCCESS;
}

int DistributedHardwareService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    DHLOGI("DistributedHardwareService  Dump.");
    
    std::vector<std::string> argsStr {};
    for (auto item : args) {
        argsStr.emplace_back(Str16ToStr8(item));
    }

    std::string result("");
    int ret = AccessManager::GetInstance()->Dump(argsStr, result);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Dump error, ret = %d", ret);
    }

    if (dprintf(fd, "%s\n", result.c_str()) < 0) {
        DHLOGE("Hidump dprintf error");
        ret = ERR_DH_FWK_HIDUMP_DPRINTF_ERROR;
    }

    return ret;
}
} // namespace DistributedHardware
} // namespace OHOS
