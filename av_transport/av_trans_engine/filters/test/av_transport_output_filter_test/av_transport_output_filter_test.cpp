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
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    int32_t key = -1;
    Any value = VideoBitStreamFormat::ANNEXB;
    ErrorCode ret = avOutputTest_->SetParameter(key, value);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, SetParameter_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    int32_t key = static_cast<int32_t>(Plugin::Tag::MIME);
    Any value = MEDIA_MIME_AUDIO_RAW;
    ErrorCode ret = avOutputTest_->SetParameter(key, value);
    EXPECT_EQ(ErrorCode::SUCCESS, ret);
}

HWTEST_F(AvTransportOutputFilterTest, SetParameter_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    int32_t key = static_cast<int32_t>(Plugin::Tag::MIME);
    Any value = MEDIA_MIME_AUDIO_RAW;
    avOutputTest_->plugin_ = PluginManager::Instance().CreateGenericPlugin<AvTransOutput,
        AvTransOutputPlugin>("name");
    ErrorCode ret = avOutputTest_->SetParameter(key, value);
    EXPECT_EQ(ErrorCode::SUCCESS, ret);
}

HWTEST_F(AvTransportOutputFilterTest, GetParameter_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    int32_t key = -1;
    Any value = VideoBitStreamFormat::ANNEXB;
    ErrorCode ret = avOutputTest_->GetParameter(key, value);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, GetParameter_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    int32_t key = static_cast<int32_t>(Plugin::Tag::MIME);
    Any value = VideoBitStreamFormat::ANNEXB;
    ErrorCode ret = avOutputTest_->GetParameter(key, value);
    EXPECT_EQ(ErrorCode::SUCCESS, ret);
}

HWTEST_F(AvTransportOutputFilterTest, Prepare_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    ErrorCode ret = avOutputTest_->Prepare();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_STATE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, Prepare_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    avOutputTest_->state_ = FilterState::INITIALIZED;
    ErrorCode ret = avOutputTest_->Prepare();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, Start_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    ErrorCode ret = avOutputTest_->Start();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_STATE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, Start_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    avOutputTest_->state_ = FilterState::READY;
    avOutputTest_->plugin_ = nullptr;
    ErrorCode ret = avOutputTest_->Start();
    EXPECT_EQ(ErrorCode::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportOutputFilterTest, Stop_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    ErrorCode ret = avOutputTest_->Stop();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_STATE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, Stop_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    avOutputTest_->state_ = FilterState::RUNNING;
    avOutputTest_->plugin_ = nullptr;
    ErrorCode ret = avOutputTest_->Stop();
    EXPECT_EQ(ErrorCode::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportOutputFilterTest, FindPlugin_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    ErrorCode ret = avOutputTest_->FindPlugin();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, FindPlugin_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    int32_t key = static_cast<int32_t>(Plugin::Tag::MIME);
    Any value = 100;
    avOutputTest_->plugin_ = PluginManager::Instance().CreateGenericPlugin<AvTransOutput,
        AvTransOutputPlugin>("name");
    avOutputTest_->SetParameter(key, value);
    ErrorCode ret = avOutputTest_->FindPlugin();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, Negotiate_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    std::string inPort = "inPort_test";
    std::shared_ptr<const Plugin::Capability> upstreamCap;
    Plugin::Capability negotiatedCap;
    Plugin::Meta upstreamParams;
    Plugin::Meta downstreamParams;
    bool ret = avOutputTest_->Negotiate(inPort, upstreamCap, negotiatedCap, upstreamParams, downstreamParams);
    EXPECT_EQ(false, ret);
}

HWTEST_F(AvTransportOutputFilterTest, CreatePlugin_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    std::shared_ptr<PluginInfo> selectedInfo = nullptr;
    ErrorCode ret = avOutputTest_->CreatePlugin(selectedInfo);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, CreatePlugin_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    std::shared_ptr<PluginInfo> selectedInfo = PluginManager::Instance().GetPluginInfo(
        PluginType::GENERIC_PLUGIN, "");
    ErrorCode ret = avOutputTest_->CreatePlugin(selectedInfo);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, InitPlugin_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    avOutputTest_->plugin_ = nullptr;
    ErrorCode ret = avOutputTest_->InitPlugin();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, ConfigPlugin_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    ErrorCode ret = avOutputTest_->ConfigPlugin();
    EXPECT_EQ(ErrorCode::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportOutputFilterTest, PreparePlugin_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    ErrorCode ret = avOutputTest_->PreparePlugin();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_TYPE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, PushData_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    std::string inPort;
    AVBufferPtr buffer;
    int64_t offset = 0;
    ErrorCode ret = avOutputTest_->PushData(inPort, buffer, offset);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_TYPE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, SetPluginParams_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    ErrorCode ret = avOutputTest_->SetPluginParams();
    EXPECT_EQ(ErrorCode::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportOutputFilterTest, SetEventCallBack_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    ErrorCode ret = avOutputTest_->SetEventCallBack();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, SetEventCallBack_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    avOutputTest_->plugin_ = PluginManager::Instance().CreateGenericPlugin<AvTransOutput,
        AvTransOutputPlugin>("name");
    ErrorCode ret = avOutputTest_->SetEventCallBack();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, SetDataCallBack_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    std::shared_ptr<Plugin::Buffer> buffer = nullptr;
    avOutputTest_->OnDataCallback(buffer);

    ErrorCode ret = avOutputTest_->SetDataCallBack();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportOutputFilterTest, SetDataCallBack_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVOutputFilter> avOutputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, FILTERNAME);
    std::shared_ptr<Plugin::Buffer> buffer = nullptr;
    avOutputTest_->OnDataCallback(buffer);

    avOutputTest_->plugin_ = PluginManager::Instance().CreateGenericPlugin<AvTransOutput,
        AvTransOutputPlugin>("name");
    ErrorCode ret = avOutputTest_->SetDataCallBack();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}
} // namespace DistributedHardware
} // namespace OHOS