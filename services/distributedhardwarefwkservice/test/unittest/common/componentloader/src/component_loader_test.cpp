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

HWTEST_F(ComponentLoaderTest, CheckComponentEnable_001, TestSize.Level1)
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
HWTEST_F(ComponentLoaderTest, GetLocalDHVersion_001, TestSize.Level1)
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
HWTEST_F(ComponentLoaderTest, GetHardwareHandler_001, TestSize.Level1)
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

#ifdef DHARDWARE_CLOSE_UT
    /**
     * @tc.name: GetHardwareHandler_002
     * @tc.desc: Verify the GetHardwareHandler function.
     * @tc.type: FUNC
     * @tc.require: AR000GHSK3
     */
    HWTEST_F(ComponentLoaderTest, GetHardwareHandler_002, TestSize.Level1)
    {
        ComponentLoader::GetInstance().Init();
        IHardwareHandler *hardwareHandlerPtr = nullptr;
        auto ret = ComponentLoader::GetInstance().GetHardwareHandler(DHType::AUDIO, hardwareHandlerPtr);
        EXPECT_EQ(DH_FWK_SUCCESS, ret);
        ComponentLoader::GetInstance().UnInit();
    }
#endif

/**
 * @tc.name: GetSource_001
 * @tc.desc: Verify the GetSource function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, GetSource_001, TestSize.Level1)
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

#ifdef DHARDWARE_CLOSE_UT
    /**
     * @tc.name: GetSource_002
     * @tc.desc: Verify the GetSource function.
     * @tc.type: FUNC
     * @tc.require: AR000GHSK3
     */
    HWTEST_F(ComponentLoaderTest, GetSource_002, TestSize.Level1)
    {
        ComponentLoader::GetInstance().Init();
        IDistributedHardwareSource *sourcePtr = nullptr;
        auto ret = ComponentLoader::GetInstance().GetSource(DHType::AUDIO, sourcePtr);
        EXPECT_EQ(DH_FWK_SUCCESS, ret);
        ComponentLoader::GetInstance().UnInit();
    }
#endif

/**
 * @tc.name: GetSink_001
 * @tc.desc: Verify the GetSink function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, GetSink_001, TestSize.Level1)
{
    IDistributedHardwareSink *sinkPtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetSink(DHType::UNKNOWN, sinkPtr);
    EXPECT_EQ(ERR_DH_FWK_LOADER_HANDLER_IS_NULL, ret);

    CompHandler comHandler;
    comHandler.sinkHandler = nullptr;
    ComponentLoader::GetInstance().compHandlerMap_[DHType::AUDIO] = comHandler;
    ret = ComponentLoader::GetInstance().GetSink(DHType::AUDIO, sinkPtr);
    EXPECT_EQ(ERR_DH_FWK_LOADER_HANDLER_IS_NULL, ret);
    ComponentLoader::GetInstance().compHandlerMap_.clear();
}

#ifdef DHARDWARE_CLOSE_UT
    /**
     * @tc.name: GetSink_002
     * @tc.desc: Verify the GetSink function.
     * @tc.type: FUNC
     * @tc.require: AR000GHSK3
     */
    HWTEST_F(ComponentLoaderTest, GetSink_002, TestSize.Level1)
    {
        ComponentLoader::GetInstance().Init();
        IDistributedHardwareSink *sinkPtr = nullptr;
        auto ret = ComponentLoader::GetInstance().GetSink(DHType::AUDIO, sinkPtr);
        EXPECT_EQ(DH_FWK_SUCCESS, ret);
        ComponentLoader::GetInstance().UnInit();
    }
#endif

