/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <cinttypes>
#include <dlfcn.h>
#include <fstream>
#include <string>

#include "config_policy_utils.h"

#include "anonymous_string.h"
#include "constants.h"
#include "dh_context.h"
#include "dh_utils_hitrace.h"
#include "dh_utils_hisysevent.h"
#include "dh_utils_tool.h"
#include "hidump_helper.h"
#include "distributed_hardware_log.h"
#include "version_info.h"
#include "version_info_manager.h"
#include "version_manager.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "ComponentLoader"

IMPLEMENT_SINGLE_INSTANCE(ComponentLoader);
using GetHardwareClass = IHardwareHandler *(*)();
using GetSourceHardwareClass = IDistributedHardwareSource *(*)();
using GetSinkHardwareClass = IDistributedHardwareSink *(*)();
namespace {
const std::string COMP_NAME = "name";
const std::string COMP_TYPE = "type";
const std::string COMP_HANDLER_LOC = "comp_handler_loc";
const std::string COMP_HANDLER_VERSION = "comp_handler_version";
const std::string COMP_SOURCE_LOC = "comp_source_loc";
const std::string COMP_SOURCE_VERSION = "comp_source_version";
const std::string COMP_SOURCE_SA_ID = "comp_source_sa_id";
const std::string COMP_SINK_LOC = "comp_sink_loc";
const std::string COMP_SINK_VERSION = "comp_sink_version";
const std::string COMP_SINK_SA_ID = "comp_sink_sa_id";
const std::string COMP_RESOURCE_DESC = "comp_resource_desc";
const std::string COMP_SUBTYPE = "subtype";
const std::string COMP_SENSITIVE = "sensitive";

const std::string COMPONENTSLOAD_DISTRIBUTED_COMPONENTS = "distributed_components";

const std::string DEFAULT_NAME = "";
const std::string DEFAULT_TYPE = "UNKNOWN";
const std::string DEFAULT_LOC = "";
const int32_t DEFAULT_SA_ID = -1;
const std::string DEFAULT_VERSION = "1.0";

std::map<std::string, DHType> g_mapDhTypeName = {
    { "UNKNOWN", DHType::UNKNOWN },
    { "CAMERA", DHType::CAMERA },
    { "AUDIO", DHType::AUDIO },
    { "SCREEN", DHType::SCREEN },
    { "GPS", DHType::GPS },
    { "INPUT", DHType::INPUT },
    { "HFP", DHType::HFP },
    { "A2D", DHType::A2D },
    { "VIRMODEM_AUDIO", DHType::VIRMODEM_AUDIO },
    { "MODEM", DHType::MODEM },
};

std::map<DHType, std::string> g_mapPartsParamName = {
    { DHType::CAMERA, "sys.dhfwk.component.dcamera.enable" },
    { DHType::AUDIO, "sys.dhfwk.component.daudio.enable" },
    { DHType::SCREEN, "sys.dhfwk.component.dscreen.enable" },
    { DHType::INPUT, "sys.dhfwk.component.dinput.enable" },
};
}

int32_t ComponentLoader::Init()
{
    DHLOGI("start");
    DHTraceStart(COMPONENT_LOAD_START);
    int32_t ret = ParseConfig();
    StoreLocalDHVersionInDB();
    DHTraceEnd();

    return ret;
}

std::vector<DHType> ComponentLoader::GetAllCompTypes()
{
    std::vector<DHType> DHTypeALL;
    for (std::map<DHType, CompHandler>::iterator it = compHandlerMap_.begin(); it != compHandlerMap_.end(); ++it) {
        DHTypeALL.push_back(it->first);
    }
    return DHTypeALL;
}

