/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "dscreen_input_test.h"

#include "dscreen_input_plugin.h"
#include "plugin_types.h"

namespace OHOS {
namespace DistributedHardware {
const std::string PLUGINNAME = "dscreen_input";

void DscreenInputTest::SetUpTestCase(void) {}

void DscreenInputTest::TearDownTestCase(void) {}

void DscreenInputTest::SetUp(void) {}

void DscreenInputTest::TearDown(void) {}

HWTEST_F(DscreenInputTest, SetParameter_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DscreenInputPlugin>(PLUGINNAME);
    std::string value = "dscreen_input_test";
    Status ret = plugin->SetParameter(Tag::USER_SHARED_MEMORY_FD, value);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DscreenInputTest, GetParameter_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DscreenInputPlugin>(PLUGINNAME);
    std::string value = "dscreen_input_test";
    plugin->SetParameter(Tag::USER_SHARED_MEMORY_FD, value);

    ValueType val;
    Status ret = plugin->GetParameter(Tag::USER_SHARED_MEMORY_FD, val);
    EXPECT_EQ(Status::OK, ret);

    plugin->paramsMap_.clear();
    ret = plugin->GetParameter(Tag::USER_SHARED_MEMORY_FD, val);
    EXPECT_EQ(Status::ERROR_NOT_EXISTED, ret);
}

HWTEST_F(DscreenInputTest, PushData_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DscreenInputPlugin>(PLUGINNAME);
    std::shared_ptr<AVBuffer> buffer = nullptr;
    Status ret = plugin->PushData("", buffer, 0);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    buffer = std::make_shared<AVBuffer>();
    buffer->AllocMemory(nullptr, 10);
    buffer->ChangeBufferMetaType(BufferMetaType::VIDEO);
    buffer->GetMemory()->Write((uint8_t*)"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 10);
    ret = plugin->PushData("", buffer, 0);
    EXPECT_EQ(Status::OK, ret);
}
} // namespace DistributedHardware
} // namespace OHOS