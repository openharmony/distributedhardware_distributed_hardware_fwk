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

#include "dsoftbus_output_audio_plugin_test.h"

#include "dsoftbus_output_audio_plugin.h"

namespace OHOS {
namespace DistributedHardware {
const std::string PLUGINNAME = "dsoftbus_output_audio_plugin";

void DsoftbusOutputAudioPluginTest::SetUpTestCase(void) {}

void DsoftbusOutputAudioPluginTest::TearDownTestCase(void) {}

void DsoftbusOutputAudioPluginTest::SetUp(void) {}

void DsoftbusOutputAudioPluginTest::TearDown(void) {}


HWTEST_F(DsoftbusOutputAudioPluginTest, Prepare_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DsoftbusOutputAudioPlugin>(PLUGINNAME);
    plugin->state_ = State::PREPARED;
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DsoftbusOutputAudioPluginTest, GetParameter_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusOutputAudioPlugin>(PLUGINNAME);
    std::string value = "dsoftbus_output_audio_plugin_test";
    plugin->SetParameter(Tag::MEDIA_DESCRIPTION, value);
    ValueType val;
    Status ret = plugin->GetParameter(Tag::MEDIA_DESCRIPTION, val);
    EXPECT_EQ(Status::OK, ret);

    plugin->paramsMap_.clear();
    ret = plugin->GetParameter(Tag::USER_FRAME_NUMBER, val);
    EXPECT_EQ(Status::ERROR_NOT_EXISTED, ret);
}

HWTEST_F(DsoftbusOutputAudioPluginTest, Start_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusOutputAudioPlugin>(PLUGINNAME);
    Status ret = plugin->Start();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DsoftbusOutputAudioPluginTest, Stop_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusOutputAudioPlugin>(PLUGINNAME);
    Status ret = plugin->Stop();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}
} // namespace DistributedHardware
} // namespace OHOS