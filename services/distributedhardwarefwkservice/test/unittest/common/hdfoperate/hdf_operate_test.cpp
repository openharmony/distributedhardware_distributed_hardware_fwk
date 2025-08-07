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

#include "hdf_operate.h"

#include <gtest/gtest.h>
#include "iremote_stub.h"

#include "component_loader.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "publisher_listener_stub.h"

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

    class TestPublisherListenerStub : public OHOS::DistributedHardware::PublisherListenerStub {
    public:
        TestPublisherListenerStub() = default;
        virtual ~TestPublisherListenerStub() = default;
        void OnMessage(const DHTopic topic, const std::string& message)
        {
            (void)topic;
            (void)message;
        }
    };
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

#ifndef DHARDWARE_CLOSE_UT
/**
 * @tc.name: LoadDistributedHDF_002
 * @tc.desc: Verify LoadDistributedHDF func
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(HdfOperateTest, LoadDistributedHDF_002, TestSize.Level1)
{
    DHLOGI("HdfOperateTest::LoadDistributedHDF_002");
    EXPECT_EQ(DH_FWK_SUCCESS, HdfOperateManager::GetInstance().LoadDistributedHDF(DHType::AUDIO));
    EXPECT_EQ(DH_FWK_SUCCESS, HdfOperateManager::GetInstance().UnLoadDistributedHDF(DHType::AUDIO));
}
#endif

/**
 * @tc.name: LoadDistributedHDF_003
 * @tc.desc: Verify LoadDistributedHDF func
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(HdfOperateTest, LoadDistributedHDF_003, TestSize.Level1)
{
    DHLOGI("HdfOperateTest::LoadDistributedHDF_003");
    HdfOperateManager::GetInstance().hdfOperateMap_[DHType::AUDIO] = nullptr;
    int32_t ret = HdfOperateManager::GetInstance().LoadDistributedHDF(DHType::AUDIO);
    HdfOperateManager::GetInstance().hdfOperateMap_.clear();
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
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
 * @tc.name: UnLoadDistributedHDF_002
 * @tc.desc: Verify UnLoadDistributedHDF func
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(HdfOperateTest, UnLoadDistributedHDF_002, TestSize.Level1)
{
    DHLOGI("HdfOperateTest::UnLoadDistributedHDF_002");
    HdfOperateManager::GetInstance().hdfOperateMap_[DHType::AUDIO] = nullptr;
    int32_t ret = HdfOperateManager::GetInstance().UnLoadDistributedHDF(DHType::AUDIO);
    HdfOperateManager::GetInstance().hdfOperateMap_.clear();
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
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

/**
 * @tc.name: RigidGetSourcePtr_001
 * @tc.desc: Verify RigidGetSourcePtr func
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(HdfOperateTest, RigidGetSourcePtr_001, TestSize.Level1)
{
    DHLOGI("HdfOperateTest::RigidGetSourcePtr_001");
    IDistributedHardwareSource *sourcePtr = nullptr;
    EXPECT_EQ(DH_FWK_SUCCESS, HdfOperateManager::GetInstance().RigidGetSourcePtr(DHType::AUDIO, sourcePtr));
    EXPECT_EQ(DH_FWK_SUCCESS, HdfOperateManager::GetInstance().RigidGetSourcePtr(DHType::AUDIO, sourcePtr));
    EXPECT_EQ(DH_FWK_SUCCESS, HdfOperateManager::GetInstance().RigidReleaseSourcePtr(DHType::AUDIO));
    EXPECT_EQ(DH_FWK_SUCCESS, HdfOperateManager::GetInstance().RigidReleaseSourcePtr(DHType::AUDIO));
    EXPECT_EQ(DH_FWK_SUCCESS, HdfOperateManager::GetInstance().RigidGetSourcePtr(DHType::CAMERA, sourcePtr));
    EXPECT_EQ(DH_FWK_SUCCESS, HdfOperateManager::GetInstance().RigidGetSourcePtr(DHType::CAMERA, sourcePtr));
    EXPECT_EQ(DH_FWK_SUCCESS, HdfOperateManager::GetInstance().RigidReleaseSourcePtr(DHType::CAMERA));
    EXPECT_EQ(DH_FWK_SUCCESS, HdfOperateManager::GetInstance().RigidReleaseSourcePtr(DHType::CAMERA));
    EXPECT_EQ(ERR_DH_FWK_NO_HDF_SUPPORT,
        HdfOperateManager::GetInstance().RigidGetSourcePtr(DHType::UNKNOWN, sourcePtr));
}

/**
 * @tc.name: RigidReleaseSourcePtr_001
 * @tc.desc: Verify RigidReleaseSourcePtr func
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(HdfOperateTest, RigidReleaseSourcePtr_001, TestSize.Level1)
{
    DHLOGI("HdfOperateTest::RigidReleaseSourcePtr_001");
    EXPECT_EQ(DH_FWK_SUCCESS, HdfOperateManager::GetInstance().RigidReleaseSourcePtr(DHType::AUDIO));
    HdfOperateManager::GetInstance().sourceHandlerDataMap_[DHType::AUDIO] = HdfOperateManager::SourceHandlerData {
        .refCount = 1, .sourceHandler = nullptr, .sourcePtr = nullptr
    };
    auto ret = HdfOperateManager::GetInstance().RigidReleaseSourcePtr(DHType::AUDIO);
    HdfOperateManager::GetInstance().sourceHandlerDataMap_.erase(DHType::AUDIO);
    EXPECT_EQ(ERR_DH_FWK_LOADER_DLCLOSE_FAIL, ret);
}

HWTEST_F(HdfOperateTest, AddDeathRecipient_001, TestSize.Level1)
{
    sptr<IRemoteObject> remote = nullptr;
    auto ret = HdfOperateManager::GetInstance().AddDeathRecipient(DHType::UNKNOWN, remote);
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
}

HWTEST_F(HdfOperateTest, AddDeathRecipient_002, TestSize.Level1)
{
    sptr<IRemoteObject> remote(new TestPublisherListenerStub());
    auto ret = HdfOperateManager::GetInstance().AddDeathRecipient(DHType::UNKNOWN, remote);
    EXPECT_EQ(ERR_DH_FWK_NO_HDF_SUPPORT, ret);
}

HWTEST_F(HdfOperateTest, RemoveDeathRecipient_001, TestSize.Level1)
{
    sptr<IRemoteObject> remote = nullptr;
    auto ret = HdfOperateManager::GetInstance().RemoveDeathRecipient(DHType::UNKNOWN, remote);
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
}

HWTEST_F(HdfOperateTest, RemoveDeathRecipient_002, TestSize.Level1)
{
    sptr<IRemoteObject> remote(new TestPublisherListenerStub());
    auto ret = HdfOperateManager::GetInstance().RemoveDeathRecipient(DHType::UNKNOWN, remote);
    EXPECT_EQ(ERR_DH_FWK_NO_HDF_SUPPORT, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
