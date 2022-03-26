/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "componentloader/component_loader.h"

#include <dlfcn.h>
#include <fstream>
#include <string>

#include "nlohmann/json.hpp"

#include "constants.h"
#include "distributed_hardware_log.h"

using nlohmann::json;

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "ComponentLoader"

IMPLEMENT_SINGLE_INSTANCE(ComponentLoader);
using GetHardwareClass = IHardwareHandler *(*)();
using GetSourceHardwareClass = IDistributedHardwareSource *(*)();
using GetSinkHardwareClass = IDistributedHardwareSink *(*)();
namespace {
const std::string DEFAULT_NAME = "";
const std::string DEFAULT_HANDLER_LOC = "";
const std::string DEFAULT_SOURCE_LOC = "";
const std::string DEFAULT_SINK_LOC = "";
const std::string DEFAULT_TYPE = "UNKNOWN";
const std::string DEFAULT_VERSION = "1.0";
}
std::map<std::string, DHType> g_mapDhTypeName = {
    { "UNKNOWN", DHType::UNKNOWN },
    { "CAMERA", DHType::CAMERA },
    { "MIC", DHType::MIC },
    { "SPEAKER", DHType::SPEAKER },
    { "DISPLAY", DHType::DISPLAY },
    { "GPS", DHType::GPS },
    { "BUTTON", DHType::BUTTON },
    { "HFP", DHType::HFP },
    { "A2D", DHType::A2D },
    { "VIRMODEM_MIC", DHType::VIRMODEM_MIC },
    { "VIRMODEM_SPEAKER", DHType::VIRMODEM_SPEAKER },
};

int32_t ComponentLoader::Init()
{
    return ParseConfig();
}

std::vector<DHType> ComponentLoader::GetAllCompTypes()
{
    std::vector<DHType> DHTypeALL;
    for (std::map<DHType, CompHandler>::iterator it = compHandlerMap_.begin(); it != compHandlerMap_.end(); it++) {
        DHTypeALL.push_back(it->first);
    }
    return DHTypeALL;
}

void from_json(const nlohmann::json &j, CompConfig &cCfg)
{
    cCfg.name = j.value("name", DEFAULT_NAME);
    cCfg.type = g_mapDhTypeName[j.value("type", DEFAULT_TYPE)];
    cCfg.compHandlerLoc = j.value("comp_handler_loc", DEFAULT_HANDLER_LOC);
    cCfg.compHandlerVersion = j.value("comp_handler_version", DEFAULT_VERSION);
    cCfg.compSourceLoc = j.value("comp_source_loc", DEFAULT_SOURCE_LOC);
    cCfg.compSourceVersion = j.value("comp_source_version", DEFAULT_VERSION);
    cCfg.compSinkLoc = j.value("comp_sink_loc", DEFAULT_SINK_LOC);
    cCfg.compSinkVersion = j.value("comp_sink_version", DEFAULT_VERSION);
}

CompVersion ComponentLoader::GetCompVersionFromComConfig(const CompConfig& cCfg)
{
    CompVersion compVersions;
    compVersions.dhType = cCfg.type;
    compVersions.name = cCfg.name;
    compVersions.handlerVersion = cCfg.compHandlerVersion;
    compVersions.sinkVersion = cCfg.compSinkVersion;
    compVersions.sourceVersion = cCfg.compSourceVersion;
    return compVersions;
}

int32_t ComponentLoader::GetCompPathAndVersion(const std::string &jsonStr, std::map<DHType, CompConfig> &dhtypeMap)
{
    auto jsonCfg = json::parse(jsonStr);
    if (jsonCfg.find(COMPONENTSLOAD_DISTRIBUTED_COMPONENTS) == jsonCfg.end()) {
        DHLOGE("not find distributed_components");
        return ERR_DH_FWK_PARA_INVALID;
    }

    std::vector<CompConfig> vecJsnCfg =
        jsonCfg.at(COMPONENTSLOAD_DISTRIBUTED_COMPONENTS).get<std::vector<CompConfig>>();
    DHLOGI("get distributed_components CompConfig size is %d", vecJsnCfg.size());
    for (std::vector<CompConfig>::iterator iter = vecJsnCfg.begin(); iter != vecJsnCfg.end(); iter++) {
        dhtypeMap.insert(std::pair<DHType, CompConfig>((*iter).type, (*iter)));
        localDHVersion_.compVersions.insert(
            std::pair<DHType, CompVersion>((*iter).type, GetCompVersionFromComConfig(*iter)));
    }
    isLocalVersionInit_.store(true);
    return DH_FWK_SUCCESS;
}

