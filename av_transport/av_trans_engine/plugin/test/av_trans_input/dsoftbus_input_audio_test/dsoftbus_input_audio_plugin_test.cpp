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

#include "dsoftbus_input_audio_plugin_test.h"

#include "dsoftbus_input_audio_plugin.h"

namespace OHOS {
namespace DistributedHardware {
const std::string PLUGINNAME = "dsoftbus_input_audio_plugin";

void DsoftbusInputAudioPluginTest::SetUpTestCase(void) {}

void DsoftbusInputAudioPluginTest::TearDownTestCase(void) {}

void DsoftbusInputAudioPluginTest::SetUp(void) {}

void DsoftbusInputAudioPluginTest::TearDown(void) {}

HWTEST_F(DsoftbusInputAudioPluginTest, Prepare_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    plugin->state_ = State::PREPARED;
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DsoftbusInputAudioPluginTest, Prepare_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    plugin->state_ = State::INITIALIZED;
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
}

HWTEST_F(DsoftbusInputAudioPluginTest, Start_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    Status ret = plugin->Start();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DsoftbusInputAudioPluginTest, Start_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    plugin->bufferPopTask_ = std::make_shared<Media::OSAL::Task>("videoBufferQueuePopThread");
    plugin->state_ = State::PREPARED;
    Status ret = plugin->Start();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputAudioPluginTest, Stop_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    plugin->state_ = State::PREPARED;
    Status ret = plugin->Stop();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DsoftbusInputAudioPluginTest, Stop_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    plugin->bufferPopTask_ = std::make_shared<Media::OSAL::Task>("videoBufferQueuePopThread");
    plugin->state_ = State::RUNNING;
    Status ret = plugin->Stop();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputAudioPluginTest, SetParameter_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    plugin->Init();
    std::string value = "dsoftbus_input_test";
    Status ret = plugin->SetParameter(Tag::MEDIA_DESCRIPTION, value);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputAudioPluginTest, GetParameter_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    std::string value = "dsoftbus_input_test";
    plugin->SetParameter(Tag::MEDIA_DESCRIPTION, value);
    ValueType val;
    Status ret = plugin->GetParameter(Tag::MEDIA_DESCRIPTION, val);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputAudioPluginTest, GetParameter_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    plugin->paramsMap_.clear();
    ValueType val;
    Status ret = plugin->GetParameter(Tag::MEDIA_DESCRIPTION, val);
    EXPECT_EQ(Status::ERROR_NOT_EXISTED, ret);

    AVTransEvent event;
    plugin->OnChannelEvent(event);
}
} // namespace DistributedHardware
} // namespace OHOS