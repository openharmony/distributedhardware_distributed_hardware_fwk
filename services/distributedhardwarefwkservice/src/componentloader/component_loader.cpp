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
constexpr const char *COMP_NAME = "name";
constexpr const char *COMP_TYPE = "type";
constexpr const char *COMP_HANDLER_LOC = "comp_handler_loc";
constexpr const char *COMP_HANDLER_VERSION = "comp_handler_version";
constexpr const char *COMP_SOURCE_LOC = "comp_source_loc";
constexpr const char *COMP_SOURCE_VERSION = "comp_source_version";
constexpr const char *COMP_SOURCE_SA_ID = "comp_source_sa_id";
constexpr const char *COMP_SINK_LOC = "comp_sink_loc";
constexpr const char *COMP_SINK_VERSION = "comp_sink_version";
constexpr const char *COMP_SINK_SA_ID = "comp_sink_sa_id";
constexpr const char *COMP_RESOURCE_DESC = "comp_resource_desc";
constexpr const char *COMP_SUBTYPE = "subtype";
constexpr const char *COMP_SENSITIVE = "sensitive";
constexpr const char *COMPONENTSLOAD_DISTRIBUTED_COMPONENTS = "distributed_components";

constexpr int32_t DEFAULT_SA_ID = -1;
constexpr uint32_t MAX_COMP_SIZE = 128;

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

void ComponentLoader::GetAllCompTypes(std::vector<DHType> &dhTypeVec)
{
    std::lock_guard<std::mutex> lock(compHandlerMapMutex_);
    for (std::map<DHType, CompHandler>::iterator it = compHandlerMap_.begin(); it != compHandlerMap_.end(); ++it) {
        dhTypeVec.push_back(it->first);
    }
}

