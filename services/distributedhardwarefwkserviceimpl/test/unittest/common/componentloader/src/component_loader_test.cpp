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
#include "versionmanager/version_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void ComponentLoaderTest::SetUpTestCase(void) {}

void ComponentLoaderTest::TearDownTestCase(void) {}

void ComponentLoaderTest::SetUp() {}

void ComponentLoaderTest::TearDown() {}

/**
 * @tc.name: component_loader_test_001
 * @tc.desc: Verify the Init function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_001, TestSize.Level0)
{
    auto ret = ComponentLoader::GetInstance().Init();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    EXPECT_TRUE(ComponentLoader::GetInstance().compHandlerMap_.size() != 0);
}

/**
 * @tc.name: component_loader_test_002
 * @tc.desc: Verify the GetLocalDHVersion function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_002, TestSize.Level0)
{
    DHVersion dhVersion;
    auto ret = ComponentLoader::GetInstance().GetLocalDHVersion(dhVersion);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    EXPECT_TRUE(!dhVersion.compVersions.empty());
}

/**
 * @tc.name: component_loader_test_003
 * @tc.desc: Verify the GetAllCompTypes function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_003, TestSize.Level0)
{
    std::vector<DHType> dhTypes = ComponentLoader::GetInstance().GetAllCompTypes();
    EXPECT_TRUE(dhTypes.size() != 0);
}

/**
 * @tc.name: component_loader_test_004
 * @tc.desc: Verify the GetHardwareHandler function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_004, TestSize.Level0)
{
    IHardwareHandler *hardwareHandlerPtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetHardwareHandler(DHType::CAMERA, hardwareHandlerPtr);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    EXPECT_TRUE(hardwareHandlerPtr != nullptr);
}

/**
 * @tc.name: component_loader_test_005
 * @tc.desc: Verify the GetSource function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_005, TestSize.Level0)
{
    IDistributedHardwareSource *sourcePtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetSource(DHType::CAMERA, sourcePtr);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    EXPECT_TRUE(sourcePtr != nullptr);
}

/**
 * @tc.name: component_loader_test_006
 * @tc.desc: Verify the GetSink function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_006, TestSize.Level0)
{
    IDistributedHardwareSink *sinkPtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetSink(DHType::CAMERA, sinkPtr);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    EXPECT_TRUE(sinkPtr != nullptr);
}

/**
 * @tc.name: component_loader_test_007
 * @tc.desc: Verify the ReleaseHardwareHandler function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_007, TestSize.Level0)
{
    auto ret = ComponentLoader::GetInstance().ReleaseHardwareHandler(DHType::CAMERA);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    EXPECT_TRUE(ComponentLoader::GetInstance().compHandlerMap_[DHType::CAMERA].hardwareHandler == nullptr);
}

/**
 * @tc.name: component_loader_test_008
 * @tc.desc: Verify the ReleaseSource function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_008, TestSize.Level0)
{
    auto ret = ComponentLoader::GetInstance().ReleaseSource(DHType::CAMERA);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    EXPECT_TRUE(ComponentLoader::GetInstance().compHandlerMap_[DHType::CAMERA].sourceHandler == nullptr);
}

/**
 * @tc.name: component_loader_test_009
 * @tc.desc: Verify the ReleaseSink function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(ComponentLoaderTest, component_loader_test_009, TestSize.Level0)
{
    auto ret = ComponentLoader::GetInstance().ReleaseSink(DHType::CAMERA);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    EXPECT_TRUE(ComponentLoader::GetInstance().compHandlerMap_[DHType::CAMERA].sinkHandler == nullptr);
}
}
}
