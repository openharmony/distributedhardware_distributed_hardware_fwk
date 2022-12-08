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

#include "component_loader_test.h"
#define private public
#include "component_loader.h"
#undef private
#include "nlohmann/json.hpp"
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
    std::string NAME = "NAME";
    std::string TYPE = "TYPE";
    std::string PATH = "PATH";
    nlohmann::json json0bject;
    nlohmann::json compVers;
    compVers[NAME] = "name";
    compVers[TYPE] = 1111;
    json0bject[PATH] = compVers;
    std::string jsonStr = json0bject.dump();
    std::map<DHType, CompConfig> dhtypeMap;
    int32_t ret = ComponentLoader::GetInstance().GetCompPathAndVersion(jsonStr, dhtypeMap);
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
    DHType dhType =  DHType::GPS;
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
    DHType dhType =  DHType::GPS;
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
    DHType dhType =  DHType::GPS;
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
    DHType dhType =  DHType::GPS;
    bool ret = ComponentLoader::GetInstance().IsDHTypeExist(dhType);
    EXPECT_EQ(false, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