void ComponentLoader::GetCompVersionFromComConfig(const CompConfig &comCfg, CompVersion &compVersion)
{
    compVersion.dhType = comCfg.type;
    compVersion.name = comCfg.name;
    compVersion.handlerVersion = comCfg.compHandlerVersion;
    compVersion.sinkVersion = comCfg.compSinkVersion;
    compVersion.sourceVersion = comCfg.compSourceVersion;
    compVersion.haveFeature = comCfg.haveFeature;
    compVersion.sourceFeatureFilters = comCfg.sourceFeatureFilters;
    compVersion.sinkSupportedFeatures = comCfg.sinkSupportedFeatures;
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
    if (!IsJsonLengthValid(jsonStr)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    cJSON *root = cJSON_Parse(jsonStr.c_str());
    if (root == NULL) {
        DHLOGE("jsonStr parse failed");
        return ERR_DH_FWK_JSON_PARSE_FAILED;
    }
    cJSON *components = cJSON_GetObjectItem(root, COMPONENTSLOAD_DISTRIBUTED_COMPONENTS);
    if (!IsArray(components)) {
        DHLOGE("distributed_components is not an array");
        cJSON_Delete(root);
        return ERR_DH_FWK_PARA_INVALID;
    }

    size_t compSize = static_cast<size_t>(cJSON_GetArraySize(components));
    if (compSize == 0 || compSize > MAX_COMP_SIZE) {
        DHLOGE("CompConfig size is invalid!");
        cJSON_Delete(root);
        return ERR_DH_FWK_PARA_INVALID;
    }
    cJSON *component = nullptr;
    cJSON_ArrayForEach(component, components) {
        CompConfig config;
        ParseCompConfigFromJson(component, config);
        dhtypeMap.insert(std::pair<DHType, CompConfig>(config.type, config));
        CompVersion compVersion;
        GetCompVersionFromComConfig(config, compVersion);
        localDHVersion_.compVersions.insert(std::pair<DHType, CompVersion>(config.type, compVersion));
    }
    cJSON_Delete(root);
    isLocalVersionInit_.store(true);
    return DH_FWK_SUCCESS;
}

void ComponentLoader::ParseCompConfigFromJson(cJSON *component, CompConfig &config)
{
    cJSON *nameJson = cJSON_GetObjectItem(component, COMP_NAME);
    if (IsString(nameJson)) {
        config.name = nameJson->valuestring;
    }
    cJSON *typeJson = cJSON_GetObjectItem(component, COMP_TYPE);
    if (IsString(typeJson)) {
        config.type = g_mapDhTypeName[typeJson->valuestring];
    }
    cJSON *handlerLocJson = cJSON_GetObjectItem(component, COMP_HANDLER_LOC);
    if (IsString(handlerLocJson)) {
        config.compHandlerLoc = handlerLocJson->valuestring;
    }
    cJSON *handlerVerJson = cJSON_GetObjectItem(component, COMP_HANDLER_VERSION);
    if (IsString(handlerVerJson)) {
        config.compHandlerVersion = handlerVerJson->valuestring;
    }
    cJSON *sourceLocJson = cJSON_GetObjectItem(component, COMP_SOURCE_LOC);
    if (IsString(sourceLocJson)) {
        config.compSourceLoc = sourceLocJson->valuestring;
    }
    cJSON *sourceVerJson = cJSON_GetObjectItem(component, COMP_SOURCE_VERSION);
    if (IsString(sourceVerJson)) {
        config.compSourceVersion = sourceVerJson->valuestring;
    }
    cJSON *sourceSaIdJson = cJSON_GetObjectItem(component, COMP_SOURCE_SA_ID);
    if (IsInt32(sourceSaIdJson)) {
        config.compSourceSaId = static_cast<int32_t>(sourceSaIdJson->valueint);
    }
    cJSON *sinkLocJson = cJSON_GetObjectItem(component, COMP_SINK_LOC);
    if (IsString(sinkLocJson)) {
        config.compSinkLoc = sinkLocJson->valuestring;
    }
    cJSON *sinkVerJson = cJSON_GetObjectItem(component, COMP_SINK_VERSION);
    if (IsString(sinkVerJson)) {
        config.compSinkVersion = sinkVerJson->valuestring;
    }
    cJSON *sinkSaIdJson = cJSON_GetObjectItem(component, COMP_SINK_SA_ID);
    if (IsInt32(sinkSaIdJson)) {
        config.compSinkSaId = static_cast<int32_t>(sinkSaIdJson->valueint);
    }
    cJSON *resourceDescs = cJSON_GetObjectItem(component, COMP_RESOURCE_DESC);
    if (IsArray(resourceDescs)) {
        ParseResourceDescFromJson(resourceDescs, config);
    }
    CheckAndParseFeatures(component, config);
}

void ComponentLoader::ParseResourceDescFromJson(cJSON *resourceDescs, CompConfig &config)
{
    cJSON *resourceDesc = nullptr;
    cJSON_ArrayForEach(resourceDesc, resourceDescs) {
        bool sensitiveValue;
        cJSON *sensitive = cJSON_GetObjectItem(resourceDesc, COMP_SENSITIVE);
        if (!IsBool(sensitive)) {
            DHLOGE("COMP_SUBTYPE is invalid!");
            return;
        }
        if (cJSON_IsTrue(sensitive)) {
            sensitiveValue = true;
        } else {
            sensitiveValue = false;
        }
        ResourceDesc resource;
        cJSON *subtypeJson = cJSON_GetObjectItem(resourceDesc, COMP_SUBTYPE);
        if (!IsString(subtypeJson)) {
            DHLOGE("COMP_SUBTYPE is invalid!");
            return;
        }
        resource.subtype = subtypeJson->valuestring;
        resource.sensitiveValue = sensitiveValue;
        config.compResourceDesc.push_back(resource);
    }
}

void ComponentLoader::ParseSourceFeatureFiltersFromJson(cJSON *sourceFeatureFilters, CompConfig &config)
{
    cJSON *filter = nullptr;
    config.sourceFeatureFilters.clear();
    cJSON_ArrayForEach(filter, sourceFeatureFilters) {
        if (filter != nullptr && filter->type == cJSON_String) {
            config.sourceFeatureFilters.push_back(std::string(filter->valuestring));
        }
    }
}

void ComponentLoader::ParseSinkSupportedFeaturesFromJson(cJSON *sinkSupportedFeatures, CompConfig &config)
{
    cJSON *feature = nullptr;
    config.sinkSupportedFeatures.clear();
    cJSON_ArrayForEach(feature, sinkSupportedFeatures) {
        if (feature != nullptr && feature->type == cJSON_String) {
            config.sinkSupportedFeatures.push_back(std::string(feature->valuestring));
        }
    }
}

void ComponentLoader::CheckAndParseFeatures(cJSON *component, CompConfig &config)
{
    cJSON *sourceFeatureFilters = cJSON_GetObjectItem(component, SOURCE_FEATURE_FILTER);
    cJSON *sinkSupportedFeatures = cJSON_GetObjectItem(component, SINK_SUPPORTED_FEATURE);
    if (sourceFeatureFilters || sinkSupportedFeatures) {
        config.haveFeature = true;
        if (IsArray(sourceFeatureFilters)) {
            ParseSourceFeatureFiltersFromJson(sourceFeatureFilters, config);
        }
        if (IsArray(sinkSupportedFeatures)) {
            ParseSinkSupportedFeaturesFromJson(sinkSupportedFeatures, config);
        }
    } else {
        config.haveFeature = false;
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
    std::lock_guard<std::mutex> lock(compHandlerMapMutex_);
    std::map<DHType, CompConfig>::iterator itor;
    for (itor = dhtypeMap.begin(); itor != dhtypeMap.end(); ++itor) {
        CompHandler comHandler;
        comHandler.type = itor->second.type;
        comHandler.sourceSaId = itor->second.compSourceSaId;
        comHandler.sinkSaId = itor->second.compSinkSaId;
        std::vector<ResourceDesc> compResourceDesc = itor->second.compResourceDesc;
        for (auto it = compResourceDesc.begin(); it != compResourceDesc.end(); it++) {
            resDescMap_[it->subtype] = it->sensitiveValue;
        }
        comHandler.compConfig = itor->second;
        compHandlerMap_[itor->second.type] = comHandler;
    }
}

int32_t ComponentLoader::GetHardwareHandler(const DHType dhType, IHardwareHandler *&hardwareHandlerPtr)
{
    std::lock_guard<std::mutex> lock(compHandlerMapMutex_);
    auto iter = compHandlerMap_.find(dhType);
    if (iter == compHandlerMap_.end()) {
        DHLOGE("DHType not exist, dhType: %{public}" PRIu32, (uint32_t)dhType);
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }
    CompHandler &compHandler = iter->second;
    if (compHandler.hardwareHandler == nullptr) {
        compHandler.hardwareHandler = GetHandler(compHandler.compConfig.compHandlerLoc);
        if (compHandler.hardwareHandler == nullptr) {
            DHLOGE("get hardware handler is null, dhType: %{public}" PRIu32, (uint32_t)dhType);
            return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
        }
    }
    GetHardwareClass getHardwareClassHandler = (GetHardwareClass)dlsym(compHandler.hardwareHandler,
        COMPONENT_LOADER_GET_HARDWARE_HANDLER.c_str());
    if (getHardwareClassHandler == nullptr) {
        DHLOGE("get getHardwareClassHandler is null, failed reason : %{public}s", dlerror());
        ReleaseHandler(compHandler.hardwareHandler);
        compHandler.hardwareHandler = nullptr;
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }
    hardwareHandlerPtr = getHardwareClassHandler();
    return DH_FWK_SUCCESS;
}

int32_t ComponentLoader::GetSource(const DHType dhType, IDistributedHardwareSource *&sourcePtr)
{
    std::lock_guard<std::mutex> lock(compHandlerMapMutex_);
    auto iter = compHandlerMap_.find(dhType);
    if (iter == compHandlerMap_.end()) {
        DHLOGE("DHType not exist, dhType: %{public}" PRIu32, (uint32_t)dhType);
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }
    CompHandler &compHandler = iter->second;
    if (compHandler.sourceHandler == nullptr) {
        compHandler.sourceHandler = GetHandler(compHandler.compConfig.compSourceLoc);
        if (compHandler.sourceHandler == nullptr) {
            DHLOGE("get source handler is null, dhType: %{public}" PRIu32, (uint32_t)dhType);
            return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
        }
    }
    GetSourceHardwareClass getSourceHardClassHandler = (GetSourceHardwareClass)dlsym(
        compHandler.sourceHandler, COMPONENT_LOADER_GET_SOURCE_HANDLER.c_str());
    if (getSourceHardClassHandler == nullptr) {
        DHLOGE("get getSourceHardClassHandler is null, failed reason : %{public}s", dlerror());
        ReleaseHandler(compHandler.sourceHandler);
        compHandler.sourceHandler = nullptr;
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }
    sourcePtr = getSourceHardClassHandler();
    return DH_FWK_SUCCESS;
}

int32_t ComponentLoader::GetSink(const DHType dhType, IDistributedHardwareSink *&sinkPtr)
{
    std::lock_guard<std::mutex> lock(compHandlerMapMutex_);
    auto iter = compHandlerMap_.find(dhType);
    if (iter == compHandlerMap_.end()) {
        DHLOGE("DHType not exist, dhType: %{public}" PRIu32, (uint32_t)dhType);
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }
    CompHandler &compHandler = iter->second;
    if (compHandler.sinkHandler == nullptr) {
        compHandler.sinkHandler = GetHandler(compHandler.compConfig.compSinkLoc);
        if (compHandler.sinkHandler == nullptr) {
            DHLOGE("get sink handler is null, dhType: %{public}" PRIu32, (uint32_t)dhType);
            return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
        }
    }
    GetSinkHardwareClass getSinkHardwareClassHandler =
        (GetSinkHardwareClass)dlsym(compHandler.sinkHandler, COMPONENT_LOADER_GET_SINK_HANDLER.c_str());
    if (getSinkHardwareClassHandler == nullptr) {
        DHLOGE("get getSinkHardwareClassHandler is null, failed reason : %{public}s", dlerror());
        ReleaseHandler(compHandler.sinkHandler);
        compHandler.sinkHandler = nullptr;
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
        DHLOGE("filePath: %{public}s Readfile fail", GetAnonyString(filePath).c_str());
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
    if (!IsMessageLengthValid(jsonStr)) {
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
    std::lock_guard<std::mutex> lock(compHandlerMapMutex_);
    for (auto iter = compHandlerMap_.begin(); iter != compHandlerMap_.end(); ++iter) {
        if (iter->second.sinkHandler) {
            ReleaseHandler(iter->second.sinkHandler);
        }
        if (iter->second.sourceHandler) {
            ReleaseHandler(iter->second.sourceHandler);
        }
        if (iter->second.hardwareHandler) {
            ReleaseHandler(iter->second.hardwareHandler);
        }
    }
    compHandlerMap_.clear();
    resDescMap_.clear();
    DHTraceEnd();
    return DH_FWK_SUCCESS;
}

int32_t ComponentLoader::ReleaseHardwareHandler(const DHType dhType)
{
    if (!IsDHTypeExist(dhType)) {
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }
    if (!IsDHTypeHandlerLoaded(dhType)) {
        return ERR_DH_FWK_LOADER_HANDLER_UNLOAD;
    }
    int32_t ret = ReleaseHandler(compHandlerMap_[dhType].hardwareHandler);
    if (ret) {
        DHLOGE("fail, dhType: %{public}#X", dhType);
        HiSysEventWriteReleaseMsg(DHFWK_RELEASE_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            dhType, ret, "dhfwk release hardware handler failed.");
    }
    compHandlerMap_[dhType].hardwareHandler = nullptr;
    return ret;
}

int32_t ComponentLoader::ReleaseSource(const DHType dhType)
{
    if (!IsDHTypeExist(dhType)) {
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }
    if (!IsDHTypeSourceLoaded(dhType)) {
        return ERR_DH_FWK_LOADER_SOURCE_UNLOAD;
    }
    int32_t ret = ReleaseHandler(compHandlerMap_[dhType].sourceHandler);
    if (ret) {
        DHLOGE("fail, dhType: %{public}#X", dhType);
        HiSysEventWriteReleaseMsg(DHFWK_RELEASE_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            dhType, ret, "dhfwk release source failed.");
    }
    compHandlerMap_[dhType].sourceHandler = nullptr;
    return ret;
}

int32_t ComponentLoader::ReleaseSink(const DHType dhType)
{
    if (!IsDHTypeExist(dhType)) {
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }
    if (!IsDHTypeSinkLoaded(dhType)) {
        return ERR_DH_FWK_LOADER_SINK_UNLOAD;
    }
    int32_t ret = ReleaseHandler(compHandlerMap_[dhType].sinkHandler);
    if (ret) {
        DHLOGE("fail, dhType: %{public}#X", dhType);
        HiSysEventWriteReleaseMsg(DHFWK_RELEASE_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            dhType, ret, "dhfwk release sink failed.");
    }
    compHandlerMap_[dhType].sinkHandler = nullptr;
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

bool ComponentLoader::IsDHTypeSinkLoaded(DHType dhType)
{
    if (compHandlerMap_[dhType].sinkHandler == nullptr) {
        DHLOGE("fail, dhType: %{public}#X sink not loaded", dhType);
        return false;
    }
    return true;
}

bool ComponentLoader::IsDHTypeSourceLoaded(DHType dhType)
{
    if (compHandlerMap_[dhType].sourceHandler == nullptr) {
        DHLOGE("fail, dhType: %{public}#X source not loaded", dhType);
        return false;
    }
    return true;
}

bool ComponentLoader::IsDHTypeHandlerLoaded(DHType dhType)
{
    if (compHandlerMap_[dhType].hardwareHandler == nullptr) {
        DHLOGE("fail, dhType: %{public}#X handler not loaded", dhType);
        return false;
    }
    return true;
}

int32_t ComponentLoader::GetSourceSaId(const DHType dhType)
{
    std::lock_guard<std::mutex> lock(compHandlerMapMutex_);
    if (compHandlerMap_.find(dhType) == compHandlerMap_.end()) {
        DHLOGE("DHType not exist, dhType: %{public}" PRIu32, (uint32_t)dhType);
        return DEFAULT_SA_ID;
    }
    return compHandlerMap_[dhType].sourceSaId;
}

DHType ComponentLoader::GetDHTypeBySrcSaId(const int32_t saId)
{
    std::lock_guard<std::mutex> lock(compHandlerMapMutex_);
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

int32_t ComponentLoader::GetSource(const DHType dhType)
{
    std::lock_guard<std::mutex> lock(compHandlerMapMutex_);
    auto iter = compHandlerMap_.find(dhType);
    if (iter == compHandlerMap_.end()) {
        DHLOGE("DHType not exist, dhType: %{public}" PRIu32, (uint32_t)dhType);
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }
    CompHandler &compHandler = iter->second;
    if (compHandler.sourceHandler != nullptr) {
        DHLOGE("sourceHandler is loaded.");
        return ERR_DH_FWK_LOADER_SOURCE_LOAD;
    }
    compHandler.sourceHandler = GetHandler(compHandler.compConfig.compSourceLoc);
    return DH_FWK_SUCCESS;
}

int32_t ComponentLoader::GetSink(const DHType dhType)
{
    std::lock_guard<std::mutex> lock(compHandlerMapMutex_);
    auto iter = compHandlerMap_.find(dhType);
    if (iter == compHandlerMap_.end()) {
        DHLOGE("DHType not exist, dhType: %{public}" PRIu32, (uint32_t)dhType);
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }
    CompHandler &compHandler = iter->second;
    if (compHandler.sinkHandler != nullptr) {
        DHLOGE("sinkHandler is loaded.");
        return ERR_DH_FWK_LOADER_SINK_LOAD;
    }
    compHandler.sinkHandler = GetHandler(compHandler.compConfig.compSinkLoc);
    return DH_FWK_SUCCESS;
}

int32_t ComponentLoader::GetHardwareHandler(const DHType dhType)
{
    std::lock_guard<std::mutex> lock(compHandlerMapMutex_);
    auto iter = compHandlerMap_.find(dhType);
    if (iter == compHandlerMap_.end()) {
        DHLOGE("DHType not exist, dhType: %{public}" PRIu32, (uint32_t)dhType);
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }
    CompHandler &compHandler = iter->second;
    if (compHandler.hardwareHandler != nullptr) {
        DHLOGE("hardwareHandler is loaded.");
        return ERR_DH_FWK_LOADER_HANDLER_LOAD;
    }
    compHandler.hardwareHandler = GetHandler(compHandler.compConfig.compHandlerLoc);
    return DH_FWK_SUCCESS;
}

bool ComponentLoader::IsDHTypeSupport(DHType dhType)
{
    return IsDHTypeExist(dhType);
}
} // namespace DistributedHardware
} // namespace OHOS