int32_t ComponentLoader::GetLocalDHVersion(DHVersion &dhVersion)
{
    if (!isLocalVersionInit_.load()) {
        DHLOGE("get local DHVersion fail");
        return ERR_DH_FWK_LOADER_GET_LOCAL_VERSION_FAIL;
    }
    dhVersion = localDHVersion_;
    return DH_FWK_SUCCESS;
}

void *ComponentLoader::GetHandler(const std::string &soName)
{
    if (soName.length() <= 0) {
        DHLOGE("%s soName length is 0", soName.c_str());
        return nullptr;
    }
    char path[PATH_MAX + 1] = {0x00};
    if (soName.length() == 0 || soName.length() > PATH_MAX || realpath(soName.c_str(), path) == nullptr) {
        DHLOGE("File canonicalization failed");
        return nullptr;
    }
    void *pHandler = dlopen(path, RTLD_LAZY | RTLD_NODELETE);
    if (pHandler == nullptr) {
        DHLOGE("%s handler load failed.", path);
        return nullptr;
    }
    return pHandler;
}

void ComponentLoader::GetAllHandler(std::map<DHType, CompConfig> &dhtypeMap)
{
    std::map<DHType, CompConfig>::iterator itor;
    for (itor = dhtypeMap.begin(); itor != dhtypeMap.end(); itor++) {
        CompHandler comHandler;
        comHandler.hardwareHandler = GetHandler(itor->second.compHandlerLoc);
        comHandler.sinkHandler = GetHandler(itor->second.compSinkLoc);
        comHandler.sourceHandler = GetHandler(itor->second.compSourceLoc);
        compHandlerMap_.insert(std::pair<DHType, CompHandler>(itor->second.type, comHandler));
    }
}

int32_t ComponentLoader::GetHardwareHandler(const DHType dhType, IHardwareHandler *&hardwareHandlerPtr)
{
    if (compHandlerMap_[dhType].hardwareHandler == nullptr) {
        DHLOGE("hardwareHandler is null.");
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }

    GetHardwareClass getHardwareClassHandler = (GetHardwareClass)dlsym(compHandlerMap_[dhType].hardwareHandler,
        COMPONENT_LOADER_GET_HARDWARE_HANDLER.c_str());
    if (getHardwareClassHandler == nullptr) {
        DHLOGE("get getHardwareClassHandler is null.");
        dlclose(compHandlerMap_[dhType].hardwareHandler);
        compHandlerMap_[dhType].hardwareHandler = nullptr;
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }
    hardwareHandlerPtr = getHardwareClassHandler();
    return DH_FWK_SUCCESS;
}

int32_t ComponentLoader::GetSource(const DHType dhType, IDistributedHardwareSource *&sourcePtr)
{
    if (compHandlerMap_[dhType].sourceHandler == nullptr) {
        DHLOGE("sourceHandler is null.");
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }

    GetSourceHardwareClass getSourceHardClassHandler = (GetSourceHardwareClass)dlsym(
        compHandlerMap_[dhType].sourceHandler, COMPONENT_LOADER_GET_SOURCE_HANDLER.c_str());
    if (getSourceHardClassHandler == nullptr) {
        DHLOGE("get getSourceHardClassHandler is null.");
        dlclose(compHandlerMap_[dhType].sourceHandler);
        compHandlerMap_[dhType].sourceHandler = nullptr;
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }
    sourcePtr = getSourceHardClassHandler();
    return DH_FWK_SUCCESS;
}

