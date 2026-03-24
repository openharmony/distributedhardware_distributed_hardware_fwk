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

class DsoftbusInputAudioPluginCallback : public Callback {
public:
    void OnEvent(const PluginEvent &event)
    {
        (void)event;
    }
};

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

    plugin->ownerName_ = "ohos.dhardware.dcamera";
    ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);

    plugin->peerDevId_ = "peerDevId_";
    ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);

    plugin->isrunning_ = false;
    plugin->HandleData();
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

    ret = plugin->SetParameter(Tag::SECTION_USER_SPECIFIC_START, value);
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

    std::shared_ptr<AVBuffer> buffer = std::make_shared<AVBuffer>();
    plugin->DataEnqueue(buffer);
}

HWTEST_F(DsoftbusInputAudioPluginTest, Reset_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    Status ret = plugin->Reset();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputAudioPluginTest, Reset_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    plugin->bufferPopTask_ = std::make_shared<Media::OSAL::Task>("videoBufferQueuePopThread");
    Status ret = plugin->Reset();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputAudioPluginTest, Pause_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    Status ret = plugin->Pause();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputAudioPluginTest, Resume_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    Status ret = plugin->Resume();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputAudioPluginTest, SetCallback_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    Status ret = plugin->SetCallback(nullptr);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(DsoftbusInputAudioPluginTest, SetDataCallback_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    Status ret = plugin->SetDataCallback(nullptr);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputAudioPluginTest, SetDataCallback_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    DsoftbusInputAudioPluginCallback cb {};
    Status ret = plugin->SetCallback(&cb);
    AVTransEvent event {EventType::EVENT_CHANNEL_OPENED, "", ""};
    plugin->OnChannelEvent(event);

    AVTransEvent event1 {EventType::EVENT_CHANNEL_OPEN_FAIL, "", ""};
    plugin->OnChannelEvent(event1);

    AVTransEvent event2 {EventType::EVENT_CHANNEL_CLOSED, "", ""};
    plugin->OnChannelEvent(event2);

    AVTransEvent event3 {EventType::EVENT_DATA_RECEIVED, "", ""};
    plugin->OnChannelEvent(event3);
    EXPECT_EQ(Status::OK, ret);
}

namespace {
StreamData CreateTestStreamData(const std::string& data)
{
    StreamData streamData;
    streamData.buf = nullptr;
    streamData.bufLen = 0;
    if (data.empty()) {
        return streamData;
    }
    char* buf = new (std::nothrow) char[data.size() + 1];
    if (buf == nullptr) {
        return streamData;
    }
    errno_t err = memcpy_s(buf, data.size() + 1, data.c_str(), data.size() + 1);
    if (err != 0) {
        delete[] buf;
        return streamData;
    }
    streamData.buf = buf;
    streamData.bufLen = static_cast<int>(data.size());
    return streamData;
}

void CleanupStreamData(StreamData& data)
{
    if (data.buf != nullptr) {
        delete[] data.buf;
        data.buf = nullptr;
    }
    data.bufLen = 0;
}

StreamData CreateTestExtData(uint32_t metaType, const std::string& paramJson)
{
    const size_t ESCAPED_QUOTE_LEN = 2;
    std::string escapedParam = paramJson;
    // Escape double quotes for valid JSON string
    size_t pos = 0;
    while ((pos = escapedParam.find('"', pos)) != std::string::npos) {
        escapedParam.replace(pos, 1, R"(\")");
        pos += ESCAPED_QUOTE_LEN;
    }
    std::string jsonStr = R"({"avtrans_data_meta_type": )" + std::to_string(metaType) +
                          R"(, "avtrans_data_param": ")" + escapedParam + R"("})";
    return CreateTestStreamData(jsonStr);
}

std::string CreateValidAudioMetaJson()
{
    return R"({"meta_data_type":0,"meta_timestamp":100,"meta_frame_number":100})";
}

cJSON* CreateTestCJsonObject(uint32_t metaType, const std::string& paramJson)
{
    cJSON* root = cJSON_CreateObject();
    if (root == nullptr) {
        return nullptr;
    }
    cJSON_AddNumberToObject(root, "avtrans_data_meta_type", static_cast<double>(metaType));
    cJSON_AddStringToObject(root, "avtrans_data_param", paramJson.c_str());
    return root;
}
}

HWTEST_F(DsoftbusInputAudioPluginTest, OnStreamReceived_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    plugin->Init();

    StreamData* data = nullptr;
    StreamData* ext = nullptr;

    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    plugin->OnStreamReceived(data, ext);
    EXPECT_EQ(plugin->dataQueue_.size(), 0u);
}

HWTEST_F(DsoftbusInputAudioPluginTest, OnStreamReceived_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    plugin->Init();

    std::string invalidJson = "{invalid json";
    StreamData ext = CreateTestStreamData(invalidJson);

    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    plugin->OnStreamReceived(nullptr, &ext);

    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    CleanupStreamData(ext);
}

HWTEST_F(DsoftbusInputAudioPluginTest, OnStreamReceived_003, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    plugin->Init();

    std::string emptyJson = "{}";
    StreamData ext = CreateTestStreamData(emptyJson);
    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    plugin->OnStreamReceived(nullptr, &ext);
    EXPECT_EQ(plugin->dataQueue_.size(), 0u);
    CleanupStreamData(ext);
}

HWTEST_F(DsoftbusInputAudioPluginTest, OnStreamReceived_004, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    plugin->Init();

    std::string nullTypeJson = R"({"avtrans_data_meta_type": null})";
    StreamData ext = CreateTestStreamData(nullTypeJson);
    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    plugin->OnStreamReceived(nullptr, &ext);
    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    CleanupStreamData(ext);
}

HWTEST_F(DsoftbusInputAudioPluginTest, OnStreamReceived_005, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    plugin->Init();

    std::string stringTypeJson = R"({"avtrans_data_meta_type": "abc"})";
    StreamData ext = CreateTestStreamData(stringTypeJson);
    EXPECT_EQ(plugin->dataQueue_.size(), 0u);
    
    plugin->OnStreamReceived(nullptr, &ext);
    EXPECT_EQ(plugin->dataQueue_.size(), 0u);
    CleanupStreamData(ext);
}

HWTEST_F(DsoftbusInputAudioPluginTest, OnStreamReceived_006, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    plugin->Init();
    plugin->SetDataCallback([](std::shared_ptr<Buffer> buffer) {
    });

    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    std::string audioData = "test audio data";
    StreamData data = CreateTestStreamData(audioData);
    StreamData ext = CreateTestExtData(0, CreateValidAudioMetaJson());

    plugin->OnStreamReceived(&data, &ext);
    EXPECT_EQ(plugin->dataQueue_.size(), 1u);

    CleanupStreamData(data);
    CleanupStreamData(ext);
}

HWTEST_F(DsoftbusInputAudioPluginTest, OnStreamReceived_007, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    plugin->Init();

    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    std::string audioData = "test audio data";
    StreamData data = CreateTestStreamData(audioData);
    std::string missingParamJson = R"({"avtrans_data_meta_type": 0})";
    StreamData ext = CreateTestStreamData(missingParamJson);

    plugin->OnStreamReceived(&data, &ext);
    EXPECT_EQ(plugin->dataQueue_.size(), 1u);

    CleanupStreamData(data);
    CleanupStreamData(ext);
}

HWTEST_F(DsoftbusInputAudioPluginTest, OnStreamReceived_008, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);
    plugin->Init();

    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    StreamData* data = nullptr;
    StreamData ext = CreateTestExtData(0, CreateValidAudioMetaJson());
    plugin->OnStreamReceived(data, &ext);
    EXPECT_EQ(plugin->dataQueue_.size(), 1u);

    CleanupStreamData(ext);
}

