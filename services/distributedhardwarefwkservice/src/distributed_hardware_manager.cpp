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

#include "distributed_hardware_manager.h"

#include "anonymous_string.h"
#include "capability_info_manager.h"
#include "component_loader.h"
#include "component_manager.h"
#include "dh_context.h"
#include "dh_utils_hisysevent.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "hidump_helper.h"
#include "local_hardware_manager.h"
#include "publisher.h"
#include "task_board.h"
#include "task_executor.h"
#include "task_factory.h"
#include "version_info_manager.h"
#include "version_manager.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "DistributedHardwareManager"


IMPLEMENT_SINGLE_INSTANCE(DistributedHardwareManager);

int32_t DistributedHardwareManager::Initialize()
{
    DHLOGI("start");

    VersionInfoManager::GetInstance()->Init();

    ComponentLoader::GetInstance().Init();

    VersionManager::GetInstance().Init();

    ComponentManager::GetInstance().Init();

    CapabilityInfoManager::GetInstance()->Init();

    LocalHardwareManager::GetInstance().Init();

    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareManager::Release()
{
    DHLOGI("start");
    TaskBoard::GetInstance().WaitForALLTaskFinish();

    LocalHardwareManager::GetInstance().UnInit();

    CapabilityInfoManager::GetInstance()->UnInit();

    ComponentManager::GetInstance().UnInit();

    VersionManager::GetInstance().UnInit();

    ComponentLoader::GetInstance().UnInit();

    VersionInfoManager::GetInstance()->UnInit();

    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareManager::SendOnLineEvent(const std::string &networkId, const std::string &uuid,
    uint16_t deviceType)
{
    (void)deviceType;

    if (networkId.size() == 0 || networkId.size() > MAX_ID_LEN || uuid.size() == 0 || uuid.size() > MAX_ID_LEN) {
        DHLOGE("NetworkId or uuid is invalid");
        return ERR_DH_FWK_PARA_INVALID;
    }

    DHLOGI("networkId = %s, uuid = %s", GetAnonyString(networkId).c_str(), GetAnonyString(uuid).c_str());

    TaskParam taskParam = {
        .networkId = networkId,
        .uuid = uuid,
        .dhId = "",
        .dhType = DHType::UNKNOWN
    };
    auto task = TaskFactory::GetInstance().CreateTask(TaskType::ON_LINE, taskParam, nullptr);
    TaskExecutor::GetInstance().PushTask(task);
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareManager::SendOffLineEvent(const std::string &networkId, const std::string &uuid,
    uint16_t deviceType)
{
    (void)deviceType;

    if (networkId.empty() || networkId.size() > MAX_ID_LEN || uuid.empty() || uuid.size() > MAX_ID_LEN) {
        DHLOGE("NetworkId or uuid is invalid");
        return ERR_DH_FWK_PARA_INVALID;
    }

    DHLOGI("networkId = %s, uuid = %s", GetAnonyString(networkId).c_str(), GetAnonyString(uuid).c_str());

    TaskParam taskParam = {
        .networkId = networkId,
        .uuid = uuid,
        .dhId = "",
        .dhType = DHType::UNKNOWN
    };
    auto task = TaskFactory::GetInstance().CreateTask(TaskType::OFF_LINE, taskParam, nullptr);
    TaskExecutor::GetInstance().PushTask(task);
    Publisher::GetInstance().PublishMessage(DHTopic::TOPIC_DEV_OFFLINE, networkId);

    HiSysEventWriteCompOfflineMsg(DHFWK_DEV_OFFLINE, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        GetAnonyString(networkId), "dhfwk device offline event.");

    return DH_FWK_SUCCESS;
}

size_t DistributedHardwareManager::GetOnLineCount()
{
    return DHContext::GetInstance().GetOnlineCount();
}

int32_t DistributedHardwareManager::GetComponentVersion(std::unordered_map<DHType, std::string> &versionMap)
{
    DHLOGI("start");
    DHVersion dhVersion;
    int32_t ret = ComponentLoader::GetInstance().GetLocalDHVersion(dhVersion);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetLocalDHVersion fail, errCode = %d", ret);
        return ret;
    }

    for (auto iter = dhVersion.compVersions.cbegin(); iter != dhVersion.compVersions.cend(); ++iter) {
        versionMap.emplace(iter->first, iter->second.sinkVersion);
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareManager::Dump(const std::vector<std::string> &argsStr, std::string &result)
{
    return HidumpHelper::GetInstance().Dump(argsStr, result);
}
} // namespace DistributedHardware
} // namespace OHOS
