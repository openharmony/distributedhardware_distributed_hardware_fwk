/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "daudio_output_test.h"

#include "av_trans_utils.h"
#include "daudio_output_plugin.h"

namespace OHOS {
namespace DistributedHardware {
const std::string PLUGINNAME = "daudio_output";

void DaudioOutputTest::SetUpTestCase() {}

void DaudioOutputTest::TearDownTestCase() {}

void DaudioOutputTest::SetUp() {}

void DaudioOutputTest::TearDown() {}

class DaudioOutputPluginCallback : public Callback {
public:
    void OnEvent(const PluginEvent &event)
    {
        (void)event;
    }
};

HWTEST_F(DaudioOutputTest, Reset_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    Status ret = plugin->Reset();
    EXPECT_EQ(Status::OK, ret);

    plugin->sendPlayTask_ = std::make_shared<OHOS::Media::OSAL::Task>("sendPlayTask_");
    plugin->resample_ = std::make_shared<Ffmpeg::Resample>();
    ret = plugin->Reset();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DaudioOutputTest, Prepare_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    plugin->state_ = State::PREPARED;
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);

    uint32_t channels = 1;
    uint32_t sampleRate = 1;
    uint32_t channelLayout = 1;
    plugin->RampleInit(channels, sampleRate, channelLayout);
}

HWTEST_F(DaudioOutputTest, Prepare_002, TestSize.Level0)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    plugin->state_ = State::INITIALIZED;
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_UNKNOWN, ret);

    int value = 1;
    plugin->SetParameter(Tag::AUDIO_SAMPLE_RATE, value);
    ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_UNKNOWN, ret);

    plugin->paramsMap_.clear();
    plugin->SetParameter(Tag::AUDIO_CHANNELS, value);
    ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_UNKNOWN, ret);

    plugin->SetParameter(Tag::AUDIO_SAMPLE_RATE, value);
    ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_UNKNOWN, ret);

    plugin->SetParameter(Tag::AUDIO_CHANNEL_LAYOUT, value);
    ret = plugin->Prepare();
    EXPECT_EQ(Status::OK, ret);

    value = 2;
    plugin->SetParameter(Tag::AUDIO_CHANNEL_LAYOUT, value);
    ret = plugin->Prepare();
    EXPECT_NE(Status::OK, ret);
}

HWTEST_F(DaudioOutputTest, Prepare_003, TestSize.Level0)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    plugin->state_ = State::INITIALIZED;
    std::shared_ptr<OSAL::Task> sendPlayTask_ = nullptr;
    int value = 1;
    plugin->SetParameter(Tag::AUDIO_SAMPLE_RATE, value);
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_UNKNOWN, ret);
}


HWTEST_F(DaudioOutputTest, SetParameter_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    std::string value = "dsoftbus_output_test";
    Status ret = plugin->SetParameter(Tag::USER_AV_SYNC_GROUP_INFO, value);
    EXPECT_EQ(Status::OK, ret);

    ret = plugin->SetParameter(Tag::USER_SHARED_MEMORY_FD, value);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DaudioOutputTest, GetParameter_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    std::string value = "dsoftbus_output_test";
    plugin->SetParameter(Tag::USER_AV_SYNC_GROUP_INFO, value);
    ValueType val;
    Status ret = plugin->GetParameter(Tag::USER_AV_SYNC_GROUP_INFO, val);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DaudioOutputTest, GetParameter_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    plugin->paramsMap_.clear();
    ValueType val;
    Status ret = plugin->GetParameter(Tag::USER_AV_SYNC_GROUP_INFO, val);
    EXPECT_EQ(Status::ERROR_NOT_EXISTED, ret);
}

HWTEST_F(DaudioOutputTest, Start_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    Status ret = plugin->Start();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DaudioOutputTest, Start_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    plugin->sendPlayTask_ = std::make_shared<Media::OSAL::Task>("sendPlayTask_");
    plugin->state_ = State::PREPARED;
    Status ret = plugin->Start();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DaudioOutputTest, Stop_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    Status ret = plugin->Stop();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DaudioOutputTest, Stop_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    plugin->sendPlayTask_ = std::make_shared<Media::OSAL::Task>("sendPlayTask_");
    plugin->state_ = State::RUNNING;
    Status ret = plugin->Stop();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DaudioOutputTest, PushData_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    std::string inPort = "inPort_test";
    int64_t offset = 1;
    Status ret = plugin->PushData(inPort, nullptr, offset);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    std::shared_ptr<Plugin::Buffer> buffer = std::make_shared<Plugin::Buffer>(BufferMetaType::AUDIO);
    ret = plugin->PushData(inPort, buffer, offset);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    size_t bufferSize = 1024;
    std::vector<uint8_t> buff(bufferSize);
    auto bufData = buffer->WrapMemory(buff.data(), bufferSize, bufferSize);
    ret = plugin->PushData(inPort, buffer, offset);
    EXPECT_EQ(Status::OK, ret);

    for (int32_t i = 0; i < bufferSize; i++) {
        plugin->outputBuffer_.push(buffer);
    }
    ret = plugin->PushData(inPort, buffer, offset);
    EXPECT_EQ(Status::OK, ret);

    plugin->isrunning_ = false;
    plugin->HandleData();
}

HWTEST_F(DaudioOutputTest, SetDataCallback_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    AVDataCallback callback = nullptr;
    Status ret = plugin->SetDataCallback(callback);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(DaudioOutputTest, WriteMasterClockToMemory_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    std::string value = "dsoftbus_output_test";
    Status ret = plugin->SetParameter(Tag::USER_AV_SYNC_GROUP_INFO, value);
    std::shared_ptr<AVBuffer> buffer = std::make_shared<AVBuffer>();
    plugin->WriteMasterClockToMemory(buffer);

    plugin->sharedMemory_.fd = 1;
    plugin->WriteMasterClockToMemory(buffer);

    plugin->sharedMemory_.fd = 0;
    plugin->sharedMemory_.size = 1;
    plugin->WriteMasterClockToMemory(buffer);

    plugin->sharedMemory_.fd = 1;
    plugin->sharedMemory_.size = 1;
    plugin->WriteMasterClockToMemory(buffer);

    plugin->sharedMemory_.fd = 0;
    plugin->sharedMemory_.size = 0;
    plugin->sharedMemory_.name = "sharedMemory_";
    plugin->WriteMasterClockToMemory(buffer);

    plugin->sharedMemory_.fd = 1;
    plugin->sharedMemory_.size = 1;
    plugin->sharedMemory_.name = "sharedMemory_";
    plugin->WriteMasterClockToMemory(buffer);

    buffer = nullptr;
    plugin->WriteMasterClockToMemory(buffer);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DaudioOutputTest, SetCallback_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    Status ret = plugin->SetCallback(nullptr);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    DaudioOutputPluginCallback cb {};
    ret = plugin->SetCallback(&cb);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DaudioOutputTest, StartOutputQueue_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    plugin->state_ = State::RUNNING;
    std::shared_ptr<Plugin::Buffer> buffer = nullptr;
    std::shared_ptr<OSAL::Task> sendPlayTask_ = nullptr;
    plugin->Prepare();
    Status ret = plugin->StartOutputQueue();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DaudioOutputTest, ControlFrameRate_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DaudioOutputPlugin>(PLUGINNAME);
    const int64_t timestamp = 1;
    Status ret = plugin->ControlFrameRate(timestamp);
    EXPECT_EQ(Status::OK, ret);
}

} // namespace DistributedHardware
} // namespace OHOS