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

#include "av_transport_input_filter_test.h"

#include "av_trans_constants.h"
#include "pipeline/filters/common/plugin_utils.h"
#include "pipeline/factory/filter_factory.h"
#include "plugin/common/plugin_attr_desc.h"
#include "pipeline/core/filter_base.h"
#include "pipeline/core/filter.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
const std::string FILTERNAME = "avinput";

void AvTransportInputFilterTest::SetUp()
{
}

void AvTransportInputFilterTest::TearDown()
{
}

void AvTransportInputFilterTest::SetUpTestCase()
{
}

void AvTransportInputFilterTest::TearDownTestCase()
{
}

HWTEST_F(AvTransportInputFilterTest, SetParameter_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVInputFilter> avInputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, FILTERNAME);
    int32_t key = -1;
    Any value = VideoBitStreamFormat::ANNEXB;
    ErrorCode ret = avInputTest_->SetParameter(key, value);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportInputFilterTest, SetParameter_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVInputFilter> avInputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, FILTERNAME);
    int32_t key = static_cast<int32_t>(Plugin::Tag::MIME);
    Any value = MEDIA_MIME_VIDEO_H264;
    ErrorCode ret = avInputTest_->SetParameter(key, value);
    EXPECT_EQ(ErrorCode::SUCCESS, ret);
}

HWTEST_F(AvTransportInputFilterTest, GetParameter_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVInputFilter> avInputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, FILTERNAME);
    int32_t key = -1;
    Any value = VideoBitStreamFormat::ANNEXB;
    ErrorCode ret = avInputTest_->GetParameter(key, value);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportInputFilterTest, GetParameter_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVInputFilter> avInputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, FILTERNAME);
    int32_t key = static_cast<int32_t>(Plugin::Tag::MIME);
    Any value = VideoBitStreamFormat::ANNEXB;
    ErrorCode ret = avInputTest_->GetParameter(key, value);
    EXPECT_EQ(ErrorCode::SUCCESS, ret);
}

HWTEST_F(AvTransportInputFilterTest, Prepare_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVInputFilter> avInputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, FILTERNAME);
    ErrorCode ret = avInputTest_->Prepare();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_STATE, ret);
}

HWTEST_F(AvTransportInputFilterTest, Start_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVInputFilter> avInputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, FILTERNAME);
    ErrorCode ret = avInputTest_->Start();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_STATE, ret);
}

HWTEST_F(AvTransportInputFilterTest, Stop_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVInputFilter> avInputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, FILTERNAME);
    ErrorCode ret = avInputTest_->Stop();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_STATE, ret);
}

HWTEST_F(AvTransportInputFilterTest, Pause_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVInputFilter> avInputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, FILTERNAME);
    ErrorCode ret = avInputTest_->Pause();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_STATE, ret);
}

HWTEST_F(AvTransportInputFilterTest, CreatePlugin_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVInputFilter> avInputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, FILTERNAME);
    std::shared_ptr<PluginInfo> selectedInfo = nullptr;
    ErrorCode ret = avInputTest_->CreatePlugin(selectedInfo);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportInputFilterTest, InitPlugin_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVInputFilter> avInputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, FILTERNAME);
    avInputTest_->plugin_ = nullptr;
    ErrorCode ret = avInputTest_->InitPlugin();
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportInputFilterTest, ConfigPlugin_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVInputFilter> avInputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, FILTERNAME);
    ErrorCode ret = avInputTest_->ConfigPlugin();
    EXPECT_EQ(ErrorCode::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportInputFilterTest, SetPluginParams_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVInputFilter> avInputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, FILTERNAME);
    ErrorCode ret = avInputTest_->SetPluginParams();
    EXPECT_EQ(ErrorCode::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportInputFilterTest, PreparePlugin_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVInputFilter> avInputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, FILTERNAME);
    ErrorCode ret = avInputTest_->PreparePlugin();
    EXPECT_EQ(ErrorCode::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportInputFilterTest, PushData_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVInputFilter> avInputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, FILTERNAME);
    std::string inPort;
    AVBufferPtr buffer;
    int64_t offset = 0;
    ErrorCode ret = avInputTest_->PushData(inPort, buffer, offset);
    EXPECT_EQ(ErrorCode::ERROR_INVALID_PARAMETER_VALUE, ret);
}

HWTEST_F(AvTransportInputFilterTest, SetEventCallBack_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVInputFilter> avInputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, FILTERNAME);
    avInputTest_->plugin_ = nullptr;
    ErrorCode ret = avInputTest_->SetEventCallBack();
    EXPECT_EQ(ErrorCode::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportInputFilterTest, SetDataCallBack_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AVInputFilter> avInputTest_ =
        FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, FILTERNAME);
    std::shared_ptr<Plugin::Buffer> buffer = nullptr;
    avInputTest_->OnDataCallback(buffer);
    avInputTest_->plugin_ = nullptr;
    ErrorCode ret = avInputTest_->SetDataCallBack();
    EXPECT_EQ(ErrorCode::ERROR_NULL_POINTER, ret);
}
} // namespace DistributedHardware
} // namespace OHOS