HWTEST_F(DsoftbusInputAudioPluginTest, CreateBuffer_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);

    uint32_t metaType = 0;
    StreamData* data = nullptr;
    cJSON* resMsg = cJSON_Parse("{}");

    auto buffer = plugin->CreateBuffer(metaType, data, resMsg);
    EXPECT_EQ(buffer, nullptr);

    if (resMsg != nullptr) {
        cJSON_Delete(resMsg);
    }
}

HWTEST_F(DsoftbusInputAudioPluginTest, CreateBuffer_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);

    uint32_t metaType = 0;
    std::string audioData = "test audio data";
    StreamData data = CreateTestStreamData(audioData);
    cJSON* resMsg = CreateTestCJsonObject(metaType, CreateValidAudioMetaJson());

    auto buffer = plugin->CreateBuffer(metaType, &data, resMsg);
    ASSERT_NE(buffer, nullptr);

    EXPECT_EQ(buffer->pts, 100);
    EXPECT_NE(buffer->GetMemory(), nullptr);
    EXPECT_EQ(buffer->GetMemory()->GetSize(), audioData.size());

    CleanupStreamData(data);
    if (resMsg != nullptr) {
        cJSON_Delete(resMsg);
    }
}

HWTEST_F(DsoftbusInputAudioPluginTest, CreateBuffer_003, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);

    uint32_t metaType = 0;
    std::string audioData = "test audio data";
    StreamData data = CreateTestStreamData(audioData);
    cJSON* resMsg = cJSON_CreateObject();
    cJSON_AddNumberToObject(resMsg, "avtrans_data_meta_type", static_cast<double>(metaType));

    auto buffer = plugin->CreateBuffer(metaType, &data, resMsg);
    EXPECT_EQ(buffer, nullptr);

    CleanupStreamData(data);
    if (resMsg != nullptr) {
        cJSON_Delete(resMsg);
    }
}

HWTEST_F(DsoftbusInputAudioPluginTest, CreateBuffer_004, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);

    uint32_t metaType = 0;
    std::string audioData = "test audio data";
    StreamData data = CreateTestStreamData(audioData);
    cJSON* resMsg = cJSON_CreateObject();
    cJSON_AddNumberToObject(resMsg, "avtrans_data_meta_type", static_cast<double>(metaType));
    cJSON_AddNumberToObject(resMsg, "avtrans_data_param", 123);

    auto buffer = plugin->CreateBuffer(metaType, &data, resMsg);
    EXPECT_EQ(buffer, nullptr);

    CleanupStreamData(data);
    if (resMsg != nullptr) {
        cJSON_Delete(resMsg);
    }
}

HWTEST_F(DsoftbusInputAudioPluginTest, CreateBuffer_005, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputAudioPlugin>(PLUGINNAME);

    uint32_t metaType = 0;
    std::string audioData = "test audio data";
    StreamData data = CreateTestStreamData(audioData);
    cJSON* resMsg = CreateTestCJsonObject(metaType, "invalid json");

    auto buffer = plugin->CreateBuffer(metaType, &data, resMsg);
    EXPECT_NE(buffer, nullptr);

    CleanupStreamData(data);
    if (resMsg != nullptr) {
        cJSON_Delete(resMsg);
    }
}
} // namespace DistributedHardware
} // namespace OHOS