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
}
}