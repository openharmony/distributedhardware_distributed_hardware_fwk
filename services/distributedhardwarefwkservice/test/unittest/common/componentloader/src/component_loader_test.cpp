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

#include "component_loader_test.h"

#include "config_policy_utils.h"
#include "component_loader.h"
#include "distributed_hardware_log.h"
#include "hitrace_meter.h"
#include "hidump_helper.h"
#include "kvstore_observer.h"
#include "cJSON.h"
#include "versionmanager/version_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void ComponentLoaderTest::SetUpTestCase(void) {}

void ComponentLoaderTest::TearDownTestCase(void) {}

void ComponentLoaderTest::SetUp()
{
}

void ComponentLoaderTest::TearDown()
{
}

HWTEST_F(ComponentLoaderTest, CheckComponentEnable_001, TestSize.Level0)
{
    CompConfig config = {
        .name = "name",
        .type = DHType::UNKNOWN,
        .compSourceSaId = 4801,
        .compSinkSaId = 4802
    };
    auto ret = ComponentLoader::GetInstance().CheckComponentEnable(config);
    EXPECT_EQ(true, ret);

    CompConfig config1 = {
        .name = "name",
        .type = DHType::INPUT,
        .compSourceSaId = 4801,
        .compSinkSaId = 4802
    };
    ret = ComponentLoader::GetInstance().CheckComponentEnable(config1);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: GetLocalDHVersion_001
 * @tc.desc: Verify the GetLocalDHVersion function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, GetLocalDHVersion_001, TestSize.Level0)
{
    DHVersion dhVersion;
    ComponentLoader::GetInstance().isLocalVersionInit_.store(false);
    ComponentLoader::GetInstance().StoreLocalDHVersionInDB();
    auto ret = ComponentLoader::GetInstance().GetLocalDHVersion(dhVersion);
    EXPECT_EQ(ERR_DH_FWK_LOADER_GET_LOCAL_VERSION_FAIL, ret);

    ComponentLoader::GetInstance().isLocalVersionInit_.store(true);
    ret = ComponentLoader::GetInstance().GetLocalDHVersion(dhVersion);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: GetHardwareHandler_001
 * @tc.desc: Verify the GetHardwareHandler function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, GetHardwareHandler_001, TestSize.Level0)
{
    IHardwareHandler *hardwareHandlerPtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetHardwareHandler(DHType::UNKNOWN, hardwareHandlerPtr);
    EXPECT_EQ(ERR_DH_FWK_LOADER_HANDLER_IS_NULL, ret);

    CompHandler comHandler;
    comHandler.hardwareHandler = nullptr;
    ComponentLoader::GetInstance().compHandlerMap_[DHType::AUDIO] = comHandler;
    ret = ComponentLoader::GetInstance().GetHardwareHandler(DHType::AUDIO, hardwareHandlerPtr);
    EXPECT_EQ(ERR_DH_FWK_LOADER_HANDLER_IS_NULL, ret);
    ComponentLoader::GetInstance().compHandlerMap_.clear();
}

