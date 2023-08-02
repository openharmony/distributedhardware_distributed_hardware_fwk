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

#include "dscreen_output_test.h"

#include "dscreen_output_plugin.h"

namespace OHOS {
namespace DistributedHardware {
const std::string PLUGINNAME = "dscreen_output";

void DscreenOutputTest::SetUpTestCase(void) {}

void DscreenOutputTest::TearDownTestCase(void) {}

void DscreenOutputTest::SetUp(void) {}

void DscreenOutputTest::TearDown(void) {}


HWTEST_F(DscreenOutputTest, Prepare_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->state_ = State::PREPARED;
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DscreenOutputTest, Prepare_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->state_ = State::INITIALIZED;
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(DscreenOutputTest, Prepare_003, TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->InitOutputController();
    plugin->state_ = State::INITIALIZED;
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DscreenOutputTest, SetParameter_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    std::string value = "dscreen_output_test";
    Status ret = plugin->SetParameter(Tag::USER_FRAME_NUMBER, value);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    plugin->InitOutputController();
    ret = plugin->SetParameter(Tag::USER_FRAME_NUMBER, value);
    EXPECT_EQ(Status::OK, ret);
}


HWTEST_F(DscreenOutputTest, GetParameter_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    std::string value = "dscreen_output_test";
    ValueType val;
    Status ret = plugin->GetParameter(Tag::USER_FRAME_NUMBER, val);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(DscreenOutputTest, GetParameter_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->InitOutputController();
    ValueType val;
    Status ret = plugin->GetParameter(Tag::USER_FRAME_NUMBER, val);
    EXPECT_EQ(Status::ERROR_NOT_EXISTED, ret);
}

HWTEST_F(DscreenOutputTest, GetParameter_003, TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->InitOutputController();
    std::string value = "dscreen_output_test";
    plugin->SetParameter(Tag::USER_FRAME_NUMBER, value);
    plugin->InitOutputController();
    ValueType val;
    Status ret = plugin->GetParameter(Tag::USER_FRAME_NUMBER, val);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DscreenOutputTest, Start_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->state_ = State::INITIALIZED;
    Status ret = plugin->Start();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);

    plugin->state_ = State::PREPARED;
    ret = plugin->Start();
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    plugin->InitOutputController();
    ret = plugin->Start();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DscreenOutputTest, Stop_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->state_ = State::INITIALIZED;
    Status ret = plugin->Stop();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);

    plugin->state_ = State::RUNNING;
    ret = plugin->Stop();
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    plugin->InitOutputController();
    ret = plugin->Stop();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DscreenOutputTest, PushData_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    std::string inPort = "inPort_test";
    std::shared_ptr<Plugin::Buffer> buffer = nullptr;
    int64_t offset = 1;
    Status ret = plugin->PushData(inPort, buffer, offset);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

} // namespace DistributedHardware
} // namespace OHOS