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

#include "dsoftbus_input_plugin_test.h"

#include "dsoftbus_input_plugin.h"

namespace OHOS {
namespace DistributedHardware {
const std::string PLUGINNAME = "dsoftbus_input_plugin";

void DsoftbusInputPluginTest::SetUpTestCase(void) {}

void DsoftbusInputPluginTest::TearDownTestCase(void) {}

void DsoftbusInputPluginTest::SetUp(void) {}

void DsoftbusInputPluginTest::TearDown(void) {}

class DaudioInputPluginCallback : public Callback {
public:
    void OnEvent(const PluginEvent &event)
    {
        (void)event;
    }
};

HWTEST_F(DsoftbusInputPluginTest, Prepare_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->state_ = State::PREPARED;
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DsoftbusInputPluginTest, Prepare_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->state_ = State::INITIALIZED;
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);

    plugin->ownerName_ = "ohos.dhardware.dcamera";
    ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);

    plugin->peerDevId_ = "peerDevId_";
    ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
}

HWTEST_F(DsoftbusInputPluginTest, Start_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    Status ret = plugin->Start();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DsoftbusInputPluginTest, Start_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->bufferPopTask_ = std::make_shared<Media::OSAL::Task>("videoBufferQueuePopThread");
    plugin->state_ = State::PREPARED;
    Status ret = plugin->Start();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputPluginTest, Stop_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->state_ = State::PREPARED;
    Status ret = plugin->Stop();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DsoftbusInputPluginTest, Stop_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->bufferPopTask_ = std::make_shared<Media::OSAL::Task>("videoBufferQueuePopThread");
    plugin->state_ = State::RUNNING;
    Status ret = plugin->Stop();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputPluginTest, SetParameter_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->Init();
    std::string value = "dsoftbus_input_test";
    Status ret = plugin->SetParameter(Tag::MEDIA_DESCRIPTION, value);
    EXPECT_EQ(Status::OK, ret);

    bool val = true;
    ret = plugin->SetParameter(Tag::SECTION_USER_SPECIFIC_START, val);
    EXPECT_EQ(Status::OK, ret);

    ret = plugin->SetParameter(Tag::SECTION_VIDEO_SPECIFIC_START, val);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputPluginTest, GetParameter_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    std::string value = "dsoftbus_input_test";
    plugin->SetParameter(Tag::MEDIA_DESCRIPTION, value);
    ValueType val;
    Status ret = plugin->GetParameter(Tag::MEDIA_DESCRIPTION, val);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputPluginTest, GetParameter_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->paramsMap_.clear();
    ValueType val;
    Status ret = plugin->GetParameter(Tag::MEDIA_DESCRIPTION, val);
    EXPECT_EQ(Status::ERROR_NOT_EXISTED, ret);

    AVTransEvent event;
    plugin->OnChannelEvent(event);

    plugin->SetDumpFlagFalse();
    std::shared_ptr<AVBuffer> buffer = std::make_shared<AVBuffer>();
    plugin->DataEnqueue(buffer);
}

HWTEST_F(DsoftbusInputPluginTest, Pause_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    Status ret = plugin->Pause();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputPluginTest, SetCallback_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    Status ret = plugin->SetCallback(nullptr);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    Callback *cb = new DaudioInputPluginCallback();
    ret = plugin->SetCallback(cb);
    EXPECT_EQ(Status::OK, ret);
    delete cb;
}

HWTEST_F(DsoftbusInputPluginTest, Resume_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    Status ret = plugin->Resume();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputPluginTest, SetDataCallback_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    AVDataCallback dataCb;
    Status ret = plugin->SetDataCallback(dataCb);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputPluginTest, Deinit_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->Init();
    Status ret = plugin->Deinit();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusInputPluginTest, Reset_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->Init();
    Status ret = plugin->Reset();
    EXPECT_EQ(Status::OK, ret);
    AVTransEvent event;
    event.type = EventType::EVENT_CHANNEL_OPENED;
    plugin->eventsCb_ = new DaudioInputPluginCallback();
    plugin->OnChannelEvent(event);

    event.type = EventType::EVENT_CHANNEL_OPEN_FAIL;
    plugin->OnChannelEvent(event);

    event.type = EventType::EVENT_CHANNEL_CLOSED;
    plugin->OnChannelEvent(event);

    event.type = EventType::EVENT_START_SUCCESS;
    plugin->OnChannelEvent(event);
    delete plugin->eventsCb_;
    AVTRANS_LOGI("zlf 2");
    plugin->isrunning_ = false;
    plugin->HandleData();
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
    std::string jsonStr = R"({"avtrans_data_meta_type": )" + std::to_string(metaType) +
                          R"(, "avtrans_data_param": ")" + paramJson + R"("})";
    return CreateTestStreamData(jsonStr);
}