/**
 * @tc.name: GetHardwareHandler_002
 * @tc.desc: Verify the GetHardwareHandler function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, GetHardwareHandler_002, TestSize.Level0)
{
    ComponentLoader::GetInstance().Init();
    IHardwareHandler *hardwareHandlerPtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetHardwareHandler(DHType::AUDIO, hardwareHandlerPtr);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    ComponentLoader::GetInstance().UnInit();
}

/**
 * @tc.name: GetSource_001
 * @tc.desc: Verify the GetSource function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, GetSource_001, TestSize.Level0)
{
    IDistributedHardwareSource *sourcePtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetSource(DHType::UNKNOWN, sourcePtr);
    EXPECT_EQ(ERR_DH_FWK_LOADER_HANDLER_IS_NULL, ret);

    CompHandler comHandler;
    comHandler.sourceHandler = nullptr;
    ComponentLoader::GetInstance().compHandlerMap_[DHType::AUDIO] = comHandler;
    ret = ComponentLoader::GetInstance().GetSource(DHType::AUDIO, sourcePtr);
    EXPECT_EQ(ERR_DH_FWK_LOADER_HANDLER_IS_NULL, ret);
}

/**
 * @tc.name: GetSource_002
 * @tc.desc: Verify the GetSource function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, GetSource_002, TestSize.Level0)
{
    ComponentLoader::GetInstance().Init();
    IDistributedHardwareSource *sourcePtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetSource(DHType::AUDIO, sourcePtr);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    ComponentLoader::GetInstance().UnInit();
}

/**
 * @tc.name: GetSink_001
 * @tc.desc: Verify the GetSink function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, GetSink_001, TestSize.Level0)
{
    IDistributedHardwareSink *sinkPtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetSink(DHType::UNKNOWN, sinkPtr);
    EXPECT_EQ(ERR_DH_FWK_LOADER_HANDLER_IS_NULL, ret);

    CompHandler comHandler;
    comHandler.sinkHandler = nullptr;
    ComponentLoader::GetInstance().compHandlerMap_[DHType::AUDIO] = comHandler;
    ret = ComponentLoader::GetInstance().GetSink(DHType::AUDIO, sinkPtr);
    EXPECT_EQ(ERR_DH_FWK_LOADER_HANDLER_IS_NULL, ret);
}

/**
 * @tc.name: GetSink_002
 * @tc.desc: Verify the GetSink function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, GetSink_002, TestSize.Level0)
{
    ComponentLoader::GetInstance().Init();
    IDistributedHardwareSink *sinkPtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetSink(DHType::AUDIO, sinkPtr);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    ComponentLoader::GetInstance().UnInit();
}

/**
 * @tc.name: Readfile_001
 * @tc.desc: Verify the Readfile function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, Readfile_001, TestSize.Level0)
{
    std::string filePath = "";
    auto ret = ComponentLoader::GetInstance().Readfile(filePath);
    EXPECT_EQ("", ret);
}

/**
 * @tc.name: ReleaseHardwareHandler_001
 * @tc.desc: Verify the ReleaseHardwareHandler function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, ReleaseHardwareHandler_001, TestSize.Level0)
{
    auto ret = ComponentLoader::GetInstance().ReleaseHardwareHandler(DHType::AUDIO);
    EXPECT_EQ(ERR_DH_FWK_TYPE_NOT_EXIST, ret);

    CompHandler comHandler;
    comHandler.hardwareHandler = nullptr;
    ComponentLoader::GetInstance().compHandlerMap_[DHType::AUDIO] = comHandler;
    ret = ComponentLoader::GetInstance().ReleaseHardwareHandler(DHType::AUDIO);
    EXPECT_EQ(ERR_DH_FWK_LOADER_HANDLER_IS_NULL, ret);
    ComponentLoader::GetInstance().compHandlerMap_.clear();
}

/**
 * @tc.name: ReleaseSource_001
 * @tc.desc: Verify the ReleaseSource function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, ReleaseSource_001, TestSize.Level0)
{
    auto ret = ComponentLoader::GetInstance().ReleaseSource(DHType::AUDIO);
    EXPECT_EQ(ERR_DH_FWK_TYPE_NOT_EXIST, ret);

    CompHandler comHandler;
    comHandler.sourceHandler = nullptr;
    ComponentLoader::GetInstance().compHandlerMap_[DHType::AUDIO] = comHandler;
    ret = ComponentLoader::GetInstance().ReleaseSource(DHType::AUDIO);
    EXPECT_EQ(ERR_DH_FWK_LOADER_HANDLER_IS_NULL, ret);
    ComponentLoader::GetInstance().compHandlerMap_.clear();
}

/**
 * @tc.name: ReleaseSink_001
 * @tc.desc: Verify the ReleaseSink function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, ReleaseSink_001, TestSize.Level0)
{
    auto ret = ComponentLoader::GetInstance().ReleaseSink(DHType::AUDIO);
    EXPECT_EQ(ERR_DH_FWK_TYPE_NOT_EXIST, ret);

    CompHandler comHandler;
    comHandler.sinkHandler = nullptr;
    ComponentLoader::GetInstance().compHandlerMap_[DHType::AUDIO] = comHandler;
    ret = ComponentLoader::GetInstance().ReleaseSink(DHType::AUDIO);
    EXPECT_EQ(ERR_DH_FWK_LOADER_HANDLER_IS_NULL, ret);
    ComponentLoader::GetInstance().compHandlerMap_.clear();
}

/**
 * @tc.name: GetHandler_001
 * @tc.desc: Verify the GetHandler function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, GetHandler_001, TestSize.Level0)
{
    std::string soNameEmpty = "";
    auto handler = ComponentLoader::GetInstance().GetHandler(soNameEmpty);
    EXPECT_EQ(nullptr, handler);
}

/**
 * @tc.name: component_loader_test_017
 * @tc.desc: Verify the GetCompPathAndVersion function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_017, TestSize.Level0)
{
    std::string jsonStr = "";
    std::map<DHType, CompConfig> dhtypeMap;
    int32_t ret = ComponentLoader::GetInstance().GetCompPathAndVersion(jsonStr, dhtypeMap);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: component_loader_test_018
 * @tc.desc: Verify the GetCompPathAndVersion function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_018, TestSize.Level0)
{
    const char *NAME = "NAME";
    const char *TYPE = "TYPE";
    const char *PATH = "PATH";
    cJSON* json0bject = cJSON_CreateObject();
    if (json0bject == nullptr) {
        return;
    }
    cJSON* compVers = cJSON_CreateObject();
    if (compVers == nullptr) {
        cJSON_Delete(json0bject);
        return;
    }
    cJSON_AddStringToObject(compVers, NAME, "name");
    cJSON_AddNumberToObject(compVers, TYPE, 1111);
    cJSON_AddItemToObject(json0bject, PATH, compVers);
    char* cjson = cJSON_PrintUnformatted(json0bject);
    if (cjson == nullptr) {
        cJSON_Delete(json0bject);
        return;
    }
    std::string jsonStr(cjson);
    std::map<DHType, CompConfig> dhtypeMap;
    int32_t ret = ComponentLoader::GetInstance().GetCompPathAndVersion(jsonStr, dhtypeMap);
    cJSON_free(cjson);
    cJSON_Delete(json0bject);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: IsDHTypeExist_001
 * @tc.desc: Verify the IsDHTypeExist function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, IsDHTypeExist_001, TestSize.Level0)
{
    CompHandler comHandler;
    ComponentLoader::GetInstance().compHandlerMap_[DHType::AUDIO] = comHandler;
    bool ret = ComponentLoader::GetInstance().IsDHTypeExist(DHType::AUDIO);
    EXPECT_EQ(true, ret);
    ComponentLoader::GetInstance().compHandlerMap_.clear();
}

/**
 * @tc.name: GetSourceSaId_001
 * @tc.desc: Verify the GetSourceSaId function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, GetSourceSaId_001, TestSize.Level0)
{
    CompHandler comHandler;
    ComponentLoader::GetInstance().compHandlerMap_[DHType::AUDIO] = comHandler;
    int32_t ret = ComponentLoader::GetInstance().GetSourceSaId(DHType::UNKNOWN);
    EXPECT_EQ(DEFAULT_SA_ID, ret);

    comHandler.sourceSaId = 1;
    ComponentLoader::GetInstance().compHandlerMap_[DHType::AUDIO] = comHandler;
    ret = ComponentLoader::GetInstance().GetSourceSaId(DHType::AUDIO);
    EXPECT_EQ(1, ret);
}

/**
 * @tc.name: component_loader_test_022
 * @tc.desc: Verify the ParseConfig function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_022, TestSize.Level0)
{
    int32_t ret = ComponentLoader::GetInstance().ParseConfig();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: component_loader_test_023
 * @tc.desc: Verify the ReleaseHandler function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_023, TestSize.Level0)
{
    void *handler = nullptr;
    int32_t ret = ComponentLoader::GetInstance().ReleaseHandler(handler);
    EXPECT_EQ(ERR_DH_FWK_LOADER_HANDLER_IS_NULL, ret);
}

/**
 * @tc.name: component_loader_test_024
 * @tc.desc: Verify the ReleaseHardwareHandler function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_024, TestSize.Level0)
{
    DHType dhType = DHType::GPS;
    int32_t ret = ComponentLoader::GetInstance().ReleaseHardwareHandler(dhType);
    EXPECT_EQ(ERR_DH_FWK_TYPE_NOT_EXIST, ret);
}

/**
 * @tc.name: component_loader_test_025
 * @tc.desc: Verify the ReleaseSource function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_025, TestSize.Level0)
{
    DHType dhType = DHType::GPS;
    int32_t ret = ComponentLoader::GetInstance().ReleaseSource(dhType);
    EXPECT_EQ(ERR_DH_FWK_TYPE_NOT_EXIST, ret);
}

/**
 * @tc.name: component_loader_test_026
 * @tc.desc: Verify the ReleaseSink function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_026, TestSize.Level0)
{
    DHType dhType = DHType::GPS;
    int32_t ret = ComponentLoader::GetInstance().ReleaseSink(dhType);
    EXPECT_EQ(ERR_DH_FWK_TYPE_NOT_EXIST, ret);
}

/**
 * @tc.name: component_loader_test_027
 * @tc.desc: Verify the ReleaseSink function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_027, TestSize.Level0)
{
    DHType dhType = DHType::GPS;
    bool ret = ComponentLoader::GetInstance().IsDHTypeExist(dhType);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: component_loader_test_028
 * @tc.desc: Verify the GetDHTypeBySrcSaId function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_028, TestSize.Level0)
{
    int32_t saId = 4801;
    DHType dhType = ComponentLoader::GetInstance().GetDHTypeBySrcSaId(saId);
    EXPECT_EQ(dhType, DHType::UNKNOWN);
}

/**
 * @tc.name: component_loader_test_029
 * @tc.desc: Verify the from_json function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_029, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddNumberToObject(json, COMP_NAME.c_str(), 4801);

    from_json(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(true, cfg.name.empty());
}

/**
 * @tc.name: component_loader_test_030
 * @tc.desc: Verify the from_json function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_030, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddStringToObject(json, COMP_NAME.c_str(), "name");
    cJSON_AddNumberToObject(json, COMP_TYPE.c_str(), 0x02);

    from_json(json, cfg);
    cJSON_Delete(json);
    EXPECT_NE(DHType::AUDIO, cfg.type);
}

/**
 * @tc.name: component_loader_test_031
 * @tc.desc: Verify the from_json function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_031, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddStringToObject(json, COMP_NAME.c_str(), "name");
    cJSON_AddStringToObject(json, COMP_TYPE.c_str(), "DHType::AUDIO");
    cJSON_AddNumberToObject(json, COMP_HANDLER_LOC.c_str(), 4801);

    from_json(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(true, cfg.compHandlerLoc.empty());
}

/**
 * @tc.name: component_loader_test_032
 * @tc.desc: Verify the from_json function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_032, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddStringToObject(json, COMP_NAME.c_str(), "name");
    cJSON_AddStringToObject(json, COMP_TYPE.c_str(), "DHType::AUDIO");
    cJSON_AddStringToObject(json, COMP_HANDLER_LOC.c_str(), "comp_handler_loc");
    cJSON_AddNumberToObject(json, COMP_HANDLER_VERSION.c_str(), 4801);

    from_json(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(true, cfg.compHandlerVersion.empty());
}

/**
 * @tc.name: component_loader_test_033
 * @tc.desc: Verify the from_json function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_033, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddStringToObject(json, COMP_NAME.c_str(), "name");
    cJSON_AddStringToObject(json, COMP_TYPE.c_str(), "DHType::AUDIO");
    cJSON_AddStringToObject(json, COMP_HANDLER_LOC.c_str(), "comp_handler_loc");
    cJSON_AddStringToObject(json, COMP_HANDLER_VERSION.c_str(), "comp_handler_version");
    cJSON_AddNumberToObject(json, COMP_SOURCE_LOC.c_str(), 4801);

    from_json(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(true, cfg.compSourceLoc.empty());
}

/**
 * @tc.name: component_loader_test_034
 * @tc.desc: Verify the from_json function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_034, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddStringToObject(json, COMP_NAME.c_str(), "name");
    cJSON_AddStringToObject(json, COMP_TYPE.c_str(), "DHType::AUDIO");
    cJSON_AddStringToObject(json, COMP_HANDLER_LOC.c_str(), "comp_handler_loc");
    cJSON_AddStringToObject(json, COMP_HANDLER_VERSION.c_str(), "comp_handler_version");
    cJSON_AddStringToObject(json, COMP_SOURCE_LOC.c_str(), "comp_source_loc");
    cJSON_AddNumberToObject(json, COMP_SOURCE_VERSION.c_str(), 4801);

    from_json(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(true, cfg.compSourceVersion.empty());
}

/**
 * @tc.name: component_loader_test_035
 * @tc.desc: Verify the from_json function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_035, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddStringToObject(json, COMP_NAME.c_str(), "name");
    cJSON_AddStringToObject(json, COMP_TYPE.c_str(), "DHType::AUDIO");
    cJSON_AddStringToObject(json, COMP_HANDLER_LOC.c_str(), "comp_handler_loc");
    cJSON_AddStringToObject(json, COMP_HANDLER_VERSION.c_str(), "comp_handler_version");
    cJSON_AddStringToObject(json, COMP_SOURCE_LOC.c_str(), "comp_source_loc");
    cJSON_AddStringToObject(json, COMP_SOURCE_VERSION.c_str(), "comp_source_version");
    cJSON_AddStringToObject(json, COMP_SOURCE_SA_ID.c_str(), "comp_source_sa_id");

    from_json(json, cfg);
    cJSON_Delete(json);
    EXPECT_NE(4801, cfg.compSourceSaId);
}

/**
 * @tc.name: component_loader_test_036
 * @tc.desc: Verify the from_json function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_036, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddStringToObject(json, COMP_NAME.c_str(), "name");
    cJSON_AddStringToObject(json, COMP_TYPE.c_str(), "DHType::AUDIO");
    cJSON_AddStringToObject(json, COMP_HANDLER_LOC.c_str(), "comp_handler_loc");
    cJSON_AddStringToObject(json, COMP_HANDLER_VERSION.c_str(), "comp_handler_version");
    cJSON_AddStringToObject(json, COMP_SOURCE_LOC.c_str(), "comp_source_loc");
    cJSON_AddStringToObject(json, COMP_SOURCE_VERSION.c_str(), "comp_source_version");
    cJSON_AddStringToObject(json, COMP_SOURCE_SA_ID.c_str(), "4801");
    cJSON_AddNumberToObject(json, COMP_SINK_LOC.c_str(), 4802);

    from_json(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(true, cfg.compSinkLoc.empty());
}

/**
 * @tc.name: component_loader_test_037
 * @tc.desc: Verify the from_json function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_037, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddStringToObject(json, COMP_NAME.c_str(), "name");
    cJSON_AddStringToObject(json, COMP_TYPE.c_str(), "DHType::AUDIO");
    cJSON_AddStringToObject(json, COMP_HANDLER_LOC.c_str(), "comp_handler_loc");
    cJSON_AddStringToObject(json, COMP_HANDLER_VERSION.c_str(), "comp_handler_version");
    cJSON_AddStringToObject(json, COMP_SOURCE_LOC.c_str(), "comp_source_loc");
    cJSON_AddStringToObject(json, COMP_SOURCE_VERSION.c_str(), "comp_source_version");
    cJSON_AddNumberToObject(json, COMP_SOURCE_SA_ID.c_str(), 4801);
    cJSON_AddStringToObject(json, COMP_SINK_LOC.c_str(), "comp_sink_loc");
    cJSON_AddNumberToObject(json, COMP_SINK_VERSION.c_str(), 4802);

    from_json(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(true, cfg.compSinkVersion.empty());
}

/**
 * @tc.name: component_loader_test_038
 * @tc.desc: Verify the from_json function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_038, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddStringToObject(json, COMP_NAME.c_str(), "name");
    cJSON_AddStringToObject(json, COMP_TYPE.c_str(), "DHType::AUDIO");
    cJSON_AddStringToObject(json, COMP_HANDLER_LOC.c_str(), "comp_handler_loc");
    cJSON_AddStringToObject(json, COMP_HANDLER_VERSION.c_str(), "comp_handler_version");
    cJSON_AddStringToObject(json, COMP_SOURCE_LOC.c_str(), "comp_source_loc");
    cJSON_AddStringToObject(json, COMP_SOURCE_VERSION.c_str(), "comp_source_version");
    cJSON_AddNumberToObject(json, COMP_SOURCE_SA_ID.c_str(), 4801);
    cJSON_AddStringToObject(json, COMP_SINK_LOC.c_str(), "comp_sink_loc");
    cJSON_AddStringToObject(json, COMP_SINK_VERSION.c_str(), "comp_sink_version");
    cJSON_AddStringToObject(json, COMP_SINK_SA_ID.c_str(), "4802");

    from_json(json, cfg);
    cJSON_Delete(json);
    EXPECT_NE(4802, cfg.compSinkSaId);
}

/**
 * @tc.name: component_loader_test_039
 * @tc.desc: Verify the from_json function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_039, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddStringToObject(json, COMP_NAME.c_str(), "name");
    cJSON_AddStringToObject(json, COMP_TYPE.c_str(), "DHType::AUDIO");
    cJSON_AddStringToObject(json, COMP_HANDLER_LOC.c_str(), "comp_handler_loc");
    cJSON_AddStringToObject(json, COMP_HANDLER_VERSION.c_str(), "comp_handler_version");
    cJSON_AddStringToObject(json, COMP_SOURCE_LOC.c_str(), "comp_source_loc");
    cJSON_AddStringToObject(json, COMP_SOURCE_VERSION.c_str(), "comp_source_version");
    cJSON_AddNumberToObject(json, COMP_SOURCE_SA_ID.c_str(), 4801);
    cJSON_AddStringToObject(json, COMP_SINK_LOC.c_str(), "comp_sink_loc");
    cJSON_AddStringToObject(json, COMP_SINK_VERSION.c_str(), "comp_sink_version");
    cJSON_AddNumberToObject(json, COMP_SINK_SA_ID.c_str(), 4802);

    from_json(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(4802, cfg.compSinkSaId);
}

HWTEST_F(ComponentLoaderTest, ParseSink_001, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddNumberToObject(json, COMP_SINK_LOC.c_str(), 100);
    auto ret = ParseSink(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(ret, ERR_DH_FWK_JSON_PARSE_FAILED);
}

HWTEST_F(ComponentLoaderTest, ParseSink_002, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddStringToObject(json, COMP_SINK_LOC.c_str(), "comp_sink_loc_test");
    cJSON_AddNumberToObject(json, COMP_SINK_VERSION.c_str(), 100);
    auto ret = ParseSink(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(ret, ERR_DH_FWK_JSON_PARSE_FAILED);
}

HWTEST_F(ComponentLoaderTest, ParseSink_003, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddStringToObject(json, COMP_SINK_LOC.c_str(), "comp_sink_loc_test");
    cJSON_AddStringToObject(json, COMP_SINK_VERSION.c_str(), "1.0");
    cJSON_AddStringToObject(json, COMP_SINK_SA_ID.c_str(), "comp_sink_sa_id_test");
    auto ret = ParseSink(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(ret, ERR_DH_FWK_JSON_PARSE_FAILED);
}

HWTEST_F(ComponentLoaderTest, ParseSink_004, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddStringToObject(json, COMP_SINK_LOC.c_str(), "comp_sink_loc_test");
    cJSON_AddStringToObject(json, COMP_SINK_VERSION.c_str(), "1.0");
    cJSON_AddNumberToObject(json, COMP_SINK_SA_ID.c_str(), 4801);
    auto ret = ParseSink(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentLoaderTest, ParseResourceDesc_001, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *array = cJSON_CreateArray();
    if (array == nullptr) {
        return;
    }
    cJSON *obj = cJSON_CreateObject();
    if (obj == nullptr) {
        cJSON_Delete(array);
        return;
    }
    cJSON_AddStringToObject(obj, COMP_SUBTYPE.c_str(), "comp_subtype");
    cJSON_AddBoolToObject(obj, COMP_SENSITIVE.c_str(), true);
    cJSON_AddItemToArray(array, obj);
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        cJSON_Delete(array);
        return;
    }
    cJSON_AddItemToObject(json, COMP_RESOURCE_DESC.c_str(), array);
    auto ret = ParseResourceDesc(json, cfg);
    cJSON_Delete(json);

    CompConfig config;
    cJSON *component = cJSON_CreateObject();
    if (component == nullptr) {
        return;
    }
    cJSON_AddNumberToObject(component, COMP_NAME.c_str(), 1);
    cJSON_AddNumberToObject(component, COMP_TYPE.c_str(), 1);
    cJSON_AddNumberToObject(component, COMP_HANDLER_LOC.c_str(), 1);
    cJSON_AddNumberToObject(component, COMP_HANDLER_VERSION.c_str(), 1.0);
    cJSON_AddNumberToObject(component, COMP_SOURCE_LOC.c_str(), 1);
    cJSON_AddNumberToObject(component, COMP_SOURCE_VERSION.c_str(), 1.0);
    cJSON_AddStringToObject(component, COMP_SOURCE_SA_ID.c_str(), "4801");
    cJSON_AddNumberToObject(component, COMP_SINK_LOC.c_str(), 1);
    cJSON_AddNumberToObject(component, COMP_SINK_VERSION.c_str(), 1.0);
    cJSON_AddStringToObject(component, COMP_SINK_SA_ID.c_str(), "4802");
    cJSON_AddStringToObject(component, COMP_RESOURCE_DESC.c_str(), "comp_resource_desc");
    ComponentLoader::GetInstance().ParseCompConfigFromJson(component, config);
    cJSON_Delete(component);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentLoaderTest, ParseResourceDesc_002, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddNumberToObject(json, COMP_NAME.c_str(), 100);
    auto ret = ParseComponent(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(ret, ERR_DH_FWK_JSON_PARSE_FAILED);

    CompConfig config1;
    cJSON *component1 = cJSON_CreateObject();
    if (component1 == nullptr) {
        return;
    }
    cJSON_AddStringToObject(component1, COMP_NAME.c_str(), "comp_name_test");
    cJSON_AddStringToObject(component1, COMP_TYPE.c_str(), "comp_type_test");
    cJSON_AddStringToObject(component1, COMP_HANDLER_LOC.c_str(), "comp_handler_loc_test");
    cJSON_AddStringToObject(component1, COMP_HANDLER_VERSION.c_str(), "comp_handler_version_test");
    cJSON_AddStringToObject(component1, COMP_SOURCE_LOC.c_str(), "comp_source_loc_test");
    cJSON_AddStringToObject(component1, COMP_SOURCE_VERSION.c_str(), "comp_source_verison_test");
    cJSON_AddNumberToObject(component1, COMP_SOURCE_SA_ID.c_str(), 4801);
    cJSON_AddStringToObject(component1, COMP_SINK_LOC.c_str(), "comp_sink_loc_test");
    cJSON_AddStringToObject(component1, COMP_SINK_VERSION.c_str(), "com_sink_version_test");
    cJSON_AddNumberToObject(component1, COMP_SINK_SA_ID.c_str(), 4802);
    cJSON_AddStringToObject(component1, COMP_RESOURCE_DESC.c_str(), "comp_resource_desc");
    ComponentLoader::GetInstance().ParseCompConfigFromJson(component1, config1);
    cJSON_Delete(component1);
}

HWTEST_F(ComponentLoaderTest, from_json_001, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddNumberToObject(json, COMP_NAME.c_str(), 100);
    from_json(json, cfg);
    cJSON_Delete(json);

    cJSON *Json1 = cJSON_CreateObject();
    if (Json1 == nullptr) {
        return;
    }
    cJSON_AddStringToObject(Json1, COMP_NAME.c_str(), "comp_name_test");
    cJSON_AddStringToObject(Json1, COMP_TYPE.c_str(), "comp_type_test");
    cJSON_AddStringToObject(Json1, COMP_HANDLER_LOC.c_str(), "comp_handler_loc_test");
    cJSON_AddStringToObject(Json1, COMP_HANDLER_VERSION.c_str(), "1.0");
    cJSON_AddNumberToObject(Json1, COMP_SOURCE_LOC.c_str(), 100);
    from_json(Json1, cfg);
    cJSON_Delete(Json1);

    cJSON *Json2 = cJSON_CreateObject();
    if (Json2 == nullptr) {
        return;
    }
    cJSON_AddStringToObject(Json2, COMP_NAME.c_str(), "comp_name_test");
    cJSON_AddStringToObject(Json2, COMP_TYPE.c_str(), "comp_type_test");
    cJSON_AddStringToObject(Json2, COMP_HANDLER_LOC.c_str(), "comp_handler_loc_test");
    cJSON_AddStringToObject(Json2, COMP_HANDLER_VERSION.c_str(), "1.0");
    cJSON_AddStringToObject(Json2, COMP_SOURCE_LOC.c_str(), "comp_source_loc_test");
    cJSON_AddStringToObject(Json2, COMP_SOURCE_VERSION.c_str(), "1.0");
    cJSON_AddNumberToObject(Json2, COMP_SOURCE_SA_ID.c_str(), 4801);
    cJSON_AddNumberToObject(Json2, COMP_SINK_LOC.c_str(), 100);
    from_json(Json2, cfg);
    EXPECT_EQ(4801, static_cast<int32_t>(cJSON_GetObjectItem(Json2, COMP_SOURCE_SA_ID.c_str())->valuedouble));
    cJSON_Delete(Json2);
}

HWTEST_F(ComponentLoaderTest, ParseComponent_001, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddNumberToObject(json, COMP_NAME.c_str(), 100);
    auto ret = ParseComponent(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(ret, ERR_DH_FWK_JSON_PARSE_FAILED);
}

HWTEST_F(ComponentLoaderTest, ParseComponent_002, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddStringToObject(json, COMP_NAME.c_str(), "comp_name_test");
    cJSON_AddNumberToObject(json, COMP_TYPE.c_str(), 100);
    auto ret = ParseComponent(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(ret, ERR_DH_FWK_JSON_PARSE_FAILED);
}

HWTEST_F(ComponentLoaderTest, ParseComponent_003, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddStringToObject(json, COMP_NAME.c_str(), "comp_name_test");
    cJSON_AddStringToObject(json, COMP_TYPE.c_str(), "comp_type_test");
    cJSON_AddNumberToObject(json, COMP_HANDLER_LOC.c_str(), 100);
    auto ret = ParseComponent(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(ret, ERR_DH_FWK_JSON_PARSE_FAILED);
}

HWTEST_F(ComponentLoaderTest, ParseComponent_004, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddStringToObject(json, COMP_NAME.c_str(), "comp_name_test");
    cJSON_AddStringToObject(json, COMP_TYPE.c_str(), "comp_type_test");
    cJSON_AddStringToObject(json, COMP_HANDLER_LOC.c_str(), "comp_handler_loc_test");
    cJSON_AddNumberToObject(json, COMP_HANDLER_VERSION.c_str(), 100);
    auto ret = ParseComponent(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(ret, ERR_DH_FWK_JSON_PARSE_FAILED);
}

HWTEST_F(ComponentLoaderTest, ParseComponent_005, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddStringToObject(json, COMP_NAME.c_str(), "comp_name_test");
    cJSON_AddStringToObject(json, COMP_TYPE.c_str(), "comp_type_test");
    cJSON_AddStringToObject(json, COMP_HANDLER_LOC.c_str(), "comp_handler_loc_test");
    cJSON_AddStringToObject(json, COMP_HANDLER_VERSION.c_str(), "1.0");
    auto ret = ParseComponent(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentLoaderTest, ParseSource_001, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddNumberToObject(json, COMP_SOURCE_LOC.c_str(), 100);
    auto ret = ParseSource(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(ret, ERR_DH_FWK_JSON_PARSE_FAILED);
}

HWTEST_F(ComponentLoaderTest, ParseSource_002, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddStringToObject(json, COMP_SOURCE_LOC.c_str(), "comp_source_loc_test");
    cJSON_AddNumberToObject(json, COMP_SOURCE_VERSION.c_str(), 100);
    auto ret = ParseSource(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(ret, ERR_DH_FWK_JSON_PARSE_FAILED);
}

HWTEST_F(ComponentLoaderTest, ParseSource_003, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddStringToObject(json, COMP_SOURCE_LOC.c_str(), "comp_source_loc_test");
    cJSON_AddStringToObject(json, COMP_SOURCE_VERSION.c_str(), "1.0");
    cJSON_AddStringToObject(json, COMP_SOURCE_SA_ID.c_str(), "4801");
    auto ret = ParseSource(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(ret, ERR_DH_FWK_JSON_PARSE_FAILED);
}

HWTEST_F(ComponentLoaderTest, ParseSource_004, TestSize.Level0)
{
    CompConfig cfg;
    cJSON *json = cJSON_CreateObject();
    if (json == nullptr) {
        return;
    }
    cJSON_AddStringToObject(json, COMP_SOURCE_LOC.c_str(), "comp_source_loc_test");
    cJSON_AddStringToObject(json, COMP_SOURCE_VERSION.c_str(), "1.0");
    cJSON_AddNumberToObject(json, COMP_SOURCE_SA_ID.c_str(), 4801);
    auto ret = ParseSource(json, cfg);
    cJSON_Delete(json);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentLoaderTest, GetHardwareHandler_003, TestSize.Level0)
{
    ComponentLoader::GetInstance().compHandlerMap_.clear();
    DHType dhType = DHType::AUDIO;
    IHardwareHandler *hardwareHandlerPtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetHardwareHandler(dhType, hardwareHandlerPtr);
    EXPECT_EQ(ret, ERR_DH_FWK_LOADER_HANDLER_IS_NULL);
}

HWTEST_F(ComponentLoaderTest, GetSource_003, TestSize.Level0)
{
    ComponentLoader::GetInstance().compHandlerMap_.clear();
    DHType dhType = DHType::AUDIO;
    IDistributedHardwareSource *dhSourcePtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetSource(dhType, dhSourcePtr);
    EXPECT_EQ(ret, ERR_DH_FWK_LOADER_HANDLER_IS_NULL);
}

HWTEST_F(ComponentLoaderTest, GetSink_003, TestSize.Level0)
{
    ComponentLoader::GetInstance().compHandlerMap_.clear();
    DHType dhType = DHType::AUDIO;
    IDistributedHardwareSink *dhSinkPtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetSink(dhType, dhSinkPtr);
    EXPECT_EQ(ret, ERR_DH_FWK_LOADER_HANDLER_IS_NULL);
}

HWTEST_F(ComponentLoaderTest, ReleaseHardwareHandler_002, TestSize.Level0)
{
    ComponentLoader::GetInstance().compHandlerMap_.clear();
    DHType dhType = DHType::AUDIO;
    auto ret = ComponentLoader::GetInstance().ReleaseHardwareHandler(dhType);
    EXPECT_EQ(ret, ERR_DH_FWK_TYPE_NOT_EXIST);

    ret = ComponentLoader::GetInstance().ReleaseSource(dhType);
    EXPECT_EQ(ret, ERR_DH_FWK_TYPE_NOT_EXIST);

    ret = ComponentLoader::GetInstance().ReleaseSource(dhType);
    EXPECT_EQ(ret, ERR_DH_FWK_TYPE_NOT_EXIST);

    ret = ComponentLoader::GetInstance().GetSourceSaId(dhType);
    EXPECT_EQ(ret, DEFAULT_SA_ID);
}

HWTEST_F(ComponentLoaderTest, GetDHTypeBySrcSaId_001, TestSize.Level0)
{
    ComponentLoader::GetInstance().compHandlerMap_.clear();
    int32_t saId = 4801;
    auto ret = ComponentLoader::GetInstance().GetDHTypeBySrcSaId(saId);
    EXPECT_EQ(ret, DHType::UNKNOWN);
}
} // namespace DistributedHardware
} // namespace OHOS