int32_t ComponentLoader::GetSink(const DHType dhType, IDistributedHardwareSink *&sinkPtr)
{
    if (compHandlerMap_[dhType].sinkHandler == nullptr) {
        DHLOGE("sinkHandler is null.");
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }

    GetSinkHardwareClass getSinkHardwareClassHandler =
        (GetSinkHardwareClass)dlsym(compHandlerMap_[dhType].sinkHandler, COMPONENT_LOADER_GET_SINK_HANDLER.c_str());
    if (getSinkHardwareClassHandler == nullptr) {
        DHLOGE("get getSinkHardwareClassHandler is null.");
        dlclose(compHandlerMap_[dhType].sinkHandler);
        compHandlerMap_[dhType].sinkHandler = nullptr;
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }
    sinkPtr = getSinkHardwareClassHandler();
    return DH_FWK_SUCCESS;
}

std::string ComponentLoader::Readfile(const std::string &filePath)
{
    std::ifstream infile;
    std::string sLine;
    std::string sAll = "";
    infile.open(filePath);
    if (!infile.is_open()) {
        DHLOGE("filePath: %s Readfile fail", filePath.c_str());
        return sAll;
    }

    while (getline(infile, sLine)) {
        sAll.append(sLine);
    }
    infile.close();
    return sAll;
}

int32_t ComponentLoader::ParseConfig()
{
    std::map<DHType, CompConfig> dhtypeMap;
    int32_t ret;
    DHLOGI("ParseConfig start");
    std::string jsonStr = Readfile(COMPONENTSLOAD_PROFILE_PATH);
    if (jsonStr.length() == 0) {
        DHLOGE("profile is empty return");
        return ERR_DH_FWK_LOADER_COMPONENT_PROFILE_IS_EMPTY;
    }
    ret = GetCompPathAndVersion(jsonStr, dhtypeMap);
    GetAllHandler(dhtypeMap);
    return ret;
}

int32_t ComponentLoader::ReleaseHandler(void *&handler)
{
    if (handler == nullptr) {
        DHLOGE("handler is null.");
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }

    if (dlclose(handler) != 0) {
        DHLOGE("dlclose failed.");
        return ERR_DH_FWK_LOADER_DLCLOSE_FAIL;
    }
    handler = nullptr;
    return DH_FWK_SUCCESS;
}

int32_t ComponentLoader::UnInit()
{
    DHLOGI("release all handler");
    int32_t ret = DH_FWK_SUCCESS;
    for (std::map<DHType, CompHandler>::iterator iter = compHandlerMap_.begin();
        iter != compHandlerMap_.end(); iter++) {
        ret += ReleaseHardwareHandler(iter->first);
        ret += ReleaseSource(iter->first);
        ret += ReleaseSink(iter->first);
    }
    compHandlerMap_.clear();
    return ret;
}

int32_t ComponentLoader::ReleaseHardwareHandler(const DHType dhType)
{
    if (!IsDHTypeExist(dhType)) {
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }
    int32_t ret = ReleaseHandler(compHandlerMap_[dhType].hardwareHandler);
    if (ret) {
        DHLOGE("fail, dhType: %#X", dhType);
    }
    return ret;
}

int32_t ComponentLoader::ReleaseSource(const DHType dhType)
{
    if (!IsDHTypeExist(dhType)) {
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }
    int32_t ret = ReleaseHandler(compHandlerMap_[dhType].sourceHandler);
    if (ret) {
        DHLOGE("fail, dhType: %#X", dhType);
    }
    return ret;
}

int32_t ComponentLoader::ReleaseSink(const DHType dhType)
{
    if (!IsDHTypeExist(dhType)) {
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }
    int32_t ret = ReleaseHandler(compHandlerMap_[dhType].sinkHandler);
    if (ret) {
        DHLOGE("fail, dhType: %#X", dhType);
    }
    return ret;
}

bool ComponentLoader::IsDHTypeExist(DHType dhType)
{
    if (compHandlerMap_.find(dhType) == compHandlerMap_.end()) {
        DHLOGE("fail, dhType: %#X not exist", dhType);
        return false;
    }
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS
