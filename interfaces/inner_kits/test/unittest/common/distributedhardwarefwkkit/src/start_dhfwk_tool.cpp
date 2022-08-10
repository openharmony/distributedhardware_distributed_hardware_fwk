/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "start_dhfwk_tool.h"

#include <cstdint>
#include <cinttypes>
#include <cstdio>
#include <sys/types.h>
#include <securec.h>
#include <unistd.h>

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "distributed_hardware_log.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "DHFWKKitTest"

namespace OHOS {
namespace DistributedHardware {
namespace {
    const int32_t LOAD_SA_TIMEOUT_MS = 1000;
}
bool StartDHFWKTool::StartDHFWK()
{
    DHLOGI("Try Start DHFWK sa");
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!samgr) {
        DHLOGE("Get SystemAbilityManager failed");
        return false;
    }

    sptr<LoadDHFWKSACallback> loadCallback = new LoadDHFWKSACallback(this);
    int32_t ret = samgr->LoadSystemAbility(DISTRIBUTED_HARDWARE_SA_ID, loadCallback);
    if (ret != ERR_OK) {
        DHLOGE("Failed Load DHFWK system ability, ret: %" PRId32, ret);
        return false;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    DHLOGI("Wait DHFWK sa to start");
    auto waitStatus = loadConVar_.wait_for(lock, std::chrono::milliseconds(LOAD_SA_TIMEOUT_MS),
        [this]() { return this->isDHFWKStart_.load(); });
    if (!waitStatus) {
        DHLOGE("DHFWK load sa timeout.");
        return false;
    }

    DHLOGI("Start DHFWK sa success");
    return true;
}

void StartDHFWKTool::KillProcess(const char *processname)
{
    if (processname == nullptr) {
        DHLOGE("process name invalid");
        return;
    }

    char cmd[128] = "";
    int32_t ret = sprintf_s(cmd, sizeof(cmd), "kill -9 $(pidof %s)", processname);
    if (ret < 0) {
        DHLOGE("Kill Process error, cmd: %s, ret: " PRId32, cmd, ret);
        return;
    }

    system(cmd);
}

StartDHFWKTool::LoadDHFWKSACallback::LoadDHFWKSACallback(StartDHFWKTool *toolPtr)
{
    this->toolPtr_ = toolPtr;
}

void StartDHFWKTool::LoadDHFWKSACallback::OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
    const sptr<IRemoteObject> &remoteObject)
{
    if (toolPtr_ == nullptr) {
        return;
    }

    std::unique_lock<std::mutex> lock(toolPtr_->mutex_);
    DHLOGI("DHFWK SA Started.");
    toolPtr_->isDHFWKStart_ = true;
}

void StartDHFWKTool::LoadDHFWKSACallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    if (toolPtr_ == nullptr) {
        return;
    }

    std::unique_lock<std::mutex> lock(toolPtr_->mutex_);
    DHLOGI("DHFWK SA Stopped.");
    toolPtr_->isDHFWKStart_ = false;
}
} // namespace DistributedHardware
} // namespace OHOS