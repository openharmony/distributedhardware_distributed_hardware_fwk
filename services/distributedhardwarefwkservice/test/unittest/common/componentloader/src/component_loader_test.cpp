/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#define private public
#include "component_loader.h"
#undef private
#include "distributed_hardware_log.h"
#include "hitrace_meter.h"
#include "hidump_helper.h"
#include "kvstore_observer.h"
#include "cJSON.h"
#include "versionmanager/version_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {
    std::map<DHType, CompHandler> g_compHandlerMap;
}

void ComponentLoaderTest::SetUpTestCase(void) {}

void ComponentLoaderTest::TearDownTestCase(void) {}

void ComponentLoaderTest::SetUp()
{
    ComponentLoader::GetInstance().Init();
    g_compHandlerMap = ComponentLoader::GetInstance().compHandlerMap_;
}

void ComponentLoaderTest::TearDown()
{
    ComponentLoader::GetInstance().UnInit();
    g_compHandlerMap.clear();
    ComponentLoader::GetInstance().compHandlerMap_.clear();
}

/**
 * @tc.name: component_loader_test_001
 * @tc.desc: Verify the GetLocalDHVersion function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_001, TestSize.Level0)
{
    DHVersion dhVersion;
    auto ret = ComponentLoader::GetInstance().GetLocalDHVersion(dhVersion);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: component_loader_test_002
 * @tc.desc: Verify the GetHardwareHandler function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_002, TestSize.Level0)
{
    for (const auto &iter : g_compHandlerMap) {
        IHardwareHandler *hardwareHandlerPtr = nullptr;
        auto ret = ComponentLoader::GetInstance().GetHardwareHandler(iter.first, hardwareHandlerPtr);
        EXPECT_EQ(DH_FWK_SUCCESS, ret);
        EXPECT_TRUE(hardwareHandlerPtr != nullptr);
    }
}

/**
 * @tc.name: component_loader_test_003
 * @tc.desc: Verify the GetHardwareHandler function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_003, TestSize.Level0)
{
    ComponentLoader::GetInstance().compHandlerMap_.clear();
    DHType dhType = DHType::AUDIO;
    IHardwareHandler *hardwareHandlerPtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetHardwareHandler(dhType, hardwareHandlerPtr);
    EXPECT_EQ(ERR_DH_FWK_LOADER_HANDLER_IS_NULL, ret);
}

/**
 * @tc.name: component_loader_test_004
 * @tc.desc: Verify the GetHardwareHandler function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_004, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    CompHandler compHandler;
    IHardwareHandler *hardwareHandlerPtr = nullptr;
    ComponentLoader::GetInstance().compHandlerMap_[DHType::CAMERA] = compHandler;
    auto ret = ComponentLoader::GetInstance().GetHardwareHandler(dhType, hardwareHandlerPtr);
    EXPECT_EQ(ERR_DH_FWK_LOADER_HANDLER_IS_NULL, ret);
}

/**
 * @tc.name: component_loader_test_005
 * @tc.desc: Verify the GetSource function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_005, TestSize.Level0)
{
    for (const auto &iter : g_compHandlerMap) {
        IDistributedHardwareSource *sourcePtr = nullptr;
        auto ret = ComponentLoader::GetInstance().GetSource(iter.first, sourcePtr);
        EXPECT_EQ(DH_FWK_SUCCESS, ret);
        EXPECT_TRUE(sourcePtr != nullptr);
    }
}

/**
 * @tc.name: component_loader_test_006
 * @tc.desc: Verify the GetSource function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_006, TestSize.Level0)
{
    ComponentLoader::GetInstance().compHandlerMap_.clear();
    DHType dhType = DHType::AUDIO;
    IDistributedHardwareSource *sourcePtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetSource(dhType, sourcePtr);
    EXPECT_EQ(ERR_DH_FWK_LOADER_HANDLER_IS_NULL, ret);
}

/**
 * @tc.name: component_loader_test_007
 * @tc.desc: Verify the GetSource function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_007, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    CompHandler compHandler;
    IDistributedHardwareSource *sourcePtr = nullptr;
    ComponentLoader::GetInstance().compHandlerMap_[DHType::CAMERA] = compHandler;
    auto ret = ComponentLoader::GetInstance().GetSource(dhType, sourcePtr);
    EXPECT_EQ(ERR_DH_FWK_LOADER_HANDLER_IS_NULL, ret);
}

/**
 * @tc.name: component_loader_test_008
 * @tc.desc: Verify the GetSink function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_008, TestSize.Level0)
{
    for (const auto &iter : g_compHandlerMap) {
        IDistributedHardwareSink *sinkPtr = nullptr;
        auto ret = ComponentLoader::GetInstance().GetSink(iter.first, sinkPtr);
        EXPECT_EQ(DH_FWK_SUCCESS, ret);
        EXPECT_TRUE(sinkPtr != nullptr);
    }
}

/**
 * @tc.name: component_loader_test_009
 * @tc.desc: Verify the GetSink function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_009, TestSize.Level0)
{
    ComponentLoader::GetInstance().compHandlerMap_.clear();
    DHType dhType = DHType::AUDIO;
    IDistributedHardwareSink *sinkPtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetSink(dhType, sinkPtr);
    EXPECT_EQ(ERR_DH_FWK_LOADER_HANDLER_IS_NULL, ret);
}

/**
 * @tc.name: component_loader_test_010
 * @tc.desc: Verify the GetSink function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_010, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    CompHandler compHandler;
    IDistributedHardwareSink *sinkPtr = nullptr;
    ComponentLoader::GetInstance().compHandlerMap_[DHType::CAMERA] = compHandler;
    auto ret = ComponentLoader::GetInstance().GetSink(dhType, sinkPtr);
    EXPECT_EQ(ERR_DH_FWK_LOADER_HANDLER_IS_NULL, ret);
}

/**
 * @tc.name: component_loader_test_011
 * @tc.desc: Verify the ReleaseHardwareHandler function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_011, TestSize.Level0)
{
    for (const auto &iter : g_compHandlerMap) {
        auto ret = ComponentLoader::GetInstance().ReleaseHardwareHandler(iter.first);
        EXPECT_EQ(DH_FWK_SUCCESS, ret);
        EXPECT_TRUE(ComponentLoader::GetInstance().compHandlerMap_[iter.first].hardwareHandler == nullptr);
    }
}

/**
 * @tc.name: component_loader_test_012
 * @tc.desc: Verify the ReleaseSource function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_012, TestSize.Level0)
{
    for (const auto &iter : g_compHandlerMap) {
        auto ret = ComponentLoader::GetInstance().ReleaseSource(iter.first);
        EXPECT_EQ(DH_FWK_SUCCESS, ret);
        EXPECT_TRUE(ComponentLoader::GetInstance().compHandlerMap_[iter.first].sourceHandler == nullptr);
    }
}

/**
 * @tc.name: component_loader_test_013
 * @tc.desc: Verify the ReleaseSink function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_013, TestSize.Level0)
{
    for (const auto &iter : g_compHandlerMap) {
        auto ret = ComponentLoader::GetInstance().ReleaseSink(iter.first);
        EXPECT_EQ(DH_FWK_SUCCESS, ret);
        EXPECT_TRUE(ComponentLoader::GetInstance().compHandlerMap_[iter.first].sinkHandler == nullptr);
    }
}

/**
 * @tc.name: component_loader_test_014
 * @tc.desc: Verify the GetAllCompTypes function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_014, TestSize.Level0)
{
    auto vec = ComponentLoader::GetInstance().GetAllCompTypes();
    EXPECT_EQ(vec.size(), ComponentLoader::GetInstance().compHandlerMap_.size());
}

/**
 * @tc.name: component_loader_test_015
 * @tc.desc: Verify the GetHandler function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_015, TestSize.Level0)
{
    std::string soNameEmpty = "";
    auto handler = ComponentLoader::GetInstance().GetHandler(soNameEmpty);
    EXPECT_EQ(nullptr, handler);
}

/**
 * @tc.name: component_loader_test_016
 * @tc.desc: Verify the GetHandler function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_016, TestSize.Level0)
{
    std::string soName = "NON_EXISTENT_SO";
    auto handler = ComponentLoader::GetInstance().GetHandler(soName);
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
    EXPECT_EQ(ERR_DH_FWK_JSON_PARSE_FAILED, ret);
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
    cJSON* compVers = cJSON_CreateObject();
    cJSON_AddStringToObject(compVers, NAME, "name");
    cJSON_AddNumberToObject(compVers, TYPE, 1111);
    cJSON_AddItemToObject(json0bject, PATH, compVers);
    char* cjson = cJSON_PrintUnformatted(json0bject);
    std::string jsonStr(cjson);
    std::map<DHType, CompConfig> dhtypeMap;
    int32_t ret = ComponentLoader::GetInstance().GetCompPathAndVersion(jsonStr, dhtypeMap);
    cJSON_free(cjson);
    cJSON_Delete(json0bject);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: component_loader_test_019
 * @tc.desc: Verify the StoreLocalDHVersionInDB function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_019, TestSize.Level0)
{
    ComponentLoader::GetInstance().isLocalVersionInit_.store(false);
    ComponentLoader::GetInstance().StoreLocalDHVersionInDB();
    EXPECT_EQ(false, ComponentLoader::GetInstance().isLocalVersionInit_.load());
}

/**
 * @tc.name: component_loader_test_020
 * @tc.desc: Verify the IsDHTypeExist function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_020, TestSize.Level0)
{
    bool ret = ComponentLoader::GetInstance().IsDHTypeExist(DHType::CAMERA);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: component_loader_test_021
 * @tc.desc: Verify the GetSourceSaId function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_021, TestSize.Level0)
{
    const int32_t INVALID_SA_ID = -1;
    int32_t ret = ComponentLoader::GetInstance().GetSourceSaId(DHType::UNKNOWN);
    EXPECT_EQ(INVALID_SA_ID, ret);
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
} // namespace DistributedHardware
} // namespace OHOS
