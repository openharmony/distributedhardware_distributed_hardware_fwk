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

#include "hidump_helper.h"

#include <unordered_map>

#include "capability_info_manager.h"
#include "component_manager.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "task_board.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(HidumpHelper);
namespace {
const std::string ARGS_HELP = "-h";
const std::string LOADED_COMP_LIST = "-l";
const std::string ENABLED_COMP_LIST = "-e";
const std::string TASK_LIST = "-t";
const std::string CAPABILITY_LIST = "-c";

const std::unordered_map<std::string, HidumpFlag> MAP_ARGS = {
    { ARGS_HELP, HidumpFlag::GET_HELP },
    { LOADED_COMP_LIST, HidumpFlag::GET_LOADED_COMP_LIST },
    { ENABLED_COMP_LIST, HidumpFlag::GET_ENABLED_COMP_LIST },
    { TASK_LIST, HidumpFlag::GET_TASK_LIST },
    { CAPABILITY_LIST, HidumpFlag::GET_CAPABILITY_LIST },
};

std::unordered_map<TaskType, std::string> g_mapTaskType = {
    { TaskType::UNKNOWN, "UNKNOWN" },
    { TaskType::ENABLE, "ENABLE" },
    { TaskType::DISABLE, "DISABLE" },
    { TaskType::ON_LINE, "ON_LINE" },
    { TaskType::OFF_LINE, "OFF_LINE" },
};

std::unordered_map<TaskStep, std::string> g_mapTaskStep = {
    { TaskStep::DO_ENABLE, "DO_ENABLE" },
    { TaskStep::DO_DISABLE, "DO_DISABLE" },
    { TaskStep::SYNC_ONLINE_INFO, "SYNC_ONLINE_INFO" },
    { TaskStep::REGISTER_ONLINE_DISTRIBUTED_HARDWARE, "REGISTER_ONLINE_DISTRIBUTED_HARDWARE" },
    { TaskStep::UNREGISTER_OFFLINE_DISTRIBUTED_HARDWARE, "UNREGISTER_OFFLINE_DISTRIBUTED_HARDWARE" },
    { TaskStep::CLEAR_OFFLINE_INFO, "CLEAR_OFFLINE_INFO" },
    { TaskStep::WAIT_UNREGISTGER_COMPLETE, "WAIT_UNREGISTGER_COMPLETE" },
};

std::unordered_map<TaskState, std::string> g_mapTaskState = {
    { TaskState::INIT, "INIT" },
    { TaskState::RUNNING, "RUNNING" },
    { TaskState::SUCCESS, "SUCCESS" },
    { TaskState::FAIL, "FAIL" },
};
}

int32_t HidumpHelper::Dump(const std::vector<std::string>& args, std::string &result)
{
    DHLOGI("HidumpHelper dump start.");
    result.clear();
    int32_t errCode = ERR_DH_FWK_HIDUMP_ERROR;

    if (args.empty()) {
        return ProcessDump(HidumpFlag::GET_HELP, result);
    }

    auto flag = MAP_ARGS.find(args[0]);
    if ((args.size() > 1) || (flag == MAP_ARGS.end())) {
        errCode = ProcessDump(HidumpFlag::UNKNOWN, result);
    } else {
        errCode = ProcessDump(flag->second, result);
    }

    return errCode;
}

int32_t HidumpHelper::ProcessDump(const HidumpFlag &flag, std::string &result)
{
    DHLOGI("Process Dump.");
    int32_t errCode = ERR_DH_FWK_HIDUMP_ERROR;
    switch (flag) {
        case HidumpFlag::GET_HELP: {
            errCode = ShowHelp(result);
            break;
        }
        case HidumpFlag::GET_LOADED_COMP_LIST: {
            errCode = ShowAllLoadedComps(result);
            break;
        }
        case HidumpFlag::GET_ENABLED_COMP_LIST : {
            errCode = ShowAllEnabledComps(result);
            break;
        }
        case HidumpFlag::GET_TASK_LIST : {
            errCode = ShowAllTaskInfos(result);
            break;
        }
        case HidumpFlag::GET_CAPABILITY_LIST : {
            errCode = ShowAllCapabilityInfos(result);
            break;
        }
        default: {
            errCode = ShowIllealInfomation(result);
            break;
        }
    }

    return errCode;
}

int32_t HidumpHelper::ShowAllLoadedComps(std::string &result)
{
    DHLOGI("Dump all loaded compTypes.");
    std::set<DHType> loadedCompSource {};
    std::set<DHType> loadedCompSink {};
    ComponentManager::GetInstance().DumpLoadedComps(loadedCompSource, loadedCompSink);

    result.append("Local loaded components:\n{");
    result.append("\n    Source     : [");
    if (!loadedCompSource.empty()) {
        for (auto compSource : loadedCompSource) {
            std::string dhTypeStr = "UNKNOWN";
            auto it = DHTypeStrMap.find(compSource);
            if (it != DHTypeStrMap.end()) {
                dhTypeStr = it->second;
            }
            result.append(" ");
            result.append(dhTypeStr);
            result.append(",");
        }
        result.replace(result.size() - 1, 1, " ");
    }
    result.append("]");

    result.append("\n    Sink       : [");
    if (!loadedCompSink.empty()) {
        for (auto compSink : loadedCompSink) {
            std::string dhTypeStr = "UNKNOWN";
            auto it = DHTypeStrMap.find(compSink);
            if (it != DHTypeStrMap.end()) {
                dhTypeStr = it->second;
            }
            result.append(" ");
            result.append(dhTypeStr);
            result.append(",");
        }
        result.replace(result.size() - 1, 1, " ");
    }
    result.append("]");
    result.append("\n}\n");
    return DH_FWK_SUCCESS;
}