int32_t ParseComponent(const cJSON *json, CompConfig &cfg)
{
    if (!IsString(json, COMP_NAME)) {
        DHLOGE("COMP_NAME is invalid");
        return ERR_DH_FWK_JSON_PARSE_FAILED;
    }
    cfg.name = cJSON_GetObjectItem(json, COMP_NAME.c_str())->valuestring;
    if (!IsString(json, COMP_TYPE)) {
        DHLOGE("COMP_TYPE is invalid");
        return ERR_DH_FWK_JSON_PARSE_FAILED;
    }
    cfg.type = g_mapDhTypeName[cJSON_GetObjectItem(json, COMP_TYPE.c_str())->valuestring];
    if (!IsString(json, COMP_HANDLER_LOC)) {
        DHLOGE("COMP_HANDLER_LOC is invalid");
        return ERR_DH_FWK_JSON_PARSE_FAILED;
    }
    cfg.compHandlerLoc = cJSON_GetObjectItem(json, COMP_HANDLER_LOC.c_str())->valuestring;
    if (!IsString(json, COMP_HANDLER_VERSION)) {
        DHLOGE("COMP_HANDLER_VERSION is invalid");
        return ERR_DH_FWK_JSON_PARSE_FAILED;
    }
    cfg.compHandlerVersion = cJSON_GetObjectItem(json, COMP_HANDLER_VERSION.c_str())->valuestring;
    return DH_FWK_SUCCESS;
}

int32_t ParseSource(const cJSON *json, CompConfig &cfg)
{
    if (!IsString(json, COMP_SOURCE_LOC)) {
        DHLOGE("COMP_SOURCE_LOC is invalid");
        return ERR_DH_FWK_JSON_PARSE_FAILED;
    }
    cfg.compSourceLoc = cJSON_GetObjectItem(json, COMP_SOURCE_LOC.c_str())->valuestring;
    if (!IsString(json, COMP_SOURCE_VERSION)) {
        DHLOGE("COMP_SOURCE_VERSION is invalid");
        return ERR_DH_FWK_JSON_PARSE_FAILED;
    }
    cfg.compSourceVersion = cJSON_GetObjectItem(json, COMP_SOURCE_VERSION.c_str())->valuestring;
    if (!IsInt32(json, COMP_SOURCE_SA_ID)) {
        DHLOGE("COMP_SOURCE_SA_ID is invalid");
        return ERR_DH_FWK_JSON_PARSE_FAILED;
    }
    cfg.compSourceSaId = static_cast<int32_t>(cJSON_GetObjectItem(json, COMP_SOURCE_SA_ID.c_str())->valuedouble);
    return DH_FWK_SUCCESS;
}

int32_t ParseSink(const cJSON *json, CompConfig &cfg)
{
    if (!IsString(json, COMP_SINK_LOC)) {
        DHLOGE("COMP_SINK_LOC is invalid");
        return ERR_DH_FWK_JSON_PARSE_FAILED;
    }
    cfg.compSinkLoc = cJSON_GetObjectItem(json, COMP_SINK_LOC.c_str())->valuestring;
    if (!IsString(json, COMP_SINK_VERSION)) {
        DHLOGE("COMP_SINK_VERSION is invalid");
        return ERR_DH_FWK_JSON_PARSE_FAILED;
    }
    cfg.compSinkVersion = cJSON_GetObjectItem(json, COMP_SINK_VERSION.c_str())->valuestring;
    if (!IsInt32(json, COMP_SINK_SA_ID)) {
        DHLOGE("COMP_SINK_SA_ID is invalid");
        return ERR_DH_FWK_JSON_PARSE_FAILED;
    }
    cfg.compSinkSaId = static_cast<int32_t>(cJSON_GetObjectItem(json, COMP_SINK_SA_ID.c_str())->valuedouble);
    return DH_FWK_SUCCESS;
}

int32_t ParseResourceDesc(const cJSON *json, CompConfig &cfg)
{
    if (!IsArray(json, COMP_RESOURCE_DESC)) {
        DHLOGE("COMP_RESOURCE_DESC is invalid");
        return ERR_DH_FWK_JSON_PARSE_FAILED;
    }
    cJSON *resourceDescArray = cJSON_GetObjectItem(json, COMP_RESOURCE_DESC.c_str());
    cJSON *element;
    cJSON_ArrayForEach(element, resourceDescArray) {
        ResourceDesc desc;
        if (!IsString(element, COMP_SUBTYPE)) {
            DHLOGE("COMP_SUBTYPE is invalid");
            return ERR_DH_FWK_JSON_PARSE_FAILED;
        }
        desc.subtype = cJSON_GetObjectItem(element, COMP_SUBTYPE.c_str())->valuestring;
        cJSON *sensitive = cJSON_GetObjectItem(element, COMP_SENSITIVE.c_str());
        if (cJSON_IsTrue(sensitive)) {
            desc.sensitiveValue = true;
        } else {
            desc.sensitiveValue = false;
        }
        cfg.compResourceDesc.push_back(desc);
    }
    return DH_FWK_SUCCESS;
}

