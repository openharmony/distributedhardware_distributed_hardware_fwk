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

#include "av_transport_output_filter_test.h"

#include "av_trans_constants.h"
#include "pipeline/filters/common/plugin_utils.h"
#include "pipeline/factory/filter_factory.h"
#include "plugin/common/plugin_attr_desc.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
const std::string FILTERNAME = "avoutput";

void AvTransportOutputFilterTest::SetUp()
{
    avOutputTest_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
}

void AvTransportOutputFilterTest::TearDown()
{
}

void AvTransportOutputFilterTest::SetUpTestCase()
{
}

void AvTransportOutputFilterTest::TearDownTestCase()
{
}

HWTEST_F(AvTransportOutputFilterTest, SetParameter_001, testing::ext::TestSize.Level1)
{
    int32_t key = -1;
    Any value = VideoBitStreamFormat::ANNEXB;
    ErrorCode ret = avOutputTest_->SetParameter(key, value);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, SetParameter_002, testing::ext::TestSize.Level1)
{
    int32_t key = static_cast<int32_t>(Plugin::Tag::MIME);
    Any value = MEDIA_MIME_AUDIO_RAW;
    ErrorCode ret = avOutputTest_->SetParameter(key, value);
    EXPECT_EQ(ErrorCode::SUCCESS, ret);

    avOutputTest_->plugin_ =
        PluginManager::Instance().CreateGenericPlugin<AvTransOutput, AvTransOutputPlugin>("AVTransDaudioOutputPlugin");
    ret = avOutputTest_->SetParameter(key, value);
    EXPECT_EQ(ErrorCode::SUCCESS, ret);
}

HWTEST_F(AvTransportOutputFilterTest, GetParameter_001, testing::ext::TestSize.Level1)
{
    int32_t key = -1;
    Any value = VideoBitStreamFormat::ANNEXB;
    ErrorCode ret = avOutputTest_->GetParameter(key, value);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, GetParameter_002, testing::ext::TestSize.Level1)
{
    int32_t key = static_cast<int32_t>(Plugin::Tag::MIME);
    Any value = VideoBitStreamFormat::ANNEXB;
    ErrorCode ret = avOutputTest_->GetParameter(key, value);
    EXPECT_EQ(ErrorCode::SUCCESS, ret);
}

HWTEST_F(AvTransportOutputFilterTest, Prepare_001, testing::ext::TestSize.Level1)
{
    ErrorCode ret = avOutputTest_->Prepare();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_STATE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, Prepare_002, testing::ext::TestSize.Level1)
{
    avOutputTest_->state_ = FilterState::INITIALIZED;
    ErrorCode ret = avOutputTest_->Prepare();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, Start_001, testing::ext::TestSize.Level1)
{
    ErrorCode ret = avOutputTest_->Start();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_STATE, ret);

    avOutputTest_->state_ = FilterState::READY;
    ret = avOutputTest_->Start();
    EXPECT_EQ(ErrorCode::ERROR_NULL_POINTER, ret);

    avOutputTest_->state_ = FilterState::PAUSED;
    ret = avOutputTest_->Start();
    EXPECT_EQ(ErrorCode::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportOutputFilterTest, Start_002, testing::ext::TestSize.Level1)
{
    avOutputTest_->plugin_ =
        PluginManager::Instance().CreateGenericPlugin<AvTransOutput, AvTransOutputPlugin>("AVTransDaudioOutputPlugin");
    avOutputTest_->state_ = FilterState::READY;
    ErrorCode ret = avOutputTest_->Start();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_OPERATION, ret);
}

HWTEST_F(AvTransportOutputFilterTest, Stop_001, testing::ext::TestSize.Level1)
{
    avOutputTest_->plugin_ = nullptr;
    ErrorCode ret = avOutputTest_->Stop();
    EXPECT_EQ(ErrorCode::ERROR_NULL_POINTER, ret);

    avOutputTest_->plugin_ =
        PluginManager::Instance().CreateGenericPlugin<AvTransOutput, AvTransOutputPlugin>("AVTransDaudioOutputPlugin");
    ret = avOutputTest_->Stop();
    EXPECT_EQ(ErrorCode::SUCCESS, ret);
}

HWTEST_F(AvTransportOutputFilterTest, Stop_002, testing::ext::TestSize.Level1)
{
    avOutputTest_->state_ = FilterState::RUNNING;
    avOutputTest_->plugin_ =
        PluginManager::Instance().CreateGenericPlugin<AvTransOutput, AvTransOutputPlugin>("AVTransDaudioOutputPlugin");
    ErrorCode ret = avOutputTest_->Stop();
    EXPECT_EQ(ErrorCode::SUCCESS, ret);
}

HWTEST_F(AvTransportOutputFilterTest, FindPlugin_001, testing::ext::TestSize.Level1)
{
    ErrorCode ret = avOutputTest_->FindPlugin();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, FindPlugin_002, testing::ext::TestSize.Level1)
{
    int32_t key = static_cast<int32_t>(Plugin::Tag::MIME);
    Any value = 100;
    avOutputTest_->plugin_ =
        PluginManager::Instance().CreateGenericPlugin<AvTransOutput, AvTransOutputPlugin>("name");
    avOutputTest_->SetParameter(key, value);
    ErrorCode ret = avOutputTest_->FindPlugin();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);

    value = MEDIA_MIME_VIDEO_RAW;
    avOutputTest_->SetParameter(key, value);
    ret = avOutputTest_->FindPlugin();
    EXPECT_NE(ErrorCode::ERROR_UNSUPPORTED_FORMAT, ret);
}

HWTEST_F(AvTransportOutputFilterTest, Negotiate_001, testing::ext::TestSize.Level1)
{
    std::string inPort = "inPort_test";
    std::shared_ptr<const Plugin::Capability> upstreamCap;
    Plugin::Capability negotiatedCap;
    Plugin::Meta upstreamParams;
    Plugin::Meta downstreamParams;
    bool ret = avOutputTest_->Negotiate(inPort, upstreamCap, negotiatedCap, upstreamParams, downstreamParams);
    EXPECT_EQ(false, ret);

    avOutputTest_->pluginInfo_ = std::make_shared<Plugin::PluginInfo>();
    avOutputTest_->pluginInfo_->inCaps.push_back(negotiatedCap);
    ret = avOutputTest_->Negotiate(inPort, upstreamCap, negotiatedCap, upstreamParams, downstreamParams);
    EXPECT_EQ(true, ret);
}

HWTEST_F(AvTransportOutputFilterTest, CreatePlugin_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<PluginInfo> selectedInfo = nullptr;
    ErrorCode ret = avOutputTest_->CreatePlugin(selectedInfo);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);

    selectedInfo = std::make_shared<PluginInfo>();
    selectedInfo->name = "";
    ret = avOutputTest_->CreatePlugin(selectedInfo);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);

    selectedInfo->name = "name_test";
    avOutputTest_->plugin_ = nullptr;
    avOutputTest_->pluginInfo_ = nullptr;
    ret = avOutputTest_->CreatePlugin(selectedInfo);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);

    avOutputTest_->plugin_ =
        PluginManager::Instance().CreateGenericPlugin<AvTransOutput, AvTransOutputPlugin>("AVTransDaudioOutputPlugin");
    ret = avOutputTest_->CreatePlugin(selectedInfo);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);

    avOutputTest_->plugin_ = nullptr;
    avOutputTest_->pluginInfo_ = std::make_shared<Plugin::PluginInfo>();
    ret = avOutputTest_->CreatePlugin(selectedInfo);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);

    avOutputTest_->plugin_ =
        PluginManager::Instance().CreateGenericPlugin<AvTransOutput, AvTransOutputPlugin>("AVTransDaudioOutputPlugin");
    avOutputTest_->pluginInfo_ = std::make_shared<Plugin::PluginInfo>();
    avOutputTest_->pluginInfo_->name = "name";
    ret = avOutputTest_->CreatePlugin(selectedInfo);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);

    avOutputTest_->pluginInfo_->name = "name_test";
    ret = avOutputTest_->CreatePlugin(selectedInfo);
    EXPECT_NE(ErrorCode::SUCCESS, ret);
}

HWTEST_F(AvTransportOutputFilterTest, CreatePlugin_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<PluginInfo> selectedInfo = PluginManager::Instance().GetPluginInfo(
        PluginType::GENERIC_PLUGIN, "");
    ErrorCode ret = avOutputTest_->CreatePlugin(selectedInfo);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, InitPlugin_001, testing::ext::TestSize.Level1)
{
    avOutputTest_->plugin_ = nullptr;
    ErrorCode ret = avOutputTest_->InitPlugin();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);

    avOutputTest_->plugin_ =
        PluginManager::Instance().CreateGenericPlugin<AvTransOutput, AvTransOutputPlugin>("AVTransDaudioOutputPlugin");
    ret = avOutputTest_->InitPlugin();
    EXPECT_EQ(ErrorCode::SUCCESS, ret);
}

HWTEST_F(AvTransportOutputFilterTest, ConfigPlugin_001, testing::ext::TestSize.Level1)
{
    ErrorCode ret = avOutputTest_->ConfigPlugin();
    EXPECT_EQ(ErrorCode::ERROR_NULL_POINTER, ret);

    avOutputTest_->plugin_ =
        PluginManager::Instance().CreateGenericPlugin<AvTransOutput, AvTransOutputPlugin>("AVTransDaudioOutputPlugin");
    ret = avOutputTest_->ConfigPlugin();
    EXPECT_EQ(ErrorCode::SUCCESS, ret);
}

HWTEST_F(AvTransportOutputFilterTest, PreparePlugin_001, testing::ext::TestSize.Level1)
{
    avOutputTest_->plugin_ = nullptr;
    ErrorCode ret = avOutputTest_->PreparePlugin();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_TYPE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, PreparePlugin_002, testing::ext::TestSize.Level1)
{
    ErrorCode ret = avOutputTest_->PreparePlugin();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_TYPE, ret);

    avOutputTest_->plugin_ =
        PluginManager::Instance().CreateGenericPlugin<AvTransOutput, AvTransOutputPlugin>("AVTransDaudioOutputPlugin");
    ret = avOutputTest_->PreparePlugin();
    EXPECT_NE(ErrorCode::SUCCESS, ret);
}

HWTEST_F(AvTransportOutputFilterTest, PushData_001, testing::ext::TestSize.Level1)
{
    std::string inPort;
    AVBufferPtr buffer;
    int64_t offset = 0;
    ErrorCode ret = avOutputTest_->PushData(inPort, buffer, offset);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_TYPE, ret);

    buffer = std::make_shared<AVBuffer>();
    inPort = "inPort_test";
    ret = avOutputTest_->PushData(inPort, buffer, offset);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_TYPE, ret);

    buffer = nullptr;
    avOutputTest_->plugin_ =
        PluginManager::Instance().CreateGenericPlugin<AvTransOutput, AvTransOutputPlugin>("AVTransDaudioOutputPlugin");
    ret = avOutputTest_->PushData(inPort, buffer, offset);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_TYPE, ret);

    buffer = std::make_shared<AVBuffer>();
    ret = avOutputTest_->PushData(inPort, buffer, offset);
    EXPECT_EQ(ErrorCode::SUCCESS, ret);
}

HWTEST_F(AvTransportOutputFilterTest, SetPluginParams_001, testing::ext::TestSize.Level1)
{
    avOutputTest_->plugin_ = nullptr;
    ErrorCode ret = avOutputTest_->SetPluginParams();
    EXPECT_EQ(ErrorCode::ERROR_NULL_POINTER, ret);

    avOutputTest_->plugin_ =
        PluginManager::Instance().CreateGenericPlugin<AvTransOutput, AvTransOutputPlugin>("AVTransDaudioOutputPlugin");
    ret = avOutputTest_->SetPluginParams();
    EXPECT_EQ(ErrorCode::SUCCESS, ret);

    int32_t key = static_cast<int32_t>(Plugin::Tag::MEDIA_DESCRIPTION);
    uint32_t value = 100;
    avOutputTest_->SetParameter(key, value);
    ret = avOutputTest_->SetPluginParams();
    EXPECT_EQ(ErrorCode::SUCCESS, ret);

    key = static_cast<int32_t>(Plugin::Tag::MEDIA_DESCRIPTION);
    avOutputTest_->SetParameter(key, value);
    ret = avOutputTest_->SetPluginParams();
    EXPECT_EQ(ErrorCode::SUCCESS, ret);

    key = static_cast<int32_t>(Plugin::Tag::AUDIO_CHANNELS);
    avOutputTest_->SetParameter(key, value);
    ret = avOutputTest_->SetPluginParams();
    EXPECT_EQ(ErrorCode::SUCCESS, ret);

    key = static_cast<int32_t>(Plugin::Tag::AUDIO_SAMPLE_RATE);
    avOutputTest_->SetParameter(key, value);
    ret = avOutputTest_->SetPluginParams();
    EXPECT_EQ(ErrorCode::SUCCESS, ret);

    key = static_cast<int32_t>(Plugin::Tag::AUDIO_CHANNEL_LAYOUT);
    avOutputTest_->SetParameter(key, value);
    ret = avOutputTest_->SetPluginParams();
    EXPECT_EQ(ErrorCode::SUCCESS, ret);

    key = static_cast<int32_t>(Plugin::Tag::SECTION_USER_SPECIFIC_START);
    avOutputTest_->SetParameter(key, value);
    ret = avOutputTest_->SetPluginParams();
    EXPECT_EQ(ErrorCode::SUCCESS, ret);

    key = static_cast<int32_t>(Plugin::Tag::SECTION_VIDEO_SPECIFIC_START);
    avOutputTest_->SetParameter(key, value);
    ret = avOutputTest_->SetPluginParams();
    EXPECT_EQ(ErrorCode::SUCCESS, ret);
}

HWTEST_F(AvTransportOutputFilterTest, SetEventCallBack_001, testing::ext::TestSize.Level1)
{
    avOutputTest_->plugin_ =
        PluginManager::Instance().CreateGenericPlugin<AvTransOutput, AvTransOutputPlugin>("AVTransDaudioOutputPlugin");
    ErrorCode ret = avOutputTest_->SetEventCallBack();
    EXPECT_EQ(ErrorCode::SUCCESS, ret);

    avOutputTest_->plugin_ = nullptr;
    ret = avOutputTest_->SetEventCallBack();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, SetDataCallBack_001, testing::ext::TestSize.Level1)
{
    avOutputTest_->plugin_ =
        PluginManager::Instance().CreateGenericPlugin<AvTransOutput, AvTransOutputPlugin>("AVTransDaudioOutputPlugin");
    std::shared_ptr<Plugin::Buffer> buffer = nullptr;
    avOutputTest_->OnDataCallback(buffer);

    buffer = std::make_shared<AVBuffer>();
    avOutputTest_->OnDataCallback(buffer);

    ErrorCode ret = avOutputTest_->SetDataCallBack();
    EXPECT_EQ(ErrorCode::SUCCESS, ret);

    avOutputTest_->plugin_ = nullptr;
    ret = avOutputTest_->SetDataCallBack();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}
} // namespace DistributedHardware
} // namespace OHOS