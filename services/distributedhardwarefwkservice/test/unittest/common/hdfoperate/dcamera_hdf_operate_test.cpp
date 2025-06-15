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

#include "dcamera_hdf_operate.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "mock_hdfoperate_device_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraHdfOperateTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
    sptr<MockDeviceManager> deviceManager_;
};

void DCameraHdfOperateTest::SetUpTestCase(void)
{
    DHLOGI("DCameraHdfOperateTest::SetUpTestCase");
}

void DCameraHdfOperateTest::TearDownTestCase(void)
{
    DHLOGI("DCameraHdfOperateTest::TearDownTestCase");
}

void DCameraHdfOperateTest::SetUp(void)
{
    DHLOGI("DCameraHdfOperateTest::SetUp");
    deviceManager_ = MockDeviceManager::GetOrCtreateInstance();
}

void DCameraHdfOperateTest::TearDown(void)
{
    DHLOGI("DCameraHdfOperateTest::TearDown");
    deviceManager_ = nullptr;
}

/**
 * @tc.name: UnLoadDcameraHDFImpl_001
 * @tc.desc: Verify UnLoadDcameraHDFImpl func
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DCameraHdfOperateTest, UnLoadDcameraHDFImpl_001, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::UnLoadDcameraHDFImpl_001");
    int32_t ret = DCameraHdfOperate::GetInstance().UnLoadDcameraHDFImpl();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: ResetRefCount_001
 * @tc.desc: Verify ResetRefCount func
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DCameraHdfOperateTest, ResetRefCount_001, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::ResetRefCount_001");
    DCameraHdfOperate::GetInstance().hdfLoadRef_ = 1;
    DCameraHdfOperate::GetInstance().ResetRefCount();
    EXPECT_EQ(DCameraHdfOperate::GetInstance().hdfLoadRef_, 0);
}

/**
 * @tc.name: WaitLoadCameraService_001
 * @tc.desc: Verify WaitLoadCameraService func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DCameraHdfOperateTest, WaitLoadCameraService_001, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::WaitLoadCameraService_001");
    DCameraHdfOperate::GetInstance().cameraServStatus_.store(OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_STOP);
    int32_t ret = DCameraHdfOperate::GetInstance().WaitLoadCameraService();
    EXPECT_EQ(ERR_DH_FWK_LOAD_HDF_TIMEOUT, ret);
}

/**
 * @tc.name: WaitLoadProviderService_001
 * @tc.desc: Verify WaitLoadProviderService func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DCameraHdfOperateTest, WaitLoadProviderService_001, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::WaitLoadProviderService_001");
    DCameraHdfOperate::GetInstance().providerServStatus_.store(OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_STOP);
    int32_t ret = DCameraHdfOperate::GetInstance().WaitLoadProviderService();
    EXPECT_EQ(ERR_DH_FWK_LOAD_HDF_TIMEOUT, ret);
}

/**
 * @tc.name: LoadDevice_001
 * @tc.desc: Verify LoadDevice func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DCameraHdfOperateTest, LoadDevice_001, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::LoadDevice_001");
    EXPECT_CALL(*deviceManager_, LoadDevice(_)).WillRepeatedly(testing::Return(HDF_ERR_DEVICE_BUSY));
    int32_t ret = DCameraHdfOperate::GetInstance().LoadDevice();
    EXPECT_EQ(ERR_DH_FWK_LOAD_HDF_TIMEOUT, ret);
    EXPECT_CALL(*deviceManager_, LoadDevice(_)).WillRepeatedly(testing::Return(ERR_DH_FWK_LOAD_HDF_FAIL));
    ret = DCameraHdfOperate::GetInstance().LoadDevice();
    EXPECT_EQ(ERR_DH_FWK_LOAD_HDF_FAIL, ret);
}

/**
 * @tc.name: LoadDevice_002
 * @tc.desc: Verify LoadDevice func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DCameraHdfOperateTest, LoadDevice_002, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::LoadDevice_002");
    bool isFirstTime = true;
    EXPECT_CALL(*deviceManager_, LoadDevice(_)).WillRepeatedly([&]()->int32_t {
        if (isFirstTime) {
            isFirstTime = false;
            return HDF_SUCCESS;
        } else {
            return HDF_ERR_DEVICE_BUSY;
        }
    });
    DCameraHdfOperate::GetInstance().cameraServStatus_ = OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START;
    int32_t ret = DCameraHdfOperate::GetInstance().LoadDevice();
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
    ret = DCameraHdfOperate::GetInstance().LoadDevice();
    EXPECT_EQ(ERR_DH_FWK_LOAD_HDF_FAIL, ret);
}

/**
 * @tc.name: UnLoadDevice_001
 * @tc.desc: Verify UnLoadDevice func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DCameraHdfOperateTest, UnloadDevice_001, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::UnloadDevice_001");
    EXPECT_CALL(*deviceManager_, UnloadDevice(_)).WillRepeatedly(testing::Return(HDF_ERR_DEVICE_BUSY));
    int32_t ret = DCameraHdfOperate::GetInstance().UnLoadDevice();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