void from_json(const cJSON *json, CompConfig &cfg)
{
    if (ParseComponent(json, cfg) != DH_FWK_SUCCESS) {
        DHLOGE("ParseComponent is failed");
        return;
    }
    if (ParseSource(json, cfg) != DH_FWK_SUCCESS) {
        DHLOGE("ParseSource is failed");
        return;
    }
    if (ParseSink(json, cfg) != DH_FWK_SUCCESS) {
        DHLOGE("ParseSink is failed");
        return;
    }
    if (ParseResourceDesc(json, cfg) != DH_FWK_SUCCESS) {
        DHLOGE("ParseResourceDesc is failed");
        return;
    }
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

bool ComponentLoader::CheckComponentEnable(const CompConfig &config)
{
    auto item = g_mapPartsParamName.find(config.type);
    if (item == g_mapPartsParamName.end()) {
        DHLOGI("Crrent component is enabled by default.");
        return true;
    }
    bool isEnable = false;
    if (!GetSysPara((item->second).c_str(), isEnable)) {
        DHLOGE("sys para: %{public}s get failed.", (item->second).c_str());
        return false;
    }
    DHLOGI("Component type: %{public}u, enable flag: %{public}d.", config.type, isEnable);
    return isEnable;
}

int32_t ComponentLoader::GetCompPathAndVersion(const std::string &jsonStr, std::map<DHType, CompConfig> &dhtypeMap)
{
    cJSON *root = cJSON_Parse(jsonStr.c_str());
    if (root == NULL) {
        DHLOGE("jsonStr parse failed");
        return ERR_DH_FWK_JSON_PARSE_FAILED;
    }
    if (!IsArray(root, COMPONENTSLOAD_DISTRIBUTED_COMPONENTS)) {
        DHLOGE("distributed_components is not an array");
        cJSON_Delete(root);
        return ERR_DH_FWK_PARA_INVALID;
    }
    cJSON *components = cJSON_GetObjectItem(root, COMPONENTSLOAD_DISTRIBUTED_COMPONENTS.c_str());

    size_t compSize = static_cast<size_t>(cJSON_GetArraySize(components));
    if (compSize == 0 || compSize > MAX_COMP_SIZE) {
        DHLOGE("CompConfig size is invalid!");
        cJSON_Delete(root);
        return ERR_DH_FWK_PARA_INVALID;
    }
    cJSON *component;
    cJSON_ArrayForEach(component, components) {
        CompConfig config;
        ParseCompConfigFromJson(component, config);
        dhtypeMap.insert(std::pair<DHType, CompConfig>(config.type, config));
        localDHVersion_.compVersions.insert(
            std::pair<DHType, CompVersion>(config.type, GetCompVersionFromComConfig(config)));
    }
    cJSON_Delete(root);
    isLocalVersionInit_.store(true);
    return DH_FWK_SUCCESS;
}

void ComponentLoader::ParseCompConfigFromJson(cJSON *component, CompConfig &config)
{
    if (IsString(component, COMP_NAME.c_str())) {
        config.name = cJSON_GetObjectItem(component, COMP_NAME.c_str())->valuestring;
    }
    if (IsString(component, COMP_TYPE.c_str())) {
        config.type = g_mapDhTypeName[cJSON_GetObjectItem(component, COMP_TYPE.c_str())->valuestring];
    }
    if (IsString(component, COMP_HANDLER_LOC.c_str())) {
        config.compHandlerLoc = cJSON_GetObjectItem(component, COMP_HANDLER_LOC.c_str())->valuestring;
    }
    if (IsString(component, COMP_HANDLER_VERSION.c_str())) {
        config.compHandlerVersion = cJSON_GetObjectItem(component, COMP_HANDLER_VERSION.c_str())->valuestring;
    }
    if (IsString(component, COMP_SOURCE_LOC.c_str())) {
        config.compSourceLoc = cJSON_GetObjectItem(component, COMP_SOURCE_LOC.c_str())->valuestring;
    }
    if (IsString(component, COMP_SOURCE_VERSION.c_str())) {
        config.compSourceVersion = cJSON_GetObjectItem(component, COMP_SOURCE_VERSION.c_str())->valuestring;
    }
    if (IsInt32(component, COMP_SOURCE_SA_ID.c_str())) {
        config.compSourceSaId =
            static_cast<int32_t>(cJSON_GetObjectItem(component, COMP_SOURCE_SA_ID.c_str())->valuedouble);
    }
    if (IsString(component, COMP_SINK_LOC.c_str())) {
        config.compSinkLoc = cJSON_GetObjectItem(component, COMP_SINK_LOC.c_str())->valuestring;
    }
    if (IsString(component, COMP_SINK_VERSION.c_str())) {
        config.compSinkVersion = cJSON_GetObjectItem(component, COMP_SINK_VERSION.c_str())->valuestring;
    }
    if (IsInt32(component, COMP_SINK_SA_ID.c_str())) {
        config.compSinkSaId =
            static_cast<int32_t>(cJSON_GetObjectItem(component, COMP_SINK_SA_ID.c_str())->valuedouble);
    }
    if (IsArray(component, COMP_RESOURCE_DESC.c_str())) {
        cJSON *resourceDescs = cJSON_GetObjectItem(component, COMP_RESOURCE_DESC.c_str());
        ParseResourceDescFromJson(resourceDescs, config);
    }
}

void ComponentLoader::ParseResourceDescFromJson(cJSON *resourceDescs, CompConfig &config)
{
    cJSON *resourceDesc;
    cJSON_ArrayForEach(resourceDesc, resourceDescs) {
        bool sensitiveValue;
        cJSON *sensitive = cJSON_GetObjectItem(resourceDesc, COMP_SENSITIVE.c_str());
        if (cJSON_IsTrue(sensitive)) {
            sensitiveValue = true;
        } else {
            sensitiveValue = false;
        }
        ResourceDesc resource;
        if (!IsString(resourceDesc, COMP_SUBTYPE)) {
            DHLOGE("COMP_SUBTYPE is invalid");
            return;
        }
        resource.subtype = cJSON_GetObjectItem(resourceDesc, COMP_SUBTYPE.c_str())->valuestring;
        resource.sensitiveValue = sensitiveValue;
        config.compResourceDesc.push_back(resource);
    }
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

void ComponentLoader::StoreLocalDHVersionInDB()
{
    if (!isLocalVersionInit_.load()) {
        DHLOGE("Store local DHVersion fail");
        return;
    }
    VersionInfo versionInfo;
    versionInfo.dhVersion = VersionManager::GetInstance().GetLocalDeviceVersion();
    versionInfo.deviceId = DHContext::GetInstance().GetDeviceInfo().deviceId;
    versionInfo.compVersions = localDHVersion_.compVersions;
    VersionInfoManager::GetInstance()->AddVersion(versionInfo);
}

void *ComponentLoader::GetHandler(const std::string &soName)
{
    if (soName.length() == 0 || soName.length() > PATH_MAX) {
        DHLOGE("File canonicalization failed, soName: %{public}s", soName.c_str());
        return nullptr;
    }
    void *pHandler = dlopen(soName.c_str(), RTLD_LAZY | RTLD_NODELETE);
    if (pHandler == nullptr) {
        DHLOGE("so: %{public}s load failed, failed reason: %{public}s", soName.c_str(), dlerror());
        HiSysEventWriteMsg(DHFWK_INIT_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            "dhfwk so open failed, soname : " + soName);
        return nullptr;
    }
    return pHandler;
}

void ComponentLoader::GetAllHandler(std::map<DHType, CompConfig> &dhtypeMap)
{
    std::map<DHType, CompConfig>::iterator itor;
    for (itor = dhtypeMap.begin(); itor != dhtypeMap.end(); ++itor) {
        CompHandler comHandler;
        comHandler.type = itor->second.type;
        comHandler.hardwareHandler = GetHandler(itor->second.compHandlerLoc);
        comHandler.sourceHandler = GetHandler(itor->second.compSourceLoc);
        comHandler.sourceSaId = itor->second.compSourceSaId;
        comHandler.sinkHandler = GetHandler(itor->second.compSinkLoc);
        comHandler.sinkSaId = itor->second.compSinkSaId;
        std::vector<ResourceDesc> compResourceDesc = itor->second.compResourceDesc;
        for (auto it = compResourceDesc.begin(); it != compResourceDesc.end(); it++) {
            resDescMap_[it->subtype] = it->sensitiveValue;
        }
        comHandler.resourceDesc = itor->second.compResourceDesc;
        compHandlerMap_[itor->second.type] = comHandler;
    }
}

int32_t ComponentLoader::GetHardwareHandler(const DHType dhType, IHardwareHandler *&hardwareHandlerPtr)
{
    if (compHandlerMap_.find(dhType) == compHandlerMap_.end()) {
        DHLOGE("DHType not exist, dhType: %{public}" PRIu32, (uint32_t)dhType);
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }

    if (compHandlerMap_[dhType].hardwareHandler == nullptr) {
        DHLOGE("hardwareHandler is null.");
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }

    GetHardwareClass getHardwareClassHandler = (GetHardwareClass)dlsym(compHandlerMap_[dhType].hardwareHandler,
        COMPONENT_LOADER_GET_HARDWARE_HANDLER.c_str());
    if (getHardwareClassHandler == nullptr) {
        DHLOGE("get getHardwareClassHandler is null, failed reason : %{public}s", dlerror());
        dlclose(compHandlerMap_[dhType].hardwareHandler);
        compHandlerMap_[dhType].hardwareHandler = nullptr;
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }
    hardwareHandlerPtr = getHardwareClassHandler();
    return DH_FWK_SUCCESS;
}

int32_t ComponentLoader::GetSource(const DHType dhType, IDistributedHardwareSource *&sourcePtr)
{
    if (compHandlerMap_.find(dhType) == compHandlerMap_.end()) {
        DHLOGE("DHType not exist, dhType: %{public}" PRIu32, (uint32_t)dhType);
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }

    if (compHandlerMap_[dhType].sourceHandler == nullptr) {
        DHLOGE("sourceHandler is null.");
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }

    GetSourceHardwareClass getSourceHardClassHandler = (GetSourceHardwareClass)dlsym(
        compHandlerMap_[dhType].sourceHandler, COMPONENT_LOADER_GET_SOURCE_HANDLER.c_str());
    if (getSourceHardClassHandler == nullptr) {
        DHLOGE("get getSourceHardClassHandler is null, failed reason : %{public}s", dlerror());
        dlclose(compHandlerMap_[dhType].sourceHandler);
        compHandlerMap_[dhType].sourceHandler = nullptr;
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }
    sourcePtr = getSourceHardClassHandler();
    return DH_FWK_SUCCESS;
}

int32_t ComponentLoader::GetSink(const DHType dhType, IDistributedHardwareSink *&sinkPtr)
{
    if (compHandlerMap_.find(dhType) == compHandlerMap_.end()) {
        DHLOGE("DHType not exist, dhType: %{public}" PRIu32, (uint32_t)dhType);
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }

    if (compHandlerMap_[dhType].sinkHandler == nullptr) {
        DHLOGE("sinkHandler is null.");
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }

    GetSinkHardwareClass getSinkHardwareClassHandler =
        (GetSinkHardwareClass)dlsym(compHandlerMap_[dhType].sinkHandler, COMPONENT_LOADER_GET_SINK_HANDLER.c_str());
    if (getSinkHardwareClassHandler == nullptr) {
        DHLOGE("get getSinkHardwareClassHandler is null, failed reason : %{public}s", dlerror());
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
        DHLOGE("filePath: %{public}s Readfile fail", filePath.c_str());
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
    char buf[MAX_PATH_LEN] = {0};
    char path[PATH_MAX + 1] = {0x00};
    char *profilePath = GetOneCfgFile(COMPONENTSLOAD_PROFILE_PATH, buf, MAX_PATH_LEN);
    if (profilePath == nullptr) {
        DHLOGE("profilePath is null.");
        return ERR_DH_FWK_LOADER_PROFILE_PATH_IS_NULL;
    }

    if (strlen(profilePath) == 0 || strlen(profilePath) > PATH_MAX || realpath(profilePath, path) == nullptr) {
        std::string comProfilePath(profilePath);
        DHLOGE("File connicailization failed, comProfilePath: %{public}s.", GetAnonyString(comProfilePath).c_str());
        return ERR_DH_FWK_LOADER_PROFILE_PATH_IS_NULL;
    }
    std::string componentProfilePath(path);
    std::string jsonStr = Readfile(componentProfilePath);
    if (jsonStr.length() == 0 || jsonStr.size() > MAX_MESSAGE_LEN) {
        DHLOGE("ConfigJson size is invalid!");
        return ERR_DH_FWK_LOADER_CONFIG_JSON_INVALID;
    }
    ret = GetCompPathAndVersion(jsonStr, dhtypeMap);
    if (ret != DH_FWK_SUCCESS) {
        return ret;
    }
    GetAllHandler(dhtypeMap);
    return DH_FWK_SUCCESS;
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
    DHTraceStart(COMPONENT_RELEASE_START);
    int32_t ret = DH_FWK_SUCCESS;
    for (std::map<DHType, CompHandler>::iterator iter = compHandlerMap_.begin();
        iter != compHandlerMap_.end(); ++iter) {
        ret += ReleaseHardwareHandler(iter->first);
        ret += ReleaseSource(iter->first);
        ret += ReleaseSink(iter->first);
    }
    compHandlerMap_.clear();
    resDescMap_.clear();
    DHTraceEnd();
    return ret;
}

int32_t ComponentLoader::ReleaseHardwareHandler(const DHType dhType)
{
    if (!IsDHTypeExist(dhType)) {
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }
    int32_t ret = ReleaseHandler(compHandlerMap_[dhType].hardwareHandler);
    if (ret) {
        DHLOGE("fail, dhType: %{public}#X", dhType);
        HiSysEventWriteReleaseMsg(DHFWK_RELEASE_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            dhType, ret, "dhfwk release hardware handler failed.");
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
        DHLOGE("fail, dhType: %{public}#X", dhType);
        HiSysEventWriteReleaseMsg(DHFWK_RELEASE_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            dhType, ret, "dhfwk release source failed.");
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
        DHLOGE("fail, dhType: %{public}#X", dhType);
        HiSysEventWriteReleaseMsg(DHFWK_RELEASE_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            dhType, ret, "dhfwk release sink failed.");
    }
    return ret;
}

bool ComponentLoader::IsDHTypeExist(DHType dhType)
{
    if (compHandlerMap_.find(dhType) == compHandlerMap_.end()) {
        DHLOGE("fail, dhType: %{public}#X not exist", dhType);
        return false;
    }
    return true;
}

int32_t ComponentLoader::GetSourceSaId(const DHType dhType)
{
    if (compHandlerMap_.find(dhType) == compHandlerMap_.end()) {
        DHLOGE("DHType not exist, dhType: %{public}" PRIu32, (uint32_t)dhType);
        return DEFAULT_SA_ID;
    }
    return compHandlerMap_[dhType].sourceSaId;
}

DHType ComponentLoader::GetDHTypeBySrcSaId(const int32_t saId)
{
    DHType type = DHType::UNKNOWN;
    for (const auto &handler : compHandlerMap_) {
        if (handler.second.sourceSaId == saId) {
            type = handler.second.type;
            break;
        }
    }
    return type;
}

std::map<std::string, bool> ComponentLoader::GetCompResourceDesc()
{
    return resDescMap_;
}
} // namespace DistributedHardware
} // namespace OHOS
