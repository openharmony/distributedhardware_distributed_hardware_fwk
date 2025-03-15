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

#include "plugin_listener_impl_test.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {
const std::string DHID_TEST = "audio_1234564";
const std::string ATTRS_TEST = "attrs_0";
const std::string SUBTYPE_TEST = "subtype";
}

void PluginListenerImplTest::SetUpTestCase(void) {}

void PluginListenerImplTest::TearDownTestCase(void) {}

void PluginListenerImplTest::SetUp() {}

void PluginListenerImplTest::TearDown() {}

/**
 * @tc.name: PluginHardware_001
 * @tc.desc: Verify the PluginHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(PluginListenerImplTest, PluginHardware_001, TestSize.Level1)
{
    DHType type = DHType::AUDIO;
    std::shared_ptr<PluginListener> listener = std::make_shared<PluginListenerImpl>(type);
    std::string dhId = "";
    std::string attrs = "";
    ASSERT_NO_FATAL_FAILURE(listener->PluginHardware(dhId, attrs, SUBTYPE_TEST));
}

/**
 * @tc.name: PluginHardware_002
 * @tc.desc: Verify the PluginHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(PluginListenerImplTest, PluginHardware_002, TestSize.Level1)
{
    DHType type = DHType::AUDIO;
    std::shared_ptr<PluginListener> listener = std::make_shared<PluginListenerImpl>(type);
    std::string dhId = "";
    ASSERT_NO_FATAL_FAILURE(listener->PluginHardware(dhId, ATTRS_TEST, SUBTYPE_TEST));
}

/**
 * @tc.name: PluginHardware_003
 * @tc.desc: Verify the PluginHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(PluginListenerImplTest, PluginHardware_003, TestSize.Level1)
{
    DHType type = DHType::AUDIO;
    std::shared_ptr<PluginListener> listener = std::make_shared<PluginListenerImpl>(type);
    std::string attrs = "";
    ASSERT_NO_FATAL_FAILURE(listener->PluginHardware(DHID_TEST, attrs, SUBTYPE_TEST));

    ASSERT_NO_FATAL_FAILURE(listener->PluginHardware(DHID_TEST, ATTRS_TEST, SUBTYPE_TEST));
}

/**
 * @tc.name: UnPluginHardware_001
 * @tc.desc: Verify the UnPluginHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(PluginListenerImplTest, UnPluginHardware_001, TestSize.Level1)
{
    std::string dhId = "";
    DHType type = DHType::AUDIO;
    std::shared_ptr<PluginListenerImpl> listener = std::make_shared<PluginListenerImpl>(type);
    ASSERT_NO_FATAL_FAILURE(listener->UnPluginHardware(dhId));
}

/**
 * @tc.name: UnPluginHardware_002
 * @tc.desc: Verify the UnPluginHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK3
 */
HWTEST_F(PluginListenerImplTest, UnPluginHardware_002, TestSize.Level1)
{
    DHType type = DHType::AUDIO;
    std::shared_ptr<PluginListenerImpl> listener = std::make_shared<PluginListenerImpl>(type);
    ASSERT_NO_FATAL_FAILURE(listener->UnPluginHardware(DHID_TEST));
}
} // namespace DistributedHardware
} // namespace OHOS
