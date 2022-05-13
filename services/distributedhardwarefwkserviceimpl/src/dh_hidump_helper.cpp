/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#include "dh_hidump_helper.h"

#include <unordered_map>

#include "capability_info_manager.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DHHidumpHelper);
namespace {
const int32_t MIN_ARGS_SIZE = 0;
const int32_t MAX_ARGS_SIZE = 1;
const int32_t FIRST_PARAM = 0;
const std::string ARGS_HELP = "-h";
const std::string LOADED_COMP_LIST = "-loaded-list";

const std::unordered_map<std::string, HidumpFlag> ARGS_MAP = {
    { ARGS_HELP, HidumpFlag::GET_HELP },
    { LOADED_COMP_LIST, HidumpFlag::GET_LOADED_COMP_LIST },
};

const std::unordered_map<DHType, std::string> mapDhTypeName = {
    { DHType::UNKNOWN, "UNKNOWN" },
    { DHType::CAMERA, "CAMERA" },
    { DHType::MIC, "MIC" },
    { DHType::SPEAKER, "SPEAKER" },
    { DHType::DISPLAY, "DISPLAY" },
    { DHType::GPS, "GPS" },
    { DHType::BUTTON, "BUTTON" },
    { DHType::HFP, "HFP" },
    { DHType::A2D, "A2D" },
    { DHType::VIRMODEM_MIC, "VIRMODEM_MIC" },
    { DHType::VIRMODEM_SPEAKER, "VIRMODEM_SPEAKER" },
};
}

bool DHHidumpHelper::Dump(const std::vector<std::string>& args, std::string &result)
{
    DHLOGI("DHfwkHidumpHelper Dump args.size():%d.", args.size());
    result.clear();
    int32_t errCode = DH_SUCCESS;
    int32_t argsSize = static_cast<int32_t>(args.size());
    for (int i = 0; i < args.size(); i++) {
        DHLOGI("DHfwkHidumpHelper Dump args[%d]: %s.", i, args.at(i).c_str());
    }
    switch (argsSize) {
        case MIN_ARGS_SIZE: {
            errCode = ProcessDump(HidumpFlag::GET_HELP, result);
            break;
        }
        case MAX_ARGS_SIZE: {
            errCode = ProcessDump(args[FIRST_PARAM], result);
            break;
        }
        default: {
            errCode = ERR_DH_SCREEN_HIDUMP_INVALID_ARGS;
            break;
        }
    }

    bool ret = false;
    switch (errCode) {
        case DH_SUCCESS: {
            ret = true;
            break;
        }
        case ERR_DH_FWK_HIDUMP_INVALID_ARGS: {
            ShowIllealInfomation(result);
            ret = true;
            break;
        }
        case ERR_DH_FWK_HIDUMP_UNKONW: {
            result.append("");
            ret = true;
            break;
        }
        default: {
            break;
        }
    }
    return ret;
}

int32_t DHHidumpHelper::ProcessDump(const HidumpFlag& flag, std::string &result)
{
    DHLOGI("ProcessDump  Dump.");
    result.clear();
    int32_t errCode = ERR_DH_FWK_HIDUMP_ERROR;
    switch (flag) {
        case HidumpFlag::GET_HELP: {
            errCode = ShowHelp(result);
            break;
        }
        case HidumpFlag::GET_LOADED_COMP_LIST: {
            errCode = ShowAllLoadCompTypes(result);
            break;
        }
        default: {
            errCode = ERR_DH_FWK_HIDUMP_INVALID_ARGS;
            break;
        }
    }

    return errCode;
}

void DumpLoadedComps(const DHType dhType)
{
    loadedCompsSet_.emplace(dhType);
}

void DumpUnloadedComps(const DHType dhType)
{
    auto it = loadedCompsSet_.find(dhType);
    if (it != loadedCompsSet_.end()) {
        loadedCompsSet_.earse(it);
    }
}

int32_t DHHidumpHelper::ShowAllLoadCompTypes(std::string &result)
{
    DHLOGI("GetAllLoadCompTypes  Dump.");
    result.append("loaded components:\n");
    result.append("    ");
    for (auto comp : loadedCompsSet_) {
        result.append(mapDhTypeName[comp]);
        result.append(" | ");
    }
    result.append("\n");
    return DH_SUCCESS;
}

void DHHidumpHelper::DumpEnabledComps(const DHType dhType, const std::string &dhId)
{
    HidumpDeviceInfo info = {
        .dhId_ = dhId,
        .dhType_ = dhType,
    }
    deviceInfoSet_.emplace(info);
}

void DHHidumpHelper::DumpDisabledComps(const DHType dhType, const std::string &dhId)
{
    HidumpDeviceInfo info = {
        .dhId_ = dhId,
        .dhType_ = dhType,
    }
    auto it = enabledDeviceInfoSet_.find(info);
    if (it != enabledDeviceInfoSet_.end()) {
        deviceInfoSet_.earse(it);
    }
}

int32_t DHHidumpHelper::ShowAllEnabledComps(std::string &result)
{
    DHLOGI("GetAllLoadCompTypes  Dump.");
    result.append("loaded components:\n");
    result.append("    ");
    for (auto info : enabledDeviceInfoSet_) {
        result.append(" DHType : ");
        result.append(mapDhTypeName[info.dhType_]);
        result.append(" DHId : ");
        result.append(info.dhId_);
        result.append(" | ");
    }
    result.append("\n");
    return DH_SUCCESS;
}

int32_t DHHidumpHelper::ShowHelp(std::string &result)
{
    DHLOGI("ShowHelp  Dump.");
    result.append("Usage:dump  <options>\n")
          .append("Description:\n")
          .append("-loaded-comp-list    ")
          .append(": Show loaded components\n");

    return DH_SUCCESS;
}

void DHHidumpHelper::ShowIllealInfomation(std::string &result)
{
    DHLOGI("ShowIllealInfomation  Dump.");
    result.append("unrecognized option, -h for help");
}
} // namespace DistributedHardware
} // namespace OHOS