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

class DsoftbusOutPutAudioPluginCallback : public Callback {
public:
    void OnEvent(const PluginEvent &event)
    {
        (void)event;
    }
};

HWTEST_F(DsoftbusOutputAudioPluginTest, Reset_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DsoftbusOutputAudioPlugin>(PLUGINNAME);
    Status ret = plugin->Reset();
    EXPECT_EQ(Status::OK, ret);
    plugin->bufferPopTask_ = std::make_shared<Media::OSAL::Task>("audioBufferQueuePopThread");
    ret = plugin->Reset();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusOutputAudioPluginTest, Prepare_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DsoftbusOutputAudioPlugin>(PLUGINNAME);
    plugin->state_ = State::PREPARED;
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DsoftbusOutputAudioPluginTest, Prepare_002, TestSize.Level0)
{
    auto plugin = std::make_shared<DsoftbusOutputAudioPlugin>(PLUGINNAME);
    plugin->state_ = State::INITIALIZED;
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);

    plugin->ownerName_ = "ohos.dhardware.dcamera";
    ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);

    plugin->ownerName_ = "";
    ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
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

    plugin->state_ = State::PREPARED;
    ret = plugin->Start();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);

    plugin->sessionName_ = "sessionName";
    ret = plugin->Start();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);

    plugin->ownerName_ = "ohos.dhardware.dcamera";
    ret = plugin->Start();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);

    plugin->peerDevId_ = "peerDevId";
    ret = plugin->Start();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
}

HWTEST_F(DsoftbusOutputAudioPluginTest, Stop_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusOutputAudioPlugin>(PLUGINNAME);
    Status ret = plugin->Stop();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DsoftbusOutputAudioPluginTest, PushData_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusOutputAudioPlugin>(PLUGINNAME);
    std::string inPort = "inPort_test";
    std::shared_ptr<Plugin::Buffer> buffer = nullptr;
    int64_t offset = 1;
    Status ret = plugin->PushData(inPort, buffer, offset);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    buffer = std::make_shared<Plugin::Buffer>(BufferMetaType::AUDIO);
    ret = plugin->PushData(inPort, buffer, offset);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    size_t bufferSize = 1024;
    std::vector<uint8_t> buff(bufferSize);
    auto bufData = buffer->WrapMemory(buff.data(), bufferSize, bufferSize);
    ret = plugin->PushData(inPort, buffer, offset);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusOutputAudioPluginTest, SetParameter_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusOutputAudioPlugin>(PLUGINNAME);
    std::string value = "dsoftbus_output_audio_plugin_test";
    Status ret = plugin->SetParameter(Tag::MEDIA_DESCRIPTION, value);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusOutputAudioPluginTest, SetCallback_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusOutputAudioPlugin>(PLUGINNAME);
    Status ret = plugin->SetCallback(nullptr);
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);

    DsoftbusOutPutAudioPluginCallback cb {};
    ret = plugin->SetCallback(&cb);
    AVTransEvent event;
    event.type = EventType::EVENT_CHANNEL_OPENED;
    plugin->OnChannelEvent(event);
    event.type = EventType::EVENT_CHANNEL_OPEN_FAIL;
    plugin->OnChannelEvent(event);
    event.type = EventType::EVENT_CHANNEL_CLOSED;
    plugin->OnChannelEvent(event);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusOutputAudioPluginTest, OpenSoftbusChannel_002, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusOutputAudioPlugin>(PLUGINNAME);
    Status ret = plugin->OpenSoftbusChannel();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);

    plugin->sessionName_ = "sessionName";
    ret = plugin->OpenSoftbusChannel();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);

    plugin->ownerName_ = "ohos.dhardware.dcamera";
    ret = plugin->OpenSoftbusChannel();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);

    plugin->peerDevId_ = "peerDevId";
    ret = plugin->OpenSoftbusChannel();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
}

HWTEST_F(DsoftbusOutputAudioPluginTest, SetDataCallback_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusOutputAudioPlugin>(PLUGINNAME);
    AVDataCallback cbk;
    Status ret = plugin->SetDataCallback(cbk);
    EXPECT_EQ(Status::OK, ret);
}
} // namespace DistributedHardware
} // namespace OHOS