std::string CreateValidVideoMetaJson(bool withExtParams = true)
{
    if (withExtParams) {
        return R"({"meta_data_type":1,"meta_timestamp":100,"meta_frame_number":100,
            "meta_ext_timestamp":50, "meta_ext_frame_number":5})";
    }
    return R"({"meta_data_type":1,"meta_timestamp":100,"meta_frame_number":100})";
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

HWTEST_F(DsoftbusInputPluginTest, OnStreamReceived_001, TestSize.Level1)
{
    // Test case: ext is nullptr
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->Init();

    StreamData* data = nullptr;
    StreamData* ext = nullptr;

    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    plugin->OnStreamReceived(data, ext);
    EXPECT_EQ(plugin->dataQueue_.size(), 0u);
}

HWTEST_F(DsoftbusInputPluginTest, OnStreamReceived_002, TestSize.Level1)
{
    // Test case: JSON parse failed
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->Init();

    std::string invalidJson = "{invalid json";
    StreamData ext = CreateTestStreamData(invalidJson);

    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    // Should return early when JSON parsing fails
    plugin->OnStreamReceived(nullptr, &ext);

    // Verify nothing was enqueued
    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    CleanupStreamData(ext);
}

HWTEST_F(DsoftbusInputPluginTest, OnStreamReceived_003, TestSize.Level1)
{
    // Test case: Missing AVT_DATA_META_TYPE field
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->Init();

    std::string emptyJson = "{}";
    StreamData ext = CreateTestStreamData(emptyJson);
    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    plugin->OnStreamReceived(nullptr, &ext);
    EXPECT_EQ(plugin->dataQueue_.size(), 0u);
    CleanupStreamData(ext);
}

HWTEST_F(DsoftbusInputPluginTest, OnStreamReceived_004, TestSize.Level1)
{
    // Test case: AVT_DATA_META_TYPE is null
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->Init();

    std::string nullTypeJson = R"({"avtrans_data_meta_type": null})";
    StreamData ext = CreateTestStreamData(nullTypeJson);
    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    plugin->OnStreamReceived(nullptr, &ext);
    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    CleanupStreamData(ext);
}

HWTEST_F(DsoftbusInputPluginTest, OnStreamReceived_005, TestSize.Level1)
{
    // Test case: AVT_DATA_META_TYPE is not a number
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->Init();

    std::string stringTypeJson = R"({"avtrans_data_meta_type": "abc"})";
    StreamData ext = CreateTestStreamData(stringTypeJson);
    EXPECT_EQ(plugin->dataQueue_.size(), 0u);
    
    plugin->OnStreamReceived(nullptr, &ext);
    EXPECT_EQ(plugin->dataQueue_.size(), 0u);
    CleanupStreamData(ext);
}

HWTEST_F(DsoftbusInputPluginTest, OnStreamReceived_006, TestSize.Level1)
{
    // Test case: Success with VIDEO type (metaType = 1)
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->Init();
    plugin->SetDataCallback([](std::shared_ptr<Buffer> buffer) {
        // Callback to handle received buffer
    });

    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    std::string videoData = "test video data";
    StreamData data = CreateTestStreamData(videoData);
    StreamData ext = CreateTestExtData(1, CreateValidVideoMetaJson());

    plugin->OnStreamReceived(&data, &ext);
    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    CleanupStreamData(data);
    CleanupStreamData(ext);
}

HWTEST_F(DsoftbusInputPluginTest, OnStreamReceived_007, TestSize.Level1)
{
    // Test case: CreateBuffer returns nullptr (missing avtrans_data_param)
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->Init();

    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    std::string videoData = "test video data";
    StreamData data = CreateTestStreamData(videoData);
    std::string missingParamJson = R"({"avtrans_data_meta_type": 1})";
    StreamData ext = CreateTestStreamData(missingParamJson);

    plugin->OnStreamReceived(&data, &ext);
    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    CleanupStreamData(data);
    CleanupStreamData(ext);
}

HWTEST_F(DsoftbusInputPluginTest, OnStreamReceived_008, TestSize.Level1)
{
    // Test case: data is nullptr but ext is valid
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);
    plugin->Init();

    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    StreamData* data = nullptr;
    StreamData ext = CreateTestExtData(1, CreateValidVideoMetaJson());
    plugin->OnStreamReceived(data, &ext);
    EXPECT_EQ(plugin->dataQueue_.size(), 0u);

    CleanupStreamData(ext);
}

HWTEST_F(DsoftbusInputPluginTest, CreateBuffer_001, TestSize.Level1)
{
    // Test case: data is nullptr
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);

    uint32_t metaType = 1;  // VIDEO
    StreamData* data = nullptr;
    cJSON* resMsg = cJSON_Parse("{}");

    auto buffer = plugin->CreateBuffer(metaType, data, resMsg);
    EXPECT_EQ(buffer, nullptr);

    if (resMsg != nullptr) {
        cJSON_Delete(resMsg);
    }
}

HWTEST_F(DsoftbusInputPluginTest, CreateBuffer_002, TestSize.Level1)
{
    // Test case: Valid data with VIDEO type
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);

    uint32_t metaType = 1;  // VIDEO
    std::string videoData = "test video data";
    StreamData data = CreateTestStreamData(videoData);
    cJSON* resMsg = CreateTestCJsonObject(metaType, CreateValidVideoMetaJson());

    auto buffer = plugin->CreateBuffer(metaType, &data, resMsg);
    ASSERT_NE(buffer, nullptr);

    // Verify buffer properties
    EXPECT_EQ(buffer->pts, 100);
    EXPECT_NE(buffer->GetMemory(), nullptr);
    EXPECT_EQ(buffer->GetMemory()->GetSize(), videoData.size());

    CleanupStreamData(data);
    if (resMsg != nullptr) {
        cJSON_Delete(resMsg);
    }
}

HWTEST_F(DsoftbusInputPluginTest, CreateBuffer_003, TestSize.Level1)
{
    // Test case: Missing AVT_DATA_PARAM field
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);

    uint32_t metaType = 1;  // VIDEO
    std::string videoData = "test video data";
    StreamData data = CreateTestStreamData(videoData);
    cJSON* resMsg = cJSON_CreateObject();
    cJSON_AddNumberToObject(resMsg, "avtrans_data_meta_type", static_cast<double>(metaType));

    auto buffer = plugin->CreateBuffer(metaType, &data, resMsg);
    EXPECT_EQ(buffer, nullptr);

    CleanupStreamData(data);
    if (resMsg != nullptr) {
        cJSON_Delete(resMsg);
    }
}

HWTEST_F(DsoftbusInputPluginTest, CreateBuffer_004, TestSize.Level1)
{
    // Test case: AVT_DATA_PARAM is not a string
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);

    uint32_t metaType = 1;  // VIDEO
    std::string videoData = "test video data";
    StreamData data = CreateTestStreamData(videoData);
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

HWTEST_F(DsoftbusInputPluginTest, CreateBuffer_005, TestSize.Level1)
{
    // Test case: UnmarshalVideoMeta failed (invalid JSON in param)
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);

    uint32_t metaType = 1;  // VIDEO
    std::string videoData = "test video data";
    StreamData data = CreateTestStreamData(videoData);
    cJSON* resMsg = CreateTestCJsonObject(metaType, "invalid json");

    auto buffer = plugin->CreateBuffer(metaType, &data, resMsg);
    EXPECT_EQ(buffer, nullptr);

    CleanupStreamData(data);
    if (resMsg != nullptr) {
        cJSON_Delete(resMsg);
    }
}

HWTEST_F(DsoftbusInputPluginTest, CreateBuffer_006, TestSize.Level1)
{
    // Test case: Success with AUDIO type (metaType = 0)
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);

    uint32_t metaType = 0;  // AUDIO
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

HWTEST_F(DsoftbusInputPluginTest, CreateBuffer_007, TestSize.Level1)
{
    // Test case: Success with VIDEO type (metaType = 1)
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);

    uint32_t metaType = 1;  // VIDEO
    std::string videoData = "test video data";
    StreamData data = CreateTestStreamData(videoData);
    cJSON* resMsg = CreateTestCJsonObject(metaType, CreateValidVideoMetaJson(false));

    auto buffer = plugin->CreateBuffer(metaType, &data, resMsg);
    ASSERT_NE(buffer, nullptr);
    EXPECT_EQ(buffer->pts, 100);
    EXPECT_NE(buffer->GetMemory(), nullptr);
    EXPECT_EQ(buffer->GetMemory()->GetSize(), videoData.size());

    CleanupStreamData(data);
    if (resMsg != nullptr) {
        cJSON_Delete(resMsg);
    }
}

HWTEST_F(DsoftbusInputPluginTest, CreateBuffer_008, TestSize.Level1)
{
    // Test case: VIDEO with extended fields (extPts > 0 and extFrameNum > 0)
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);

    uint32_t metaType = 1;  // VIDEO
    std::string videoData = "test video data";
    StreamData data = CreateTestStreamData(videoData);
    cJSON* resMsg = CreateTestCJsonObject(metaType, CreateValidVideoMetaJson(true));

    auto buffer = plugin->CreateBuffer(metaType, &data, resMsg);
    ASSERT_NE(buffer, nullptr);
    EXPECT_EQ(buffer->pts, 100);
    EXPECT_NE(buffer->GetMemory(), nullptr);

    CleanupStreamData(data);
    if (resMsg != nullptr) {
        cJSON_Delete(resMsg);
    }
}

HWTEST_F(DsoftbusInputPluginTest, CreateBuffer_009, TestSize.Level1)
{
    // Test case: VIDEO without extended fields (extPts = 0 or extFrameNum = 0)
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);

    uint32_t metaType = 1;  // VIDEO
    std::string videoData = "test video data";
    StreamData data = CreateTestStreamData(videoData);
    // Create JSON with extPts = 0
    std::string metaJson = R"({"meta_data_type":1,"meta_timestamp":100,"meta_frame_number":100})";
    cJSON* resMsg = CreateTestCJsonObject(metaType, metaJson);

    auto buffer = plugin->CreateBuffer(metaType, &data, resMsg);
    ASSERT_NE(buffer, nullptr);

    // Verify buffer properties
    EXPECT_EQ(buffer->pts, 100);
    EXPECT_NE(buffer->GetMemory(), nullptr);
    EXPECT_EQ(buffer->GetMemory()->GetSize(), videoData.size());

    CleanupStreamData(data);
    if (resMsg != nullptr) {
        cJSON_Delete(resMsg);
    }
}

HWTEST_F(DsoftbusInputPluginTest, CreateBuffer_010, TestSize.Level1)
{
    // Test case: VIDEO with only extFrameNum > 0 but extPts = 0
    auto plugin = std::make_shared<DsoftbusInputPlugin>(PLUGINNAME);

    uint32_t metaType = 1;  // VIDEO
    std::string videoData = "test video data";
    StreamData data = CreateTestStreamData(videoData);
    // Create JSON with extPts = 0, extFrameNum > 0
    // Extended metadata should NOT be set since condition requires both > 0
    std::string metaJson = R"({"meta_data_type":1,"meta_timestamp":100,"meta_frame_number":100})";
    cJSON* resMsg = CreateTestCJsonObject(metaType, metaJson);

    auto buffer = plugin->CreateBuffer(metaType, &data, resMsg);
    ASSERT_NE(buffer, nullptr);

    // Verify buffer properties
    EXPECT_EQ(buffer->pts, 100);
    EXPECT_NE(buffer->GetMemory(), nullptr);

    CleanupStreamData(data);
    if (resMsg != nullptr) {
        cJSON_Delete(resMsg);
    }
}
} // namespace DistributedHardware
} // namespace OHOS