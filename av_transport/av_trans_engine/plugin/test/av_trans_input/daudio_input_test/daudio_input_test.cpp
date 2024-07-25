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

#include "daudio_input_test.h"

#include "av_trans_message.h"
#include "av_trans_errno.h"
#include "softbus_channel_adapter.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    const std::string META_DATA_TYPE = "meta_data_type";
    const std::string META_TIMESTAMP = "meta_timestamp";
    const std::string META_FRAME_NUMBER = "meta_frame_number";
    const std::string META_EXT_TIMESTAMP = "meta_ext_timestamp";
    const std::string META_EXT_FRAME_NUMBER = "meta_ext_frame_number";
    const std::string KEY_OWNER_NAME = "ownerName";
    const std::string KEY_PEER_DEVID = "peerDevId";
    const std::string KEY_TYPE = "type";
    const std::string KEY_CONTENT = "content";
    const std::string PLUGINNAME = "daudio_input";
}

void DaudioInputTest::SetUpTestCase() {}

void DaudioInputTest::TearDownTestCase() {}

void DaudioInputTest::SetUp() {}

void DaudioInputTest::TearDown() {}

HWTEST_F(DaudioInputTest, Pause_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DaudioInputPlugin>(PLUGINNAME);
    AVTransSharedMemory sharedMemory1 {1, 0, ""};
    plugin->sharedMemory_ = sharedMemory1;
    Status ret = plugin->Pause();
    EXPECT_EQ(Status::OK, ret);

    AVTransSharedMemory sharedMemory2 { 1, 1, "" };
    plugin->sharedMemory_ = sharedMemory2;
    ret = plugin->Pause();
    EXPECT_EQ(Status::OK, ret);

    AVTransSharedMemory sharedMemory3 { 1, 1, "name" };
    plugin->sharedMemory_ = sharedMemory3;
    ret = plugin->Pause();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DaudioInputTest, SetParameter_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DaudioInputPlugin>(PLUGINNAME);
    std::string value = "dsoftbus_input_test";
    Status ret = plugin->SetParameter(Tag::USER_SHARED_MEMORY_FD, value);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DaudioInputTest, GetParameter_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DaudioInputPlugin>(PLUGINNAME);
    std::string value = "dsoftbus_input_test";
    plugin->SetParameter(Tag::USER_SHARED_MEMORY_FD, value);

    ValueType val;
    Status ret = plugin->GetParameter(Tag::USER_SHARED_MEMORY_FD, val);
    EXPECT_EQ(Status::OK, ret);

    plugin->tagMap_.clear();
    ret = plugin->GetParameter(Tag::USER_SHARED_MEMORY_FD, val);
    EXPECT_EQ(Status::ERROR_NOT_EXISTED, ret);
}

HWTEST_F(DaudioInputTest, PushData_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DaudioInputPlugin>(PLUGINNAME);
    std::shared_ptr<AVBuffer> buffer = nullptr;
    Status ret = plugin->PushData("", buffer, 0);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    buffer = std::make_shared<AVBuffer>();
    ret = plugin->PushData("", buffer, 0);
    EXPECT_EQ(Status::ERROR_INVALID_PARAMETER, ret);

    buffer->AllocMemory(nullptr, 10);
    buffer->GetMemory()->Write((uint8_t*)"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 10);
    ret = plugin->PushData("", buffer, 0);
    EXPECT_EQ(Status::OK, ret);

    plugin->sharedMemory_.fd = 1;
    ret = plugin->PushData("", buffer, 0);
    EXPECT_EQ(Status::OK, ret);

    plugin->sharedMemory_.fd = 1;
    plugin->sharedMemory_.size = 1;
    ret = plugin->PushData("", buffer, 0);
    EXPECT_EQ(Status::OK, ret);

    plugin->sharedMemory_.fd = 0;
    plugin->sharedMemory_.size = 0;
    plugin->sharedMemory_.name = "sharedMemory_";
    ret = plugin->PushData("", buffer, 0);
    EXPECT_EQ(Status::OK, ret);

    plugin->sharedMemory_.fd = 1;
    plugin->sharedMemory_.size = 1;
    plugin->sharedMemory_.name = "sharedMemory_";
    ret = plugin->PushData("", buffer, 0);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DaudioInputTest, MarshalAudioMeta_001, TestSize.Level0)
{
    auto meta = std::make_shared<AVTransAudioBufferMeta>();
    meta->dataType_ = BufferDataType::AUDIO;
    meta->pts_ = 10;
    meta->frameNum_ = 10;
    auto ret = meta->MarshalAudioMeta();
    EXPECT_EQ(false, ret.empty());
}

HWTEST_F(DaudioInputTest, UnmarshalAudioMeta_001, TestSize.Level0)
{
    auto meta = std::make_shared<AVTransAudioBufferMeta>();
    cJSON *metaJson = cJSON_CreateObject();
    cJSON_AddStringToObject(metaJson, META_DATA_TYPE.c_str(), "data_type_test");
    cJSON_AddStringToObject(metaJson, META_TIMESTAMP.c_str(), "timestamp_test");
    cJSON_AddStringToObject(metaJson, META_FRAME_NUMBER.c_str(), "frame_num_test");
    char* cjson = cJSON_PrintUnformatted(metaJson);
    std::string jsonStr(cjson);
    auto ret = meta->UnmarshalAudioMeta(jsonStr);
    EXPECT_EQ(false, ret);
    cJSON_free(cjson);
    cJSON_Delete(metaJson);

    cJSON *metaJson1 = cJSON_CreateObject();
    cJSON_AddNumberToObject(metaJson1, META_DATA_TYPE.c_str(), static_cast<uint32_t>(BufferDataType::AUDIO));
    cJSON_AddStringToObject(metaJson1, META_TIMESTAMP.c_str(), "timestamp_test");
    cJSON_AddStringToObject(metaJson1, META_FRAME_NUMBER.c_str(), "frame_num_test");
    char* cjson1 = cJSON_PrintUnformatted(metaJson1);
    std::string jsonStr1(cjson1);
    ret = meta->UnmarshalAudioMeta(jsonStr1);
    EXPECT_EQ(false, ret);
    cJSON_free(cjson1);
    cJSON_Delete(metaJson1);

    cJSON *metaJson2 = cJSON_CreateObject();
    cJSON_AddNumberToObject(metaJson2, META_DATA_TYPE.c_str(), static_cast<uint32_t>(BufferDataType::AUDIO));
    cJSON_AddNumberToObject(metaJson2, META_TIMESTAMP.c_str(), 100);
    cJSON_AddStringToObject(metaJson2, META_FRAME_NUMBER.c_str(), "frame_num_test");
    char* cjson2 = cJSON_PrintUnformatted(metaJson2);
    std::string jsonStr2(cjson2);
    ret = meta->UnmarshalAudioMeta(jsonStr2);
    EXPECT_EQ(false, ret);
    cJSON_free(cjson2);
    cJSON_Delete(metaJson2);
}

HWTEST_F(DaudioInputTest, UnmarshalAudioMeta_002, TestSize.Level0)
{
    auto meta = std::make_shared<AVTransAudioBufferMeta>();
    cJSON *metaJson3 = cJSON_CreateObject();
    cJSON_AddNumberToObject(metaJson3, META_DATA_TYPE.c_str(), static_cast<uint32_t>(BufferDataType::AUDIO));
    cJSON_AddStringToObject(metaJson3, META_TIMESTAMP.c_str(), "timestamp_test");
    cJSON_AddNumberToObject(metaJson3, META_FRAME_NUMBER.c_str(), 10);
    char* cjson3 = cJSON_PrintUnformatted(metaJson3);
    std::string jsonStr3(cjson3);
    auto ret = meta->UnmarshalAudioMeta(jsonStr3);
    EXPECT_EQ(false, ret);
    cJSON_free(cjson3);
    cJSON_Delete(metaJson3);

    cJSON *metaJson4 = cJSON_CreateObject();
    cJSON_AddStringToObject(metaJson4, META_DATA_TYPE.c_str(), "data_type_test");
    cJSON_AddNumberToObject(metaJson4, META_TIMESTAMP.c_str(), 100);
    cJSON_AddNumberToObject(metaJson4, META_FRAME_NUMBER.c_str(), 10);
    char* cjson4 = cJSON_PrintUnformatted(metaJson4);
    std::string jsonStr4(cjson4);
    ret = meta->UnmarshalAudioMeta(jsonStr4);
    EXPECT_EQ(false, ret);
    cJSON_free(cjson4);
    cJSON_Delete(metaJson4);

    cJSON *metaJson5 = cJSON_CreateObject();
    cJSON_AddNumberToObject(metaJson5, META_DATA_TYPE.c_str(), static_cast<uint32_t>(BufferDataType::AUDIO));
    cJSON_AddNumberToObject(metaJson5, META_TIMESTAMP.c_str(), 100);
    cJSON_AddNumberToObject(metaJson5, META_FRAME_NUMBER.c_str(), 10);
    char* cjson5 = cJSON_PrintUnformatted(metaJson5);
    std::string jsonStr5(cjson5);
    ret = meta->UnmarshalAudioMeta(jsonStr5);
    EXPECT_EQ(true, ret);
    cJSON_free(cjson5);
    cJSON_Delete(metaJson5);
}

HWTEST_F(DaudioInputTest, MarshalVideoMeta_001, TestSize.Level0)
{
    auto meta = std::make_shared<AVTransVideoBufferMeta>();
    meta->dataType_ = BufferDataType::AUDIO;
    meta->pts_ = 10;
    meta->frameNum_ = 10;
    meta->extPts_ = 1;
    meta->extFrameNum_ = 1;
    auto ret = meta->MarshalVideoMeta();
    EXPECT_EQ(false, ret.empty());
}

HWTEST_F(DaudioInputTest, MarshalVideoMeta_002, TestSize.Level0)
{
    auto meta = std::make_shared<AVTransVideoBufferMeta>();
    meta->dataType_ = BufferDataType::AUDIO;
    meta->pts_ = 10;
    meta->frameNum_ = 10;
    meta->extPts_ = 0;
    meta->extFrameNum_ = 0;
    auto ret = meta->MarshalVideoMeta();
    EXPECT_EQ(false, ret.empty());
}

HWTEST_F(DaudioInputTest, UnmarshalVideoMeta_001, TestSize.Level0)
{
    auto meta = std::make_shared<AVTransVideoBufferMeta>();
    std::string str = "";
    auto ret = meta->UnmarshalVideoMeta(str);
    EXPECT_EQ(false, ret);

    cJSON *metaJson = cJSON_CreateObject();
    cJSON_AddStringToObject(metaJson, META_DATA_TYPE.c_str(), "data_type_test");
    char* cjson = cJSON_PrintUnformatted(metaJson);
    std::string jsonStr(cjson);
    ret = meta->UnmarshalVideoMeta(jsonStr);
    EXPECT_EQ(true, ret);
    cJSON_free(cjson);
    cJSON_Delete(metaJson);

    cJSON *metaJson1 = cJSON_CreateObject();
    cJSON_AddNumberToObject(metaJson1, META_DATA_TYPE.c_str(), static_cast<uint32_t>(BufferDataType::AUDIO));
    cJSON_AddStringToObject(metaJson1, META_TIMESTAMP.c_str(), "timestamp_test");
    char* cjson1 = cJSON_PrintUnformatted(metaJson1);
    std::string jsonStr1(cjson1);
    ret = meta->UnmarshalVideoMeta(jsonStr1);
    EXPECT_EQ(true, ret);
    cJSON_free(cjson1);
    cJSON_Delete(metaJson1);

    cJSON *metaJson2 = cJSON_CreateObject();
    cJSON_AddNumberToObject(metaJson2, META_DATA_TYPE.c_str(), static_cast<uint32_t>(BufferDataType::AUDIO));
    cJSON_AddNumberToObject(metaJson2, META_TIMESTAMP.c_str(), 100);
    cJSON_AddStringToObject(metaJson2, META_FRAME_NUMBER.c_str(), "frame_num_test");
    char* cjson2 = cJSON_PrintUnformatted(metaJson2);
    std::string jsonStr2(cjson2);
    ret = meta->UnmarshalVideoMeta(jsonStr2);
    EXPECT_EQ(true, ret);
    cJSON_free(cjson2);
    cJSON_Delete(metaJson2);
}

HWTEST_F(DaudioInputTest, UnmarshalVideoMeta_002, TestSize.Level0)
{
    auto meta = std::make_shared<AVTransVideoBufferMeta>();
    cJSON *metaJson3 = cJSON_CreateObject();
    cJSON_AddNumberToObject(metaJson3, META_DATA_TYPE.c_str(), static_cast<uint32_t>(BufferDataType::AUDIO));
    cJSON_AddNumberToObject(metaJson3, META_TIMESTAMP.c_str(), 100);
    cJSON_AddNumberToObject(metaJson3, META_FRAME_NUMBER.c_str(), 10);
    cJSON_AddStringToObject(metaJson3, META_EXT_TIMESTAMP.c_str(), "ext_timestamp_test");
    char* cjson3 = cJSON_PrintUnformatted(metaJson3);
    std::string jsonStr3(cjson3);
    auto ret = meta->UnmarshalVideoMeta(jsonStr3);
    EXPECT_EQ(true, ret);
    cJSON_free(cjson3);
    cJSON_Delete(metaJson3);

    cJSON *metaJson4 = cJSON_CreateObject();
    cJSON_AddNumberToObject(metaJson4, META_DATA_TYPE.c_str(), static_cast<uint32_t>(BufferDataType::AUDIO));
    cJSON_AddNumberToObject(metaJson4, META_TIMESTAMP.c_str(), 100);
    cJSON_AddNumberToObject(metaJson4, META_FRAME_NUMBER.c_str(), 10);
    cJSON_AddNumberToObject(metaJson4, META_EXT_TIMESTAMP.c_str(), 10);
    cJSON_AddStringToObject(metaJson4, META_EXT_FRAME_NUMBER.c_str(), "ext_frame_num_test");
    char* cjson4 = cJSON_PrintUnformatted(metaJson4);
    std::string jsonStr4(cjson4);
    ret = meta->UnmarshalVideoMeta(jsonStr4);
    EXPECT_EQ(true, ret);
    cJSON_free(cjson4);
    cJSON_Delete(metaJson4);

    cJSON *metaJson5 = cJSON_CreateObject();
    cJSON_AddNumberToObject(metaJson5, META_DATA_TYPE.c_str(), static_cast<uint32_t>(BufferDataType::AUDIO));
    cJSON_AddNumberToObject(metaJson5, META_TIMESTAMP.c_str(), 100);
    cJSON_AddNumberToObject(metaJson5, META_FRAME_NUMBER.c_str(), 10);
    cJSON_AddNumberToObject(metaJson5, META_EXT_TIMESTAMP.c_str(), 10);
    cJSON_AddNumberToObject(metaJson5, META_EXT_FRAME_NUMBER.c_str(), 10);
    char* cjson5 = cJSON_PrintUnformatted(metaJson5);
    std::string jsonStr5(cjson5);
    ret = meta->UnmarshalVideoMeta(jsonStr5);
    EXPECT_EQ(true, ret);
    cJSON_free(cjson5);
    cJSON_Delete(metaJson5);
}

HWTEST_F(DaudioInputTest, ParseChannelDescription_001, TestSize.Level0)
{
    std::string descJsonStr = "";
    std::string ownerName = "";
    std::string peerDevId = "";
    ParseChannelDescription(descJsonStr, ownerName, peerDevId);
    EXPECT_EQ(true, descJsonStr.empty());
}

HWTEST_F(DaudioInputTest, ParseChannelDescription_002, TestSize.Level0)
{
    std::string ownerName = "";
    std::string peerDevId = "";
    cJSON *metaJson = cJSON_CreateObject();
    cJSON_AddNumberToObject(metaJson, KEY_OWNER_NAME.c_str(), 1);
    char* cjson = cJSON_PrintUnformatted(metaJson);
    std::string descJsonStr(cjson);
    ParseChannelDescription(descJsonStr, ownerName, peerDevId);
    EXPECT_EQ(false, descJsonStr.empty());
    cJSON_free(cjson);
    cJSON_Delete(metaJson);


    cJSON *metaJson1 = cJSON_CreateObject();
    cJSON_AddStringToObject(metaJson1, KEY_OWNER_NAME.c_str(), "owner_name_test");
    cJSON_AddNumberToObject(metaJson1, KEY_PEER_DEVID.c_str(), 1);
    char* cjson1 = cJSON_PrintUnformatted(metaJson1);
    std::string descJsonStr1(cjson1);
    ParseChannelDescription(descJsonStr1, ownerName, peerDevId);
    EXPECT_EQ(false, descJsonStr1.empty());
    cJSON_free(cjson1);
    cJSON_Delete(metaJson1);

    cJSON *metaJson2 = cJSON_CreateObject();
    cJSON_AddStringToObject(metaJson2, KEY_OWNER_NAME.c_str(), "owner_name_test");
    cJSON_AddStringToObject(metaJson2, KEY_PEER_DEVID.c_str(), "peer_devid_test");
    char* cjson2 = cJSON_PrintUnformatted(metaJson2);
    std::string descJsonStr2(cjson2);
    ParseChannelDescription(descJsonStr2, ownerName, peerDevId);
    EXPECT_EQ(false, descJsonStr2.empty());
    cJSON_free(cjson2);
    cJSON_Delete(metaJson2);
}

HWTEST_F(DaudioInputTest, HiSBuffer2TransBuffer_001, TestSize.Level0)
{
    std::shared_ptr<AVBuffer> hisBuffer = nullptr;
    auto ret = HiSBuffer2TransBuffer(hisBuffer);
    EXPECT_EQ(nullptr, ret);

    hisBuffer = std::make_shared<AVBuffer>();
    ret = HiSBuffer2TransBuffer(hisBuffer);
    EXPECT_EQ(nullptr, ret);
}

HWTEST_F(DaudioInputTest, CastEventType_001, TestSize.Level0)
{
    Plugin::PluginEventType type = Plugin::PluginEventType::EVENT_CHANNEL_OPENED;
    bool isAbnormal = true;
    auto ret = CastEventType(type, isAbnormal);
    EXPECT_EQ(EventType::EVENT_START_SUCCESS, ret);

    type = Plugin::PluginEventType::EVENT_CHANNEL_OPEN_FAIL;
    ret = CastEventType(type, isAbnormal);
    EXPECT_EQ(EventType::EVENT_START_FAIL, ret);

    type = Plugin::PluginEventType::EVENT_CHANNEL_CLOSED;
    ret = CastEventType(type, isAbnormal);
    EXPECT_EQ(EventType::EVENT_ENGINE_ERROR, ret);

    std::string fileName = "";
    uint8_t *buffer = nullptr;
    int32_t bufSize = 1;
    DumpBufferToFile(fileName, buffer, bufSize);

    fileName = "fileName_test";
    DumpBufferToFile(fileName, buffer, bufSize);

    unsigned char* adtsHeader = new unsigned char[1024];
    uint32_t packetLen = 1;
    uint32_t profile = 2;
    uint32_t sampleRate = 1;
    uint32_t channels = 1;
    GenerateAdtsHeader(adtsHeader, packetLen, profile, sampleRate, channels);

    profile = 0;
    GenerateAdtsHeader(adtsHeader, packetLen, profile, sampleRate, channels);
    delete [] adtsHeader;
    adtsHeader = nullptr;
}

HWTEST_F(DaudioInputTest, GetBufferData_001, TestSize.Level0)
{
    std::shared_ptr<AVTransBuffer> transBuffer = std::make_shared<AVTransBuffer>();
    uint32_t index = 10;
    auto ret = transBuffer->GetBufferData(index);
    EXPECT_EQ(nullptr, ret);
}

HWTEST_F(DaudioInputTest, MarshalMessage_001, TestSize.Level0)
{
    auto avMessage = std::make_shared<AVTransMessage>((uint32_t)AVTransTag::START_AV_SYNC,
        "content_test", "dstDevId_test");
    auto ret = avMessage->MarshalMessage();
    EXPECT_EQ(false, ret.empty());
}

HWTEST_F(DaudioInputTest, UnmarshalMessage_001, TestSize.Level0)
{
    auto avMessage = std::make_shared<AVTransMessage>();
    std::string peerDevId = "peerDevId_test";
    cJSON *msgJson = cJSON_CreateObject();
    cJSON_AddStringToObject(msgJson, KEY_TYPE.c_str(), "type_test");
    cJSON_AddNumberToObject(msgJson, KEY_CONTENT.c_str(), 1);
    char* cjson = cJSON_PrintUnformatted(msgJson);
    std::string jsonStr(cjson);
    auto ret = avMessage->UnmarshalMessage(jsonStr, peerDevId);
    EXPECT_EQ(false, ret);
    cJSON_free(cjson);
    cJSON_Delete(msgJson);

    cJSON *msgJson1 = cJSON_CreateObject();
    cJSON_AddNumberToObject(msgJson1, KEY_TYPE.c_str(), (uint32_t)AVTransTag::START_AV_SYNC);
    cJSON_AddNumberToObject(msgJson1, KEY_CONTENT.c_str(), 1);
    char* cjson1 = cJSON_PrintUnformatted(msgJson1);
    std::string jsonStr1(cjson1);
    ret = avMessage->UnmarshalMessage(jsonStr1, peerDevId);
    EXPECT_EQ(false, ret);
    cJSON_free(cjson1);
    cJSON_Delete(msgJson1);

    cJSON *msgJson2 = cJSON_CreateObject();
    cJSON_AddStringToObject(msgJson2, KEY_TYPE.c_str(), "type_test");
    cJSON_AddStringToObject(msgJson2, KEY_CONTENT.c_str(), "content_test");
    char* cjson2 = cJSON_PrintUnformatted(msgJson2);
    std::string jsonStr2(cjson2);
    ret = avMessage->UnmarshalMessage(jsonStr2, peerDevId);
    EXPECT_EQ(false, ret);
    cJSON_free(cjson2);
    cJSON_Delete(msgJson2);

    cJSON *msgJson3 = cJSON_CreateObject();
    cJSON_AddNumberToObject(msgJson3, KEY_TYPE.c_str(), (uint32_t)AVTransTag::START_AV_SYNC);
    cJSON_AddStringToObject(msgJson3, KEY_CONTENT.c_str(), "content_test");
    char* cjson3 = cJSON_PrintUnformatted(msgJson3);
    std::string jsonStr3(cjson3);
    ret = avMessage->UnmarshalMessage(jsonStr3, peerDevId);
    EXPECT_EQ(true, ret);
    cJSON_free(cjson3);
    cJSON_Delete(msgJson3);
}

HWTEST_F(DaudioInputTest, CreateChannelServer_001, TestSize.Level0)
{
    std::string pkgName = "pkgName_test";
    std::string sessName = "sessName_avtrans.data";
    int32_t socketId = 1;
    SoftbusChannelAdapter::GetInstance().serverMap_.insert(std::make_pair(pkgName + "_" + sessName, socketId));
    auto ret = SoftbusChannelAdapter::GetInstance().CreateChannelServer(pkgName, sessName);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);

    SoftbusChannelAdapter::GetInstance().serverMap_.clear();
    ret = SoftbusChannelAdapter::GetInstance().CreateChannelServer(pkgName, sessName);
    EXPECT_EQ(ERR_DH_AVT_SESSION_ERROR, ret);
}

HWTEST_F(DaudioInputTest, RemoveChannelServer_001, TestSize.Level0)
{
    SoftbusChannelAdapter::GetInstance().serverMap_.clear();
    SoftbusChannelAdapter::GetInstance().devId2SessIdMap_.clear();
    std::string pkgName = "pkgName_test";
    std::string sessName = "sessName_avtrans.data";
    int32_t socketId = 1;
    auto ret = SoftbusChannelAdapter::GetInstance().RemoveChannelServer(pkgName, sessName);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);

    SoftbusChannelAdapter::GetInstance().serverMap_.insert(std::make_pair(pkgName + "_" + sessName, socketId));
    SoftbusChannelAdapter::GetInstance().devId2SessIdMap_.insert(std::make_pair(sessName, socketId));
    ret = SoftbusChannelAdapter::GetInstance().RemoveChannelServer(pkgName, sessName);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

HWTEST_F(DaudioInputTest, OpenSoftbusChannel_001, TestSize.Level0)
{
    SoftbusChannelAdapter::GetInstance().devId2SessIdMap_.clear();
    std::string mySessName = "mySessName_test";
    std::string peerSessName = "peerSessName_test";
    std::string peerDevId = "peerDevId_test";
    int32_t socketId = 1;
    std::string key = mySessName + "_" + peerDevId;
    SoftbusChannelAdapter::GetInstance().devId2SessIdMap_.insert(std::make_pair(key, socketId));
    auto ret = SoftbusChannelAdapter::GetInstance().OpenSoftbusChannel(mySessName, peerSessName, peerDevId);
    EXPECT_EQ(ERR_DH_AVT_SESSION_HAS_OPENED, ret);

    mySessName = "mySessName_avtrans.data";
    ret = SoftbusChannelAdapter::GetInstance().OpenSoftbusChannel(mySessName, peerSessName, peerDevId);
    EXPECT_EQ(ERR_DH_AVT_SESSION_ERROR, ret);
}

HWTEST_F(DaudioInputTest, SendBytesData_001, TestSize.Level0)
{
    SoftbusChannelAdapter::GetInstance().devId2SessIdMap_.clear();
    std::string sessName = "sessName_test";
    std::string peerDevId = "peerDevId_test";
    std::string data = "data_test";
    int32_t socketId = 1;
    auto ret = SoftbusChannelAdapter::GetInstance().SendBytesData(sessName, peerDevId, data);
    EXPECT_EQ(ERR_DH_AVT_SEND_DATA_FAILED, ret);

    std::string key = sessName + "_" + peerDevId;
    SoftbusChannelAdapter::GetInstance().devId2SessIdMap_.insert(std::make_pair(key, socketId));
    ret = SoftbusChannelAdapter::GetInstance().SendBytesData(sessName, peerDevId, data);
    EXPECT_EQ(ERR_DH_AVT_SEND_DATA_FAILED, ret);
}

HWTEST_F(DaudioInputTest, GetSessionNameById_001, TestSize.Level0)
{
    SoftbusChannelAdapter::GetInstance().devId2SessIdMap_.clear();
    std::string sessName = "sessName_test";
    std::string peerDevId = "peerDevId_test";
    int32_t sessionId = 1;
    ShutdownReason reason = ShutdownReason::SHUTDOWN_REASON_UNKNOWN;
    uint32_t dataLen = 100;
    char data[10] = "data_test";
    SoftbusChannelAdapter::GetInstance().OnSoftbusChannelClosed(sessionId, reason);
    SoftbusChannelAdapter::GetInstance().OnSoftbusBytesReceived(sessionId, data, dataLen);
    auto ret = SoftbusChannelAdapter::GetInstance().GetSessionNameById(sessionId);
    EXPECT_EQ(EMPTY_STRING, ret);

    std::string key = sessName + "_" + peerDevId;
    SoftbusChannelAdapter::GetInstance().devId2SessIdMap_.insert(std::make_pair(key, sessionId));
    SoftbusChannelAdapter::GetInstance().OnSoftbusChannelClosed(sessionId, reason);
    SoftbusChannelAdapter::GetInstance().devId2SessIdMap_.insert(std::make_pair(key, sessionId));
    ret = SoftbusChannelAdapter::GetInstance().GetSessionNameById(sessionId);
    EXPECT_EQ(key, ret);
}

HWTEST_F(DaudioInputTest, GetPeerDevIdBySessId_001, TestSize.Level0)
{
    SoftbusChannelAdapter::GetInstance().devId2SessIdMap_.clear();
    std::string peerDevId = "peerDevIdtest";
    std::string sessName = "sessNametest";
    int32_t sessionId = 1;
    std::string key = sessName + peerDevId;
    SoftbusChannelAdapter::GetInstance().devId2SessIdMap_.insert(std::make_pair(key, sessionId));
    auto ret = SoftbusChannelAdapter::GetInstance().GetPeerDevIdBySessId(2);
    EXPECT_EQ(EMPTY_STRING, ret);

    ret = SoftbusChannelAdapter::GetInstance().GetPeerDevIdBySessId(sessionId);
    EXPECT_EQ(EMPTY_STRING, ret);

    key = sessName + "_" + peerDevId;
    SoftbusChannelAdapter::GetInstance().devId2SessIdMap_.insert(std::make_pair(key, sessionId));
    ret = SoftbusChannelAdapter::GetInstance().GetPeerDevIdBySessId(sessionId);
    EXPECT_EQ(peerDevId, ret);

    SoftbusChannelAdapter::GetInstance().devId2SessIdMap_.clear();
    peerDevId = "av.trans.special.device.id";
    key = sessName + "_" + peerDevId;
    SoftbusChannelAdapter::GetInstance().devId2SessIdMap_.insert(std::make_pair(key, sessionId));
    ret = SoftbusChannelAdapter::GetInstance().GetPeerDevIdBySessId(sessionId);
    EXPECT_EQ(EMPTY_STRING, ret);
}
} // namespace DistributedHardware
} // namespace OHOS