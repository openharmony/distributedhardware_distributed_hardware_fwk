/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "component_loader.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "hdf_operate.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class HdfOperateTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

constexpr int32_t TEST_COMP_SINK_SA_ID = 4804;
constexpr int32_t TEST_SINK_SA_ID = 12345;
constexpr int32_t TEST_SOURCE_SA_ID = 12345;

void HdfOperateTest::SetUpTestCase(void)
{
    DHLOGI("HdfOperateTest::SetUpTestCase");
}

void HdfOperateTest::TearDownTestCase(void)
{
    DHLOGI("HdfOperateTest::TearDownTestCase");
}

void HdfOperateTest::SetUp(void)
{
    DHLOGI("HdfOperateTest::SetUp");
}

void HdfOperateTest::TearDown(void)
{
    DHLOGI("HdfOperateTest::TearDown");
}

static void SetUpComponentLoaderConfig()
{
    if (ComponentLoader::GetInstance().compHandlerMap_.find(DHType::AUDIO)
        == ComponentLoader::GetInstance().compHandlerMap_.end()) {
        CompHandler handler;
        handler.compConfig.name = "distributed_audio";
        handler.compConfig.type = DHType::AUDIO;
        handler.compConfig.compHandlerLoc = "libdistributed_camera_handler.z.so";
        handler.compConfig.compHandlerVersion = "1.0";
        handler.compConfig.compSourceLoc = "libdistributed_camera_source_sdk.z.so";
        handler.compConfig.compSourceVersion = "1.0";
        handler.compConfig.compSinkLoc = "libdistributed_camera_sink_sdk.z.so";
        handler.compConfig.compSinkVersion = "2.0";
        handler.compConfig.compSinkSaId = TEST_COMP_SINK_SA_ID;
        handler.compConfig.haveFeature = false;
        handler.hardwareHandler = nullptr;
        handler.sourceHandler = nullptr;
        handler.sinkHandler = nullptr;
        handler.type = DHType::AUDIO;
        handler.sinkSaId = TEST_SINK_SA_ID;
        handler.sourceSaId = TEST_SOURCE_SA_ID;
        ComponentLoader::GetInstance().compHandlerMap_[DHType::AUDIO] = handler;
    }
}

static void SetDownComponentLoaderConfig()
{
    auto itHandler = ComponentLoader::GetInstance().compHandlerMap_.find(DHType::AUDIO);
    if (itHandler != ComponentLoader::GetInstance().compHandlerMap_.end()) {
        CompHandler &handler = itHandler->second;
        if (handler.sinkSaId == TEST_SINK_SA_ID && handler.sourceSaId == TEST_SOURCE_SA_ID) {
            ComponentLoader::GetInstance().compHandlerMap_.erase(itHandler);
        }
    }
}

/**
 * @tc.name: LoadDistributedHDF_001
 * @tc.desc: Verify LoadDistributedHDF func
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(HdfOperateTest, LoadDistributedHDF_001, TestSize.Level1)
{
    DHLOGI("HdfOperateTest::LoadDistributedHDF_001");
    SetUpComponentLoaderConfig();
    EXPECT_EQ(DH_FWK_SUCCESS, HdfOperateManager::GetInstance().LoadDistributedHDF(DHType::AUDIO));
    EXPECT_EQ(DH_FWK_SUCCESS, HdfOperateManager::GetInstance().LoadDistributedHDF(DHType::AUDIO));
    EXPECT_EQ(DH_FWK_SUCCESS, HdfOperateManager::GetInstance().UnLoadDistributedHDF(DHType::AUDIO));
    EXPECT_EQ(DH_FWK_SUCCESS, HdfOperateManager::GetInstance().UnLoadDistributedHDF(DHType::AUDIO));
    SetDownComponentLoaderConfig();
}

/**
 * @tc.name: UnLoadDistributedHDF_001
 * @tc.desc: Verify UnLoadDistributedHDF func
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(HdfOperateTest, UnLoadDistributedHDF_001, TestSize.Level1)
{
    DHLOGI("HdfOperateTest::UnLoadDistributedHDF_001");
    HdfOperateManager::GetInstance().ResetRefCount(DHType::AUDIO);
    int32_t ret = HdfOperateManager::GetInstance().UnLoadDistributedHDF(DHType::AUDIO);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: ResetRefCount_001
 * @tc.desc: Verify ResetRefCount func
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(HdfOperateTest, ResetRefCount_001, TestSize.Level1)
{
    DHLOGI("HdfOperateTest::ResetRefCount_001");
    SetUpComponentLoaderConfig();
    HdfOperateManager::GetInstance().hdfOperateMap_.clear();
    HdfOperateManager::GetInstance().ResetRefCount(DHType::AUDIO);
    int32_t ret = HdfOperateManager::GetInstance().LoadDistributedHDF(DHType::AUDIO);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    HdfOperateManager::GetInstance().ResetRefCount(DHType::AUDIO);
    ret = HdfOperateManager::GetInstance().LoadDistributedHDF(DHType::AUDIO);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    ret = HdfOperateManager::GetInstance().UnLoadDistributedHDF(DHType::AUDIO);
    SetDownComponentLoaderConfig();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
