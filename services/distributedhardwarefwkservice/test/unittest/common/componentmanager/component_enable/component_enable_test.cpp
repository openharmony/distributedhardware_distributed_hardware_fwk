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

#include "component_enable.h"

#include <gtest/gtest.h>
#include <string>

#include "anonymous_string.h"
#include "constants.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "idistributed_hardware_source.h"

using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
using namespace std;
constexpr int32_t ENABLE_RESULT_FAILED = -1;

class FakeDistributedHardwareSource : public IDistributedHardwareSource {
public:
    int32_t InitSource(const std::string &params) override
    {
        (void)params;
        return DH_FWK_SUCCESS;
    }

    int32_t ReleaseSource() override
    {
        return DH_FWK_SUCCESS;
    }

    int32_t RegisterDistributedHardware(const std::string &networkId, const std::string &dhId,
        const EnableParam &param, std::shared_ptr<RegisterCallback> callback) override
    {
        (void)param;
        if (triggerCallback_ && callback != nullptr) {
            callback->OnRegisterResult(networkId, dhId, callbackStatus_, "");
        }
        return registerRet_;
    }

    int32_t UnregisterDistributedHardware(const std::string &networkId, const std::string &dhId,
        std::shared_ptr<UnregisterCallback> callback) override
    {
        (void)networkId;
        (void)dhId;
        (void)callback;
        return DH_FWK_SUCCESS;
    }

    int32_t ConfigDistributedHardware(const std::string &networkId, const std::string &dhId, const std::string &key,
        const std::string &value) override
    {
        (void)networkId;
        (void)dhId;
        configCalled_ = true;
        configKey_ = key;
        configValue_ = value;
        return configRet_;
    }

    void RegisterDistributedHardwareStateListener(std::shared_ptr<DistributedHardwareStateListener> listener) override
    {
        (void)listener;
    }

    void UnregisterDistributedHardwareStateListener() override {}

    void RegisterDataSyncTriggerListener(std::shared_ptr<DataSyncTriggerListener> listener) override
    {
        (void)listener;
    }

    void UnregisterDataSyncTriggerListener() override {}

    void SetRegisterRet(int32_t ret)
    {
        registerRet_ = ret;
    }

    void SetConfigRet(int32_t ret)
    {
        configRet_ = ret;
    }

    void SetTriggerCallback(bool trigger)
    {
        triggerCallback_ = trigger;
    }

    bool IsConfigCalled() const
    {
        return configCalled_;
    }

    const std::string &GetConfigKey() const
    {
        return configKey_;
    }

    const std::string &GetConfigValue() const
    {
        return configValue_;
    }

private:
    int32_t registerRet_ = DH_FWK_SUCCESS;
    int32_t configRet_ = DH_FWK_SUCCESS;
    int32_t callbackStatus_ = DH_FWK_SUCCESS;
    bool triggerCallback_ = true;
    bool configCalled_ = false;
    std::string configKey_;
    std::string configValue_;
};

class ComponentEnableTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ComponentEnableTest::SetUpTestCase()
{
}

void ComponentEnableTest::TearDownTestCase()
{
}

void ComponentEnableTest::SetUp()
{
}

void ComponentEnableTest::TearDown()
{
}

HWTEST_F(ComponentEnableTest, Enable_001, testing::ext::TestSize.Level1)
{
    auto compEnable = std::make_shared<ComponentEnable>();
    std::string networkId = "";
    std::string dhId = "";
    EnableParam param;
    IDistributedHardwareSource *sourcePtr = nullptr;
    auto ret = compEnable->Enable(networkId, dhId, param, sourcePtr);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);

    networkId = "networkId_test";
    ret = compEnable->Enable(networkId, dhId, param, sourcePtr);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);

    networkId = "";
    dhId = "dhId_test";
    ret = compEnable->Enable(networkId, dhId, param, sourcePtr);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
}

HWTEST_F(ComponentEnableTest, Enable_002, testing::ext::TestSize.Level1)
{
    auto compEnable = std::make_shared<ComponentEnable>();
    std::string networkId = "networkId_test";
    std::string dhId = "dhId_test";
    EnableParam param;
    IDistributedHardwareSource *sourcePtr = nullptr;
    auto ret = compEnable->Enable(networkId, dhId, param, sourcePtr);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
}

HWTEST_F(ComponentEnableTest, OnRegisterResult_001, testing::ext::TestSize.Level1)
{
    auto compEnable = std::make_shared<ComponentEnable>();
    std::string networkId = "";
    std::string dhId = "";
    int32_t status = 0;
    std::string data;
    auto ret = compEnable->OnRegisterResult(networkId, dhId, status, data);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);

    networkId = "networkId_test";
    ret = compEnable->OnRegisterResult(networkId, dhId, status, data);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);

    networkId = "";
    dhId = "dhId_test";
    ret = compEnable->OnRegisterResult(networkId, dhId, status, data);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
}

HWTEST_F(ComponentEnableTest, OnRegisterResult_002, testing::ext::TestSize.Level1)
{
    auto compEnable = std::make_shared<ComponentEnable>();
    std::string networkId = "networkId_test";
    std::string dhId = "dhId_test";
    int32_t status = 0;
    std::string data;
    auto ret = compEnable->OnRegisterResult(networkId, dhId, status, data);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);

    status = ENABLE_RESULT_FAILED;
    ret = compEnable->OnRegisterResult(networkId, dhId, status, data);
    EXPECT_EQ(ret, ENABLE_RESULT_FAILED);
}

HWTEST_F(ComponentEnableTest, Enable_003, testing::ext::TestSize.Level1)
{
    auto compEnable = std::make_shared<ComponentEnable>();
    FakeDistributedHardwareSource source;
    source.SetConfigRet(ERR_DH_FWK_PARA_INVALID);
    EnableParam param;

    auto ret = compEnable->Enable("networkId_test", "dhId_test", param, &source, "enable_init_params");
    EXPECT_EQ(ret, ERR_DH_FWK_COMPONENT_CONFIG_FAILED);
}

HWTEST_F(ComponentEnableTest, Enable_004, testing::ext::TestSize.Level1)
{
    auto compEnable = std::make_shared<ComponentEnable>();
    FakeDistributedHardwareSource source;
    source.SetConfigRet(DH_FWK_SUCCESS);
    EnableParam param;

    auto ret = compEnable->Enable("networkId_test", "dhId_test", param, &source, "enable_init_params");
    EXPECT_NE(ret, ERR_DH_FWK_COMPONENT_CONFIG_FAILED);
}

HWTEST_F(ComponentEnableTest, Enable_005, testing::ext::TestSize.Level1)
{
    auto compEnable = std::make_shared<ComponentEnable>();
    FakeDistributedHardwareSource source;
    source.SetConfigRet(ERR_DH_FWK_PARA_INVALID);
    EnableParam param;

    auto ret = compEnable->Enable("networkId_test", "dhId_test", param, &source, "");
    EXPECT_NE(ret, ERR_DH_FWK_COMPONENT_CONFIG_FAILED);
}
}
}