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

#include "daudio_hdf_operate.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "mock_hdfoperate_device_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DAudioHdfOperateTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

private:
    sptr<MockDeviceManager> deviceManager_;
};

void DAudioHdfOperateTest::SetUpTestCase(void)
{
    DHLOGI("DAudioHdfOperateTest::SetUpTestCase");
}

void DAudioHdfOperateTest::TearDownTestCase(void)
{
    DHLOGI("DAudioHdfOperateTest::TearDownTestCase");
}

void DAudioHdfOperateTest::SetUp(void)
{
    DHLOGI("DAudioHdfOperateTest::SetUp");
    deviceManager_ = MockDeviceManager::GetOrCtreateInstance();
}

void DAudioHdfOperateTest::TearDown(void)
{
    DHLOGI("DAudioHdfOperateTest::TearDown");
    deviceManager_ = nullptr;
}

/**
 * @tc.name: UnLoadDaudioHDFImpl_001
 * @tc.desc: Verify UnLoadDaudioHDFImpl func
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DAudioHdfOperateTest, UnLoadDaudioHDFImpl_001, TestSize.Level1)
{
    DHLOGI("DAudioHdfOperateTest::UnLoadDaudioHDFImpl_001");
    int32_t ret = DaudioHdfOperate::GetInstance().UnLoadDaudioHDFImpl();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: ResetRefCount_001
 * @tc.desc: Verify ResetRefCount func
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DAudioHdfOperateTest, ResetRefCount_001, TestSize.Level1)
{
    DHLOGI("DAudioHdfOperateTest::ResetRefCount_001");
    DaudioHdfOperate::GetInstance().hdfLoadRef_ = 1;
    DaudioHdfOperate::GetInstance().ResetRefCount();
    EXPECT_EQ(DaudioHdfOperate::GetInstance().hdfLoadRef_, 0);
}

/**
 * @tc.name: WaitLoadService_001
 * @tc.desc: Verify WaitLoadService func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DAudioHdfOperateTest, WaitLoadService_001, TestSize.Level1)
{
    DHLOGI("DAudioHdfOperateTest::WaitLoadService_001");
    DaudioHdfOperate::GetInstance().audioServStatus_.store(OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_STOP);
    int32_t ret = DaudioHdfOperate::GetInstance().WaitLoadService(AUDIO_SERVICE_NAME);
    EXPECT_EQ(ERR_DH_FWK_LOAD_HDF_TIMEOUT, ret);
}

/**
 * @tc.name: WaitLoadService_002
 * @tc.desc: Verify WaitLoadService func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DAudioHdfOperateTest, WaitLoadService_002, TestSize.Level1)
{
    DHLOGI("DAudioHdfOperateTest::WaitLoadService_002");
    DaudioHdfOperate::GetInstance().audioextServStatus_.store(OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_STOP);
    int32_t ret = DaudioHdfOperate::GetInstance().WaitLoadService(AUDIOEXT_SERVICE_NAME);
    EXPECT_EQ(ERR_DH_FWK_LOAD_HDF_TIMEOUT, ret);
}

/**
 * @tc.name: LoadDevice_001
 * @tc.desc: Verify LoadDevice func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DAudioHdfOperateTest, LoadDevice_001, TestSize.Level1)
{
    DHLOGI("DAudioHdfOperateTest::LoadDevice_001");
    EXPECT_CALL(*deviceManager_, LoadDevice(_)).WillRepeatedly(testing::Return(HDF_ERR_DEVICE_BUSY));
    int32_t ret = DaudioHdfOperate::GetInstance().LoadDevice();
    EXPECT_EQ(ERR_DH_FWK_LOAD_HDF_TIMEOUT, ret);
    EXPECT_CALL(*deviceManager_, LoadDevice(_)).WillRepeatedly(testing::Return(ERR_DH_FWK_LOAD_HDF_FAIL));
    ret = DaudioHdfOperate::GetInstance().LoadDevice();
    EXPECT_EQ(ERR_DH_FWK_LOAD_HDF_FAIL, ret);
}

/**
 * @tc.name: LoadDevice_002
 * @tc.desc: Verify LoadDevice func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DAudioHdfOperateTest, LoadDevice_002, TestSize.Level1)
{
    DHLOGI("DAudioHdfOperateTest::LoadDevice_002");
    bool isFirstTime = true;
    EXPECT_CALL(*deviceManager_, LoadDevice(_)).WillRepeatedly([&]()->int32_t {
        if (isFirstTime) {
            isFirstTime = false;
            return HDF_SUCCESS;
        } else {
            return HDF_ERR_DEVICE_BUSY;
        }
    });
    DaudioHdfOperate::GetInstance().audioServStatus_ = OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START;
    int32_t ret = DaudioHdfOperate::GetInstance().LoadDevice();
    EXPECT_EQ(ERR_DH_FWK_LOAD_HDF_TIMEOUT, ret);
    isFirstTime = true;
    EXPECT_CALL(*deviceManager_, LoadDevice(_)).WillRepeatedly([&]()->int32_t {
        if (isFirstTime) {
            isFirstTime = false;
            return HDF_SUCCESS;
        } else {
            return ERR_DH_FWK_LOAD_HDF_FAIL;
        }
    });
    ret = DaudioHdfOperate::GetInstance().LoadDevice();
    EXPECT_EQ(ERR_DH_FWK_LOAD_HDF_FAIL, ret);
}

/**
 * @tc.name: UnLoadDevice_001
 * @tc.desc: Verify UnLoadDevice func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DAudioHdfOperateTest, UnloadDevice_001, TestSize.Level1)
{
    DHLOGI("DAudioHdfOperateTest::UnloadDevice_001");
    EXPECT_CALL(*deviceManager_, UnloadDevice(_)).WillRepeatedly(testing::Return(HDF_ERR_DEVICE_BUSY));
    int32_t ret = DaudioHdfOperate::GetInstance().UnLoadDevice();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: UnLoadDevice_002
 * @tc.desc: Verify UnLoadDevice func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DAudioHdfOperateTest, UnloadDevice_002, TestSize.Level1)
{
    DHLOGI("DAudioHdfOperateTest::UnloadDevice_002");
    auto devmgr = DaudioHdfOperate::GetInstance().devmgr_;
    DaudioHdfOperate::GetInstance().devmgr_ = nullptr;
    int32_t ret = DaudioHdfOperate::GetInstance().UnLoadDevice();
    DaudioHdfOperate::GetInstance().devmgr_ = devmgr;
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
}

/**
 * @tc.name: UnRegisterHdfListener_001
 * @tc.desc: Verify UnRegisterHdfListener func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DAudioHdfOperateTest, UnRegisterHdfListener_001, TestSize.Level1)
{
    DHLOGI("DAudioHdfOperateTest::UnRegisterHdfListener_001");
    auto audioSrvHdf = DaudioHdfOperate::GetInstance().audioSrvHdf_;
    DaudioHdfOperate::GetInstance().audioSrvHdf_ = nullptr;
    int32_t ret = DaudioHdfOperate::GetInstance().UnRegisterHdfListener();
    DaudioHdfOperate::GetInstance().audioSrvHdf_ = audioSrvHdf;
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