int32_t HidumpHelper::ShowAllEnabledComps(std::string &result)
{
    DHLOGI("Dump all enabled comps.");
    std::set<HidumpCompInfo> compInfoSet {};
    EnabledCompsDump::GetInstance().Dump(compInfoSet);

    result.append("All enabled components:");
    if (compInfoSet.empty()) {
        return DH_FWK_SUCCESS;
    }

    for (auto info : compInfoSet) {
        std::string dhTypeStr = "UNKNOWN";
        auto it = DHTypeStrMap.find(info.dhType_);
        if (it != DHTypeStrMap.end()) {
            dhTypeStr = it->second;
        }
        result.append("\n{");
        result.append("\n    NetworkId      : ");
        result.append(GetAnonyString(info.networkId_));
        result.append("\n    DHType         : ");
        result.append(dhTypeStr);
        result.append("\n    DHId           : ");
        result.append(GetAnonyString(info.dhId_));
        result.append("\n},");
    }
    result.replace(result.size() - 1, 1, "\n");
    return DH_FWK_SUCCESS;
}

int32_t HidumpHelper::ShowAllTaskInfos(std::string &result)
{
    DHLOGI("Dump all task infos.");
    std::vector<TaskDump> taskInfos {};
    TaskBoard::GetInstance().DumpAllTasks(taskInfos);

    result.append("All task infos:");
    if (taskInfos.empty()) {
        return DH_FWK_SUCCESS;
    }

    for (auto taskInfo : taskInfos) {
        std::string dhTypeStr = "UNKNOWN";
        auto it = DHTypeStrMap.find(taskInfo.taskParm.dhType);
        if (it != DHTypeStrMap.end()) {
            dhTypeStr = it->second;
        }
        result.append("\n{");
        result.append("\n    TaskId     : ");
        result.append(taskInfo.id);
        result.append("\n    TaskType   : ");
        result.append(g_mapTaskType[taskInfo.taskType]);
        result.append("\n    DHType     : ");
        result.append(dhTypeStr);
        result.append("\n    DHId       : ");
        result.append(GetAnonyString(taskInfo.taskParm.dhId));
        result.append("\n    TaskState  : ");
        result.append(g_mapTaskState[taskInfo.taskState]);
        result.append("\n    TaskStep   : [ ");
        std::vector<TaskStep> taskSteps = taskInfo.taskSteps;
        for (auto step : taskSteps) {
            result.append(g_mapTaskStep[step]);
            result.append(" ");
        }
        result.append("]\n");
        result.append("},");
    }
    result.replace(result.size() - 1, 1, "\n");
    return DH_FWK_SUCCESS;
}

int32_t HidumpHelper::ShowAllCapabilityInfos(std::string &result)
{
    DHLOGI("Dump all capability infos.");
    std::vector<CapabilityInfo> capInfos;
    CapabilityInfoManager::GetInstance()->DumpCapabilityInfos(capInfos);

    result.append("All capability info of online components :");
    if (capInfos.empty()) {
        return DH_FWK_SUCCESS;
    }

    for (auto info : capInfos) {
        std::string dhTypeStr = "UNKNOWN";
        auto it = DHTypeStrMap.find(info.GetDHType());
        if (it != DHTypeStrMap.end()) {
            dhTypeStr = it->second;
        }
        result.append("\n{");
        result.append("\n    DeviceName     : ");
        result.append(GetAnonyString(info.GetDeviceName()));
        result.append("\n    DeviceId       : ");
        result.append(GetAnonyString(info.GetDeviceId()));
        result.append("\n    DeviceType     : ");
        result.append(std::to_string(info.GetDeviceType()));
        result.append("\n    DHType         : ");
        result.append(dhTypeStr);
        result.append("\n    DHId           : ");
        result.append(GetAnonyString(info.GetDHId()));
        result.append("\n    DHAttrs        :\n");
        result.append(info.GetDHAttrs());
        result.append("\n},");
    }
    result.replace(result.size() - 1, 1, "\n");
    return DH_FWK_SUCCESS;
}

int32_t HidumpHelper::ShowHelp(std::string &result)
{
    DHLOGI("Show dump help.");
    result.append("DistributedHardwareFramework dump options:\n");
    result.append(" -h    ");
    result.append(": Show help\n");
    result.append(" -l    ");
    result.append(": Show all loaded components\n");
    result.append(" -e    ");
    result.append(": Show all enabled components\n");
    result.append(" -t    ");
    result.append(": Show all tasks\n");
    result.append(" -c    ");
    result.append(": Show all Capability info of online components\n\n");

    return DH_FWK_SUCCESS;
}

int32_t HidumpHelper::ShowIllealInfomation(std::string &result)
{
    DHLOGI("ShowIllealInfomation  Dump.");
    result.clear();
    result.append("Unrecognized option, -h for help.");
    return ERR_DH_FWK_HIDUMP_INVALID_ARGS;
}
} // namespace DistributedHardware
} // namespace OHOS