/**
 * @tc.name: Readfile_001
 * @tc.desc: Verify the Readfile function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, Readfile_001, TestSize.Level1)
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
HWTEST_F(ComponentLoaderTest, ReleaseHardwareHandler_001, TestSize.Level1)
{
    auto ret = ComponentLoader::GetInstance().ReleaseHardwareHandler(DHType::AUDIO);
    EXPECT_EQ(ERR_DH_FWK_TYPE_NOT_EXIST, ret);

    CompHandler comHandler;
    comHandler.hardwareHandler = nullptr;
    ComponentLoader::GetInstance().compHandlerMap_[DHType::AUDIO] = comHandler;
    ret = ComponentLoader::GetInstance().ReleaseHardwareHandler(DHType::AUDIO);
    EXPECT_EQ(ERR_DH_FWK_LOADER_HANDLER_UNLOAD, ret);
    ComponentLoader::GetInstance().compHandlerMap_.clear();
}

/**
 * @tc.name: ReleaseSource_001
 * @tc.desc: Verify the ReleaseSource function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, ReleaseSource_001, TestSize.Level1)
{
    auto ret = ComponentLoader::GetInstance().ReleaseSource(DHType::AUDIO);
    EXPECT_EQ(ERR_DH_FWK_TYPE_NOT_EXIST, ret);

    CompHandler comHandler;
    comHandler.sourceHandler = nullptr;
    ComponentLoader::GetInstance().compHandlerMap_[DHType::AUDIO] = comHandler;
    ret = ComponentLoader::GetInstance().ReleaseSource(DHType::AUDIO);
    EXPECT_EQ(ERR_DH_FWK_LOADER_SOURCE_UNLOAD, ret);
    ComponentLoader::GetInstance().compHandlerMap_.clear();
}

/**
 * @tc.name: ReleaseSink_001
 * @tc.desc: Verify the ReleaseSink function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, ReleaseSink_001, TestSize.Level1)
{
    auto ret = ComponentLoader::GetInstance().ReleaseSink(DHType::AUDIO);
    EXPECT_EQ(ERR_DH_FWK_TYPE_NOT_EXIST, ret);

    CompHandler comHandler;
    comHandler.sinkHandler = nullptr;
    ComponentLoader::GetInstance().compHandlerMap_[DHType::AUDIO] = comHandler;
    ret = ComponentLoader::GetInstance().ReleaseSink(DHType::AUDIO);
    EXPECT_EQ(ERR_DH_FWK_LOADER_SINK_UNLOAD, ret);
    ComponentLoader::GetInstance().compHandlerMap_.clear();
}

/**
 * @tc.name: GetHandler_001
 * @tc.desc: Verify the GetHandler function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, GetHandler_001, TestSize.Level1)
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
HWTEST_F(ComponentLoaderTest, component_loader_test_017, TestSize.Level1)
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
HWTEST_F(ComponentLoaderTest, component_loader_test_018, TestSize.Level1)
{
    const char *NAME = "NAME";
    const char *TYPE = "TYPE";
    const char *PATH = "PATH";
    cJSON* json0bject = cJSON_CreateObject();
    ASSERT_TRUE(json0bject != nullptr);
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
HWTEST_F(ComponentLoaderTest, IsDHTypeExist_001, TestSize.Level1)
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
HWTEST_F(ComponentLoaderTest, GetSourceSaId_001, TestSize.Level1)
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
 * @tc.name: GetSinkSaId_001
 * @tc.desc: Verify the GetSinkSaId function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, GetSinkSaId_001, TestSize.Level1)
{
    CompHandler comHandler;
    ComponentLoader::GetInstance().compHandlerMap_[DHType::AUDIO] = comHandler;
    int32_t ret = ComponentLoader::GetInstance().GetSinkSaId(DHType::UNKNOWN);
    EXPECT_EQ(DEFAULT_SA_ID, ret);

    comHandler.sinkSaId = 2;
    ComponentLoader::GetInstance().compHandlerMap_[DHType::AUDIO] = comHandler;
    ret = ComponentLoader::GetInstance().GetSinkSaId(DHType::AUDIO);
    EXPECT_EQ(2, ret);
}

/**
 * @tc.name: component_loader_test_022
 * @tc.desc: Verify the ParseConfig function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_022, TestSize.Level1)
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
HWTEST_F(ComponentLoaderTest, component_loader_test_023, TestSize.Level1)
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
HWTEST_F(ComponentLoaderTest, component_loader_test_024, TestSize.Level1)
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
HWTEST_F(ComponentLoaderTest, component_loader_test_025, TestSize.Level1)
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
HWTEST_F(ComponentLoaderTest, component_loader_test_026, TestSize.Level1)
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
HWTEST_F(ComponentLoaderTest, component_loader_test_027, TestSize.Level1)
{
    DHType dhType = DHType::GPS;
    bool ret = ComponentLoader::GetInstance().IsDHTypeExist(dhType);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: component_loader_test_028
 * @tc.desc: Verify the GetDHTypeBySaId function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_028, TestSize.Level1)
{
    int32_t saId = 4801;
    DHType dhType = ComponentLoader::GetInstance().GetDHTypeBySaId(saId);
    EXPECT_EQ(dhType, DHType::UNKNOWN);
}

HWTEST_F(ComponentLoaderTest, ParseResourceDesc_001, TestSize.Level1)
{
    CompConfig config;
    cJSON *component = cJSON_CreateObject();
    ASSERT_TRUE(component != nullptr);
    cJSON_AddNumberToObject(component, COMP_NAME, 1);
    cJSON_AddNumberToObject(component, COMP_TYPE, 1);
    cJSON_AddNumberToObject(component, COMP_HANDLER_LOC, 1);
    cJSON_AddNumberToObject(component, COMP_HANDLER_VERSION, 1.0);
    cJSON_AddNumberToObject(component, COMP_SOURCE_LOC, 1);
    cJSON_AddNumberToObject(component, COMP_SOURCE_VERSION, 1.0);
    cJSON_AddStringToObject(component, COMP_SOURCE_SA_ID, "4801");
    cJSON_AddNumberToObject(component, COMP_SINK_LOC, 1);
    cJSON_AddNumberToObject(component, COMP_SINK_VERSION, 1.0);
    cJSON_AddStringToObject(component, COMP_SINK_SA_ID, "4802");
    cJSON_AddStringToObject(component, COMP_RESOURCE_DESC, "comp_resource_desc");
    ComponentLoader::GetInstance().ParseCompConfigFromJson(component, config);
    cJSON_Delete(component);
}

HWTEST_F(ComponentLoaderTest, ParseResourceDesc_002, TestSize.Level1)
{
    CompConfig config1;
    cJSON *component1 = cJSON_CreateObject();
    ASSERT_TRUE(component1 != nullptr);
    cJSON_AddStringToObject(component1, COMP_NAME, "comp_name_test");
    cJSON_AddStringToObject(component1, COMP_TYPE, "comp_type_test");
    cJSON_AddStringToObject(component1, COMP_HANDLER_LOC, "comp_handler_loc_test");
    cJSON_AddStringToObject(component1, COMP_HANDLER_VERSION, "comp_handler_version_test");
    cJSON_AddStringToObject(component1, COMP_SOURCE_LOC, "comp_source_loc_test");
    cJSON_AddStringToObject(component1, COMP_SOURCE_VERSION, "comp_source_verison_test");
    cJSON_AddNumberToObject(component1, COMP_SOURCE_SA_ID, 4801);
    cJSON_AddStringToObject(component1, COMP_SINK_LOC, "comp_sink_loc_test");
    cJSON_AddStringToObject(component1, COMP_SINK_VERSION, "com_sink_version_test");
    cJSON_AddNumberToObject(component1, COMP_SINK_SA_ID, 4802);
    cJSON_AddStringToObject(component1, COMP_RESOURCE_DESC, "comp_resource_desc");
    ASSERT_NO_FATAL_FAILURE(ComponentLoader::GetInstance().ParseCompConfigFromJson(component1, config1));
    cJSON_Delete(component1);
}

HWTEST_F(ComponentLoaderTest, ParseResourceDescFromJson_003, TestSize.Level1)
{
    CompConfig config;
    cJSON *resourceDescs = cJSON_CreateArray();
    ASSERT_TRUE(resourceDescs != nullptr);
    cJSON *sensitive = cJSON_CreateObject();
    if (sensitive == nullptr) {
        cJSON_Delete(resourceDescs);
        return;
    }
    cJSON_AddBoolToObject(sensitive, COMP_SENSITIVE, true);
    cJSON_AddItemToArray(resourceDescs, sensitive);
    cJSON *subtype = cJSON_CreateObject();
    if (subtype == nullptr) {
        cJSON_Delete(resourceDescs);
        return;
    }
    cJSON_AddBoolToObject(subtype, COMP_SUBTYPE, true);
    cJSON_AddItemToArray(resourceDescs, subtype);
    ComponentLoader::GetInstance().ParseResourceDescFromJson(resourceDescs, config);
    cJSON_Delete(resourceDescs);
    EXPECT_TRUE(config.compResourceDesc.empty());
}

HWTEST_F(ComponentLoaderTest, GetHardwareHandler_003, TestSize.Level1)
{
    ComponentLoader::GetInstance().compHandlerMap_.clear();
    DHType dhType = DHType::AUDIO;
    IHardwareHandler *hardwareHandlerPtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetHardwareHandler(dhType, hardwareHandlerPtr);
    EXPECT_EQ(ret, ERR_DH_FWK_LOADER_HANDLER_IS_NULL);
}

HWTEST_F(ComponentLoaderTest, GetSource_003, TestSize.Level1)
{
    ComponentLoader::GetInstance().compHandlerMap_.clear();
    DHType dhType = DHType::AUDIO;
    IDistributedHardwareSource *dhSourcePtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetSource(dhType, dhSourcePtr);
    EXPECT_EQ(ret, ERR_DH_FWK_LOADER_HANDLER_IS_NULL);
}

HWTEST_F(ComponentLoaderTest, GetSink_003, TestSize.Level1)
{
    ComponentLoader::GetInstance().compHandlerMap_.clear();
    DHType dhType = DHType::AUDIO;
    IDistributedHardwareSink *dhSinkPtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetSink(dhType, dhSinkPtr);
    EXPECT_EQ(ret, ERR_DH_FWK_LOADER_HANDLER_IS_NULL);
}

HWTEST_F(ComponentLoaderTest, ReleaseHardwareHandler_002, TestSize.Level1)
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

    ret = ComponentLoader::GetInstance().GetSinkSaId(dhType);
    EXPECT_EQ(ret, DEFAULT_SA_ID);
}

HWTEST_F(ComponentLoaderTest, GetDHTypeBySaId_001, TestSize.Level1)
{
    ComponentLoader::GetInstance().compHandlerMap_.clear();
    int32_t saId = 4801;
    auto ret = ComponentLoader::GetInstance().GetDHTypeBySaId(saId);
    EXPECT_EQ(ret, DHType::UNKNOWN);
}

HWTEST_F(ComponentLoaderTest, GetSource_004, TestSize.Level1)
{
    auto ret = ComponentLoader::GetInstance().GetSource(DHType::AUDIO);
    EXPECT_EQ(ret, ERR_DH_FWK_LOADER_HANDLER_IS_NULL);
}

HWTEST_F(ComponentLoaderTest, GetSink_004, TestSize.Level1)
{
    auto ret = ComponentLoader::GetInstance().GetSink(DHType::AUDIO);
    EXPECT_EQ(ret, ERR_DH_FWK_LOADER_HANDLER_IS_NULL);
}

HWTEST_F(ComponentLoaderTest, GetHardwareHandler_004, TestSize.Level1)
{
    auto ret = ComponentLoader::GetInstance().GetHardwareHandler(DHType::AUDIO);
    EXPECT_EQ(ret, ERR_DH_FWK_LOADER_HANDLER_IS_NULL);
}

HWTEST_F(ComponentLoaderTest, IsDHTypeSupport_001, TestSize.Level1)
{
    auto ret = ComponentLoader::GetInstance().IsDHTypeSupport(DHType::AUDIO);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: GetDHTypeBySaId_002
 * @tc.desc: Verify the GetDHTypeBySaId function matches sourceSaId and sinkSaId.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, GetDHTypeBySaId_002, TestSize.Level1)
{
    ComponentLoader::GetInstance().compHandlerMap_.clear();
    CompHandler comHandler;
    comHandler.type = DHType::AUDIO;
    comHandler.sourceSaId = 4805;
    comHandler.sinkSaId = 4806;
    ComponentLoader::GetInstance().compHandlerMap_[DHType::AUDIO] = comHandler;
    EXPECT_EQ(DHType::AUDIO, ComponentLoader::GetInstance().GetDHTypeBySaId(4805));
    EXPECT_EQ(DHType::AUDIO, ComponentLoader::GetInstance().GetDHTypeBySaId(4806));
    EXPECT_EQ(DHType::UNKNOWN, ComponentLoader::GetInstance().GetDHTypeBySaId(4807));
}

HWTEST_F(ComponentLoaderTest, ParseSourceFeatureFiltersFromJson_001, TestSize.Level1)
{
    CompConfig config;
    cJSON *srcFilters = cJSON_CreateArray();
    ASSERT_TRUE(srcFilters != nullptr);
    ComponentLoader::GetInstance().ParseSourceFeatureFiltersFromJson(srcFilters, config);
    EXPECT_TRUE(config.sourceFeatureFilters.empty());
    cJSON_Delete(srcFilters);

    cJSON *srcFilters1 = cJSON_CreateArray();
    ASSERT_TRUE(srcFilters1 != nullptr);
    cJSON_AddItemToArray(srcFilters1, cJSON_CreateString("dcamera_1"));
    cJSON_AddItemToArray(srcFilters1, cJSON_CreateNumber(1));
    ComponentLoader::GetInstance().ParseSourceFeatureFiltersFromJson(srcFilters1, config);
    EXPECT_FALSE(config.sourceFeatureFilters.empty());
    cJSON_Delete(srcFilters1);
}

HWTEST_F(ComponentLoaderTest, ParseSinkSupportedFeaturesFromJson_001, TestSize.Level1)
{
    CompConfig config;
    cJSON *sinkFilters = cJSON_CreateArray();
    ASSERT_TRUE(sinkFilters != nullptr);
    ComponentLoader::GetInstance().ParseSinkSupportedFeaturesFromJson(sinkFilters, config);
    EXPECT_TRUE(config.sinkSupportedFeatures.empty());
    cJSON_Delete(sinkFilters);

    cJSON *sinkFilters1 = cJSON_CreateArray();
    ASSERT_TRUE(sinkFilters1 != nullptr);
    cJSON_AddItemToArray(sinkFilters1, cJSON_CreateString("dcamera_1"));
    cJSON_AddItemToArray(sinkFilters1, cJSON_CreateNumber(1));
    ComponentLoader::GetInstance().ParseSinkSupportedFeaturesFromJson(sinkFilters1, config);
    EXPECT_FALSE(config.sinkSupportedFeatures.empty());
    cJSON_Delete(sinkFilters1);
}

HWTEST_F(ComponentLoaderTest, ParseComponentEnableConfig_001, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    cJSON *root = cJSON_CreateObject();
    cJSON *audio = cJSON_CreateObject();
    cJSON *mic = cJSON_CreateObject();
    cJSON_AddBoolToObject(mic, "sink", false);
    cJSON_AddBoolToObject(mic, "source", true);
    cJSON *speaker = cJSON_CreateObject();
    cJSON_AddBoolToObject(speaker, "sink", true);
    cJSON_AddBoolToObject(speaker, "source", true);
    cJSON_AddItemToObject(audio, "mic", mic);
    cJSON_AddItemToObject(audio, "speaker", speaker);
    cJSON_AddItemToObject(root, "AUDIO", audio);
    ComponentLoader::GetInstance().ParseComponentEnableConfig(root);
    ComponentLoader::GetInstance().isLocalVersionInit_.store(true);
    EXPECT_FALSE(ComponentLoader::GetInstance().IsComponentSubtypeEnabled(DHType::AUDIO, "mic", "sink"));
    EXPECT_TRUE(ComponentLoader::GetInstance().IsComponentSubtypeEnabled(DHType::AUDIO, "mic", "source"));
    EXPECT_TRUE(ComponentLoader::GetInstance().IsComponentSubtypeEnabled(DHType::AUDIO, "speaker", "sink"));
    ComponentLoader::GetInstance().isLocalVersionInit_.store(false);
    cJSON_Delete(root);
    ComponentLoader::GetInstance().componentEnableMap_.clear();
}

HWTEST_F(ComponentLoaderTest, ParseComponentEnableConfig_002, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    cJSON *root = cJSON_CreateObject();
    ComponentLoader::GetInstance().ParseComponentEnableConfig(root);
    EXPECT_TRUE(ComponentLoader::GetInstance().IsComponentSubtypeEnabled(DHType::AUDIO, "mic", "sink"));
    EXPECT_TRUE(ComponentLoader::GetInstance().IsComponentSubtypeEnabled(DHType::AUDIO, "mic", "source"));
    EXPECT_TRUE(ComponentLoader::GetInstance().IsComponentSubtypeEnabled(DHType::AUDIO, "speaker", "sink"));
    cJSON_Delete(root);
    ComponentLoader::GetInstance().componentEnableMap_.clear();
}

HWTEST_F(ComponentLoaderTest, ParseComponentEnableConfig_003, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    cJSON *root = cJSON_CreateObject();
    cJSON *audio = cJSON_CreateObject();
    cJSON *mic = cJSON_CreateObject();
    cJSON_AddBoolToObject(mic, "sink", false);
    cJSON_AddBoolToObject(mic, "source", false);
    cJSON *speaker = cJSON_CreateObject();
    cJSON_AddBoolToObject(speaker, "sink", false);
    cJSON_AddBoolToObject(speaker, "source", false);
    cJSON_AddItemToObject(audio, "mic", mic);
    cJSON_AddItemToObject(audio, "speaker", speaker);
    cJSON_AddItemToObject(root, "AUDIO", audio);
    ComponentLoader::GetInstance().ParseComponentEnableConfig(root);
    ComponentLoader::GetInstance().isLocalVersionInit_.store(true);
    EXPECT_FALSE(ComponentLoader::GetInstance().IsComponentSubtypeEnabled(DHType::AUDIO, "mic", "sink"));
    EXPECT_FALSE(ComponentLoader::GetInstance().IsComponentSubtypeEnabled(DHType::AUDIO, "mic", "source"));
    EXPECT_FALSE(ComponentLoader::GetInstance().IsComponentSubtypeEnabled(DHType::AUDIO, "speaker", "source"));
    ComponentLoader::GetInstance().isLocalVersionInit_.store(false);
    cJSON_Delete(root);
    ComponentLoader::GetInstance().componentEnableMap_.clear();
}

HWTEST_F(ComponentLoaderTest, AudioCompConfig_001, TestSize.Level1)
{
    EXPECT_STREQ(g_audioCompConfig.name.c_str(), AUDIO_COMP_NAME);
    EXPECT_EQ(g_audioCompConfig.type, DHType::AUDIO);
    EXPECT_EQ(g_audioCompConfig.compSourceSaId, AUDIO_SOURCE_SA_ID);
    EXPECT_EQ(g_audioCompConfig.compSinkSaId, AUDIO_SINK_SA_ID);
    EXPECT_STREQ(g_audioCompConfig.compHandlerLoc.c_str(), AUDIO_HANDLER_LOC);
    EXPECT_STREQ(g_audioCompConfig.compSourceLoc.c_str(), AUDIO_SOURCE_LOC);
    EXPECT_STREQ(g_audioCompConfig.compSinkLoc.c_str(), AUDIO_SINK_LOC);
}

HWTEST_F(ComponentLoaderTest, IsComponentSubtypeEnabled_001, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "mic"}]["sink"] = false;
    ComponentLoader::GetInstance().isLocalVersionInit_.store(true);
    EXPECT_FALSE(ComponentLoader::GetInstance().IsComponentSubtypeEnabled(DHType::AUDIO, "mic", "sink"));
    EXPECT_TRUE(ComponentLoader::GetInstance().IsComponentSubtypeEnabled(DHType::AUDIO, "mic", "source"));
    EXPECT_TRUE(ComponentLoader::GetInstance().IsComponentSubtypeEnabled(DHType::AUDIO, "speaker", "sink"));
    ComponentLoader::GetInstance().isLocalVersionInit_.store(false);
    ComponentLoader::GetInstance().componentEnableMap_.clear();
}

HWTEST_F(ComponentLoaderTest, ParseComponentEnableConfig_004, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    ComponentLoader::GetInstance().ParseComponentEnableConfig(nullptr);
    EXPECT_TRUE(ComponentLoader::GetInstance().componentEnableMap_.empty());
    EXPECT_TRUE(ComponentLoader::GetInstance().IsComponentSubtypeEnabled(DHType::AUDIO, "mic", "sink"));
    ComponentLoader::GetInstance().componentEnableMap_.clear();
}

HWTEST_F(ComponentLoaderTest, ParseComponentEnableConfig_005, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    cJSON *root = cJSON_CreateObject();
    cJSON *audio = cJSON_CreateObject();
    cJSON_AddBoolToObject(audio, "mic", false);
    cJSON_AddBoolToObject(audio, "speaker", true);
    cJSON_AddItemToObject(root, "AUDIO", audio);
    ComponentLoader::GetInstance().ParseComponentEnableConfig(root);
    ComponentLoader::GetInstance().isLocalVersionInit_.store(true);
    EXPECT_FALSE(ComponentLoader::GetInstance().IsComponentSubtypeEnabled(DHType::AUDIO, "mic", "sink"));
    EXPECT_FALSE(ComponentLoader::GetInstance().IsComponentSubtypeEnabled(DHType::AUDIO, "mic", "source"));
    EXPECT_TRUE(ComponentLoader::GetInstance().IsComponentSubtypeEnabled(DHType::AUDIO, "speaker", "sink"));
    EXPECT_TRUE(ComponentLoader::GetInstance().IsComponentSubtypeEnabled(DHType::AUDIO, "speaker", "source"));
    ComponentLoader::GetInstance().isLocalVersionInit_.store(false);
    cJSON_Delete(root);
    ComponentLoader::GetInstance().componentEnableMap_.clear();
}

HWTEST_F(ComponentLoaderTest, ParseComponentEnableConfig_006, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    cJSON *root = cJSON_CreateObject();
    cJSON *audio = cJSON_CreateObject();
    cJSON_AddNumberToObject(audio, "mic", 123);
    cJSON_AddItemToObject(root, "AUDIO", audio);
    ComponentLoader::GetInstance().ParseComponentEnableConfig(root);
    EXPECT_TRUE(ComponentLoader::GetInstance().componentEnableMap_.empty());
    EXPECT_TRUE(ComponentLoader::GetInstance().IsComponentSubtypeEnabled(DHType::AUDIO, "mic", "sink"));
    cJSON_Delete(root);
    ComponentLoader::GetInstance().componentEnableMap_.clear();
}

HWTEST_F(ComponentLoaderTest, IsSubtypeAnyRoleEnabled_001, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    EXPECT_FALSE(ComponentLoader::GetInstance().IsSubtypeAnyRoleEnabled("mic"));
    EXPECT_FALSE(ComponentLoader::GetInstance().IsSubtypeAnyRoleEnabled("speaker"));
    ComponentLoader::GetInstance().componentEnableMap_.clear();
}

HWTEST_F(ComponentLoaderTest, IsSubtypeAnyRoleEnabled_002, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "mic"}]["sink"] = false;
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "mic"}]["source"] = false;
    EXPECT_FALSE(ComponentLoader::GetInstance().IsSubtypeAnyRoleEnabled("mic"));
    ComponentLoader::GetInstance().componentEnableMap_.clear();
}

HWTEST_F(ComponentLoaderTest, IsSubtypeAnyRoleEnabled_003, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "mic"}]["sink"] = false;
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "mic"}]["source"] = true;
    EXPECT_TRUE(ComponentLoader::GetInstance().IsSubtypeAnyRoleEnabled("mic"));
    ComponentLoader::GetInstance().componentEnableMap_.clear();
}

HWTEST_F(ComponentLoaderTest, RegisterAudioComponentIfNeeded_001, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    std::map<DHType, CompConfig> dhtypeMap;
    ComponentLoader::GetInstance().RegisterAudioComponentIfNeeded(dhtypeMap);
    EXPECT_NE(dhtypeMap.find(DHType::AUDIO), dhtypeMap.end());
    EXPECT_EQ(dhtypeMap[DHType::AUDIO].name, AUDIO_COMP_NAME);
    ComponentLoader::GetInstance().componentEnableMap_.clear();
}

HWTEST_F(ComponentLoaderTest, RegisterAudioComponentIfNeeded_002, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "mic"}]["sink"] = false;
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "mic"}]["source"] = false;
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "speaker"}]["sink"] = false;
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "speaker"}]["source"] = false;
    std::map<DHType, CompConfig> dhtypeMap;
    dhtypeMap[DHType::AUDIO] = g_audioCompConfig;
    ComponentLoader::GetInstance().RegisterAudioComponentIfNeeded(dhtypeMap);
    EXPECT_EQ(dhtypeMap.find(DHType::AUDIO), dhtypeMap.end());
    ComponentLoader::GetInstance().componentEnableMap_.clear();
}

HWTEST_F(ComponentLoaderTest, RegisterAudioComponentIfNeeded_003, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "mic"}]["sink"] = false;
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "mic"}]["source"] = false;
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "speaker"}]["sink"] = true;
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "speaker"}]["source"] = true;
    std::map<DHType, CompConfig> dhtypeMap;
    ComponentLoader::GetInstance().RegisterAudioComponentIfNeeded(dhtypeMap);
    EXPECT_NE(dhtypeMap.find(DHType::AUDIO), dhtypeMap.end());
    ComponentLoader::GetInstance().componentEnableMap_.clear();
}

HWTEST_F(ComponentLoaderTest, RegisterAudioComponentIfNeeded_004, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "mic"}]["sink"] = true;
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "mic"}]["source"] = true;
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "speaker"}]["sink"] = false;
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "speaker"}]["source"] = false;
    std::map<DHType, CompConfig> dhtypeMap;
    ComponentLoader::GetInstance().RegisterAudioComponentIfNeeded(dhtypeMap);
    EXPECT_NE(dhtypeMap.find(DHType::AUDIO), dhtypeMap.end());
    ComponentLoader::GetInstance().componentEnableMap_.clear();
}

HWTEST_F(ComponentLoaderTest, RegisterAudioComponentIfNeeded_005, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    ComponentLoader::GetInstance().localDHVersion_.compVersions.clear();
    std::map<DHType, CompConfig> dhtypeMap;
    ComponentLoader::GetInstance().RegisterAudioComponentIfNeeded(dhtypeMap);
    EXPECT_NE(dhtypeMap.find(DHType::AUDIO), dhtypeMap.end());
    EXPECT_NE(ComponentLoader::GetInstance().localDHVersion_.compVersions.find(DHType::AUDIO),
        ComponentLoader::GetInstance().localDHVersion_.compVersions.end());
    auto &compVersion = ComponentLoader::GetInstance().localDHVersion_.compVersions[DHType::AUDIO];
    EXPECT_EQ(compVersion.name, AUDIO_COMP_NAME);
    EXPECT_EQ(compVersion.sourceVersion, AUDIO_SOURCE_VERSION);
    EXPECT_EQ(compVersion.sinkVersion, AUDIO_SINK_VERSION);
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    ComponentLoader::GetInstance().localDHVersion_.compVersions.clear();
}

HWTEST_F(ComponentLoaderTest, RegisterAudioComponentIfNeeded_006, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    ComponentLoader::GetInstance().localDHVersion_.compVersions.clear();
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "mic"}]["sink"] = false;
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "mic"}]["source"] = false;
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "speaker"}]["sink"] = false;
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "speaker"}]["source"] = false;
    CompVersion preVersion;
    preVersion.dhType = DHType::AUDIO;
    ComponentLoader::GetInstance().localDHVersion_.compVersions[DHType::AUDIO] = preVersion;
    std::map<DHType, CompConfig> dhtypeMap;
    ComponentLoader::GetInstance().RegisterAudioComponentIfNeeded(dhtypeMap);
    EXPECT_EQ(dhtypeMap.find(DHType::AUDIO), dhtypeMap.end());
    EXPECT_EQ(ComponentLoader::GetInstance().localDHVersion_.compVersions.find(DHType::AUDIO),
        ComponentLoader::GetInstance().localDHVersion_.compVersions.end());
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    ComponentLoader::GetInstance().localDHVersion_.compVersions.clear();
}

HWTEST_F(ComponentLoaderTest, RegisterAudioComponentIfNeeded_007, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    ComponentLoader::GetInstance().localDHVersion_.compVersions.clear();
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "mic"}]["sink"] = false;
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "mic"}]["source"] = false;
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "speaker"}]["sink"] = true;
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "speaker"}]["source"] = true;
    std::map<DHType, CompConfig> dhtypeMap;
    ComponentLoader::GetInstance().RegisterAudioComponentIfNeeded(dhtypeMap);
    EXPECT_NE(dhtypeMap.find(DHType::AUDIO), dhtypeMap.end());
    EXPECT_NE(ComponentLoader::GetInstance().localDHVersion_.compVersions.find(DHType::AUDIO),
        ComponentLoader::GetInstance().localDHVersion_.compVersions.end());
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    ComponentLoader::GetInstance().localDHVersion_.compVersions.clear();
}

HWTEST_F(ComponentLoaderTest, RegisterAudioComponentIfNeeded_008, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    ComponentLoader::GetInstance().localDHVersion_.compVersions.clear();
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "mic"}]["sink"] = true;
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "mic"}]["source"] = true;
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "speaker"}]["sink"] = false;
    ComponentLoader::GetInstance().componentEnableMap_[{DHType::AUDIO, "speaker"}]["source"] = false;
    std::map<DHType, CompConfig> dhtypeMap;
    ComponentLoader::GetInstance().RegisterAudioComponentIfNeeded(dhtypeMap);
    EXPECT_NE(dhtypeMap.find(DHType::AUDIO), dhtypeMap.end());
    EXPECT_NE(ComponentLoader::GetInstance().localDHVersion_.compVersions.find(DHType::AUDIO),
        ComponentLoader::GetInstance().localDHVersion_.compVersions.end());
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    ComponentLoader::GetInstance().localDHVersion_.compVersions.clear();
}

HWTEST_F(ComponentLoaderTest, AudioCompConfig_002, TestSize.Level1)
{
    EXPECT_STREQ(g_audioCompConfig.compHandlerVersion.c_str(), AUDIO_HANDLER_VERSION);
    EXPECT_STREQ(g_audioCompConfig.compSourceVersion.c_str(), AUDIO_SOURCE_VERSION);
    EXPECT_STREQ(g_audioCompConfig.compSinkVersion.c_str(), AUDIO_SINK_VERSION);
    EXPECT_TRUE(g_audioCompConfig.haveFeature);
    EXPECT_TRUE(g_audioCompConfig.sourceFeatureFilters.empty());
    EXPECT_TRUE(g_audioCompConfig.sinkSupportedFeatures.empty());
    EXPECT_EQ(g_audioCompConfig.compResourceDesc.size(), 2u);
    EXPECT_STREQ(g_audioCompConfig.compResourceDesc[0].subtype.c_str(), "mic");
    EXPECT_TRUE(g_audioCompConfig.compResourceDesc[0].sensitiveValue);
    EXPECT_STREQ(g_audioCompConfig.compResourceDesc[1].subtype.c_str(), "speaker");
    EXPECT_TRUE(g_audioCompConfig.compResourceDesc[1].sensitiveValue);
}

HWTEST_F(ComponentLoaderTest, ParseComponentEnableConfig_nullptr_typeEntry, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToArray(root, cJSON_CreateNull());
    ComponentLoader::GetInstance().ParseComponentEnableConfig(root);
    EXPECT_TRUE(ComponentLoader::GetInstance().componentEnableMap_.empty());
    cJSON_Delete(root);
    ComponentLoader::GetInstance().componentEnableMap_.clear();
}

HWTEST_F(ComponentLoaderTest, ParseComponentEnableConfig_nullptr_subEntry, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    cJSON *root = cJSON_CreateObject();
    cJSON *audio = cJSON_CreateObject();
    cJSON_AddItemToArray(audio, cJSON_CreateNull());
    cJSON_AddItemToObject(root, "AUDIO", audio);
    ComponentLoader::GetInstance().ParseComponentEnableConfig(root);
    EXPECT_TRUE(ComponentLoader::GetInstance().componentEnableMap_.empty());
    cJSON_Delete(root);
    ComponentLoader::GetInstance().componentEnableMap_.clear();
}

HWTEST_F(ComponentLoaderTest, ParseComponentEnableConfig_nullptr_roleEntry, TestSize.Level1)
{
    ComponentLoader::GetInstance().componentEnableMap_.clear();
    cJSON *root = cJSON_CreateObject();
    cJSON *audio = cJSON_CreateObject();
    cJSON *mic = cJSON_CreateObject();
    cJSON_AddItemToArray(mic, cJSON_CreateNull());
    cJSON_AddItemToObject(audio, "mic", mic);
    cJSON_AddItemToObject(root, "AUDIO", audio);
    ComponentLoader::GetInstance().ParseComponentEnableConfig(root);
    EXPECT_TRUE(ComponentLoader::GetInstance().componentEnableMap_.empty());
    cJSON_Delete(root);
    ComponentLoader::GetInstance().componentEnableMap_.clear();
}
} // namespace DistributedHardware
} // namespace OHOS
