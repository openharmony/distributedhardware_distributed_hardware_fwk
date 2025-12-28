/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "device_param_mgr_test.h"

#include <algorithm>
#include <securec.h>

#include "device_param_mgr.h"

using namespace testing::ext;

namespace OHOS {
bool g_parameterVaule = true;
namespace system {
bool GetBoolParameter(const std::string& key, bool def)
{
    return g_parameterVaule;
}
}
}

namespace OHOS {
namespace DistributedHardware {
static int32_t g_getParameter = 0;
constexpr const char *SYNCTYPE = "1";
static std::string g_paramOutContent = "0";

namespace {
    constexpr int32_t ERR_ACCOUNT_COMMON = -1;
}

extern "C" int GetParameter(const char *key, const char *def, char *value, uint32_t len)
{
    (void)key;
    (void)def;
    if (!value || len <= 0) {
        return 0;
    }
    int n = std::min(static_cast<int>(g_paramOutContent.size()), static_cast<int>(len));
    if (n > 0) {
        if (memcpy_s(value, len + 1, g_paramOutContent.data(), n) != EOK) {
            return -1;
        }
    }
    if (n < len) {
        value[n] = '\0';
    }
    return g_getParameter;
}

void DeviceParamMgrTest::SetUpTestCase(void)
{
}

void DeviceParamMgrTest::TearDownTestCase(void)
{
}

void DeviceParamMgrTest::SetUp()
{
    deviceDataSyncParamMgr_ = std::make_shared<AccountSA::MockIOsAccountManager>();
    AccountSA::MockIOsAccountManager::ParamDataSyncMgrInstance_ = deviceDataSyncParamMgr_;
}

void DeviceParamMgrTest::TearDown()
{
    AccountSA::MockIOsAccountManager::ParamDataSyncMgrInstance_ = nullptr;
    deviceDataSyncParamMgr_ = nullptr;
}

/**
 * @tc.name: QueryDeviceDataSyncMode_001
 * @tc.desc: Query device datasync mode function
 * @tc.type: FUNC
 */
HWTEST_F(DeviceParamMgrTest, QueryDeviceDataSyncMode_001, TestSize.Level1)
{
    g_getParameter = 1;
    g_paramOutContent = SYNCTYPE;

    g_parameterVaule = false;

    EXPECT_CALL(*deviceDataSyncParamMgr_, GetForegroundOsAccountLocalId(testing::_))
        .WillRepeatedly(testing::DoAll(testing::SetArgReferee<0>(-1), testing::Return(ERR_ACCOUNT_COMMON)));

    ASSERT_NO_FATAL_FAILURE(DeviceParamMgr::GetInstance().QueryDeviceDataSyncMode());
}

/**
 * @tc.name: QueryDeviceDataSyncMode_002
 * @tc.desc: Query device datasync mode function
 * @tc.type: FUNC
 */
HWTEST_F(DeviceParamMgrTest, QueryDeviceDataSyncMode_002, TestSize.Level1)
{
    g_getParameter = 1;
    g_paramOutContent = SYNCTYPE;

    g_parameterVaule = true;

    EXPECT_CALL(*deviceDataSyncParamMgr_, GetForegroundOsAccountLocalId(testing::_))
        .WillRepeatedly(testing::DoAll(testing::SetArgReferee<0>(100), testing::Return(ERR_OK)));

    AccountSA::DomainAccountInfo domainInfo;
    domainInfo.accountName_ = "corp_user";
    EXPECT_CALL(*deviceDataSyncParamMgr_, GetOsAccountDomainInfo(testing::_, testing::_))
        .WillRepeatedly(testing::DoAll(testing::SetArgReferee<1>(domainInfo), testing::Return(ERR_OK)));

    ASSERT_NO_FATAL_FAILURE(DeviceParamMgr::GetInstance().QueryDeviceDataSyncMode());
}

/**
 * @tc.name: QueryDeviceDataSyncMode_003
 * @tc.desc: Query device datasync mode function
 * @tc.type: FUNC
 */
HWTEST_F(DeviceParamMgrTest, QueryDeviceDataSyncMode_003, TestSize.Level1)
{
    g_getParameter = 1;
    g_paramOutContent = SYNCTYPE;

    g_parameterVaule = false;

    EXPECT_CALL(*deviceDataSyncParamMgr_, GetForegroundOsAccountLocalId(testing::_))
        .WillRepeatedly(testing::DoAll(testing::SetArgReferee<0>(100), testing::Return(ERR_OK)));

    AccountSA::DomainAccountInfo domainInfo;
    domainInfo.accountName_ = "corp_user";
    EXPECT_CALL(*deviceDataSyncParamMgr_, GetOsAccountDomainInfo(testing::_, testing::_))
        .WillRepeatedly(testing::DoAll(testing::SetArgReferee<1>(domainInfo), testing::Return(ERR_OK)));

    ASSERT_NO_FATAL_FAILURE(DeviceParamMgr::GetInstance().QueryDeviceDataSyncMode());
}

/**
 * @tc.name: QueryDeviceDataSyncMode_004
 * @tc.desc: Query device datasync mode function
 * @tc.type: FUNC
 */
HWTEST_F(DeviceParamMgrTest, QueryDeviceDataSyncMode_004, TestSize.Level1)
{
    g_getParameter = 1;
    g_paramOutContent = SYNCTYPE;

    g_parameterVaule = true;

    EXPECT_CALL(*deviceDataSyncParamMgr_, GetForegroundOsAccountLocalId(testing::_))
        .WillRepeatedly(testing::DoAll(testing::SetArgReferee<0>(-1), testing::Return(ERR_ACCOUNT_COMMON)));

    ASSERT_NO_FATAL_FAILURE(DeviceParamMgr::GetInstance().QueryDeviceDataSyncMode());
}

/**
 * @tc.name: QueryUserBelongToSpace_001
 * @tc.desc: Query the space to which the user belongs function
 * @tc.type: FUNC
 */
HWTEST_F(DeviceParamMgrTest, QueryUserBelongToSpace_001, TestSize.Level1)
{
    EXPECT_CALL(*deviceDataSyncParamMgr_, GetForegroundOsAccountLocalId(testing::_))
        .WillRepeatedly(testing::DoAll(testing::SetArgReferee<0>(-1), testing::Return(ERR_ACCOUNT_COMMON)));

    EXPECT_EQ(DeviceParamMgr::GetInstance().QueryUserBelongToSpace(), false);
}

/**
 * @tc.name: QueryUserBelongToSpace_002
 * @tc.desc: Query the space to which the user belongs function
 * @tc.type: FUNC
 */
HWTEST_F(DeviceParamMgrTest, QueryUserBelongToSpace_002, TestSize.Level1)
{
    EXPECT_CALL(*deviceDataSyncParamMgr_, GetForegroundOsAccountLocalId(testing::_))
        .WillRepeatedly(testing::DoAll(testing::SetArgReferee<0>(100), testing::Return(ERR_OK)));

    AccountSA::DomainAccountInfo domainInfo;
    domainInfo.accountName_.clear();
    EXPECT_CALL(*deviceDataSyncParamMgr_, GetOsAccountDomainInfo(testing::_, testing::_))
        .WillRepeatedly(testing::DoAll(testing::SetArgReferee<1>(domainInfo), testing::Return(ERR_OK)));

    EXPECT_EQ(DeviceParamMgr::GetInstance().QueryUserBelongToSpace(), false);
}

/**
 * @tc.name: QueryUserBelongToSpace_003
 * @tc.desc: Query the space to which the user belongs function
 * @tc.type: FUNC
 */
HWTEST_F(DeviceParamMgrTest, QueryUserBelongToSpace_003, TestSize.Level1)
{
    EXPECT_CALL(*deviceDataSyncParamMgr_, GetForegroundOsAccountLocalId(testing::_))
        .WillRepeatedly(testing::DoAll(testing::SetArgReferee<0>(100), testing::Return(ERR_OK)));

    AccountSA::DomainAccountInfo domainInfo;
    domainInfo.accountName_ = "corp_user";
    EXPECT_CALL(*deviceDataSyncParamMgr_, GetOsAccountDomainInfo(testing::_, testing::_))
        .WillRepeatedly(testing::DoAll(testing::SetArgReferee<1>(domainInfo), testing::Return(ERR_ACCOUNT_COMMON)));

    EXPECT_EQ(DeviceParamMgr::GetInstance().QueryUserBelongToSpace(), false);
}

/**
 * @tc.name: QueryUserBelongToSpace_004
 * @tc.desc: Query the space to which the user belongs function
 * @tc.type: FUNC
 */
HWTEST_F(DeviceParamMgrTest, QueryUserBelongToSpace_004, TestSize.Level1)
{
    EXPECT_CALL(*deviceDataSyncParamMgr_, GetForegroundOsAccountLocalId(testing::_))
        .WillRepeatedly(testing::DoAll(testing::SetArgReferee<0>(100), testing::Return(ERR_OK)));

    AccountSA::DomainAccountInfo domainInfo;
    domainInfo.accountName_ = "corp_user";
    EXPECT_CALL(*deviceDataSyncParamMgr_, GetOsAccountDomainInfo(testing::_, testing::_))
        .WillRepeatedly(testing::DoAll(testing::SetArgReferee<1>(domainInfo), testing::Return(ERR_OK)));

    EXPECT_EQ(DeviceParamMgr::GetInstance().QueryUserBelongToSpace(), true);
}

HWTEST_F(DeviceParamMgrTest, GetDeviceSyncDataMode_001, TestSize.Level1)
{
    DeviceParamMgr::GetInstance().isDeviceE2ESync_.store(false);
    auto ret = DeviceParamMgr::GetInstance().GetDeviceSyncDataMode();
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceParamMgrTest, GetDeviceSyncDataMode_002, TestSize.Level1)
{
    DeviceParamMgr::GetInstance().isDeviceE2ESync_.store(true);
    g_parameterVaule = true;
    EXPECT_CALL(*deviceDataSyncParamMgr_, GetForegroundOsAccountLocalId(testing::_))
        .WillRepeatedly(testing::DoAll(testing::SetArgReferee<0>(100), testing::Return(ERR_OK)));

    AccountSA::DomainAccountInfo domainInfo;
    domainInfo.accountName_ = "corp_user";
    EXPECT_CALL(*deviceDataSyncParamMgr_, GetOsAccountDomainInfo(testing::_, testing::_))
        .WillRepeatedly(testing::DoAll(testing::SetArgReferee<1>(domainInfo), testing::Return(ERR_OK)));
    auto ret = DeviceParamMgr::GetInstance().GetDeviceSyncDataMode();
    EXPECT_EQ(ret, true);
}

HWTEST_F(DeviceParamMgrTest, GetDeviceSyncDataMode_003, TestSize.Level1)
{
    DeviceParamMgr::GetInstance().isDeviceE2ESync_.store(true);
    g_parameterVaule = true;
    EXPECT_CALL(*deviceDataSyncParamMgr_, GetForegroundOsAccountLocalId(testing::_))
        .WillRepeatedly(testing::DoAll(testing::SetArgReferee<0>(-1), testing::Return(ERR_ACCOUNT_COMMON)));

    AccountSA::DomainAccountInfo domainInfo;
    domainInfo.accountName_ = "corp_user";
    EXPECT_CALL(*deviceDataSyncParamMgr_, GetOsAccountDomainInfo(testing::_, testing::_))
        .WillRepeatedly(testing::DoAll(testing::SetArgReferee<1>(domainInfo), testing::Return(ERR_OK)));

    auto ret = DeviceParamMgr::GetInstance().GetDeviceSyncDataMode();
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceParamMgrTest, GetDeviceSyncDataMode_004, TestSize.Level1)
{
    DeviceParamMgr::GetInstance().isDeviceE2ESync_.store(true);
    g_parameterVaule = false;;
    auto ret = DeviceParamMgr::GetInstance().GetDeviceSyncDataMode();
    EXPECT_EQ(ret, true);
}
} // namespace DistributedHardware
} // namespace OHOS
