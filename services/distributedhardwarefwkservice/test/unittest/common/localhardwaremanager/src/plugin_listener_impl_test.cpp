/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "plugin_listener_impl_test.h"

#include "distributed_hardware_errno.h"
#include "mock_hardware_handler.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {
const std::string dhId = "00000000000000000";
const std::string attrs = "11111111111111111";
std::shared_ptr<PluginListener> g_listener;
std::shared_ptr<MockHardwareHandler> g_mockHardwareHandler;
}

void PluginListenerImplTest::SetUpTestCase(void)
{
    DHType dhType = DHType::CAMERA;
    g_listener = std::make_shared<PluginListenerImpl>(dhType);
    g_mockHardwareHandler = std::make_shared<MockHardwareHandler>();
}

void PluginListenerImplTest::TearDownTestCase(void) {}

void PluginListenerImplTest::SetUp() {}

void PluginListenerImplTest::TearDown() {}

/**
 * @tc.name: plugin_listener_impl_test_001
 * @tc.desc: Verify the PluginHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(PluginListenerImplTest, plugin_listener_impl_test_001, TestSize.Level0)
{
    g_mockHardwareHandler->RegisterPluginListener(g_listener);
    EXPECT_EQ(g_mockHardwareHandler->PluginHardware(dhId, attrs), DH_FWK_SUCCESS);
}

/**
 * @tc.name: plugin_listener_impl_test_002
 * @tc.desc: Verify the UnPluginHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(PluginListenerImplTest, plugin_listener_impl_test_002, TestSize.Level0)
{
    EXPECT_EQ(g_mockHardwareHandler->UnPluginHardware(dhId), DH_FWK_SUCCESS);
    g_mockHardwareHandler->UnRegisterPluginListener();
}
} // namespace DistributedHardware
} // namespace OHOS
