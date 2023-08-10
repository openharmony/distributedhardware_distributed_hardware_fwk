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

#include "dsoftbus_output_plugin_test.h"

#include "dsoftbus_output_plugin.h"

namespace OHOS {
namespace DistributedHardware {
const std::string PLUGINNAME = "dsoftbus_output_plugin";

void DsoftbusOutputPluginTest::SetUpTestCase(void) {}

void DsoftbusOutputPluginTest::TearDownTestCase(void) {}

void DsoftbusOutputPluginTest::SetUp(void) {}

void DsoftbusOutputPluginTest::TearDown(void) {}

HWTEST_F(DsoftbusOutputPluginTest, Reset_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DsoftbusOutputPlugin>(PLUGINNAME);
    Status ret = plugin->Reset();
    EXPECT_EQ(Status::OK, ret);
    plugin->bufferPopTask_ = std::make_shared<Media::OSAL::Task>("videoBufferQueuePopThread");
    ret = plugin->Reset();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DsoftbusOutputPluginTest, Prepare_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DsoftbusOutputPlugin>(PLUGINNAME);
    plugin->state_ = State::PREPARED;
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DsoftbusOutputPluginTest, Prepare_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusOutputPlugin>(PLUGINNAME);
    plugin->state_ = State::INITIALIZED;
    plugin->ownerName_ = "ohos_dhardware.dcamera";
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
}

HWTEST_F(DsoftbusOutputPluginTest, GetParameter_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusOutputPlugin>(PLUGINNAME);
    std::string value = "dsoftbus_output_plugin_test";
    plugin->SetParameter(Tag::MEDIA_DESCRIPTION, value);
    ValueType val;
    Status ret = plugin->GetParameter(Tag::MEDIA_DESCRIPTION, val);
    EXPECT_EQ(Status::OK, ret);

    plugin->paramsMap_.clear();
    ret = plugin->GetParameter(Tag::USER_FRAME_NUMBER, val);
    EXPECT_EQ(Status::ERROR_NOT_EXISTED, ret);
}

HWTEST_F(DsoftbusOutputPluginTest, Start_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusOutputPlugin>(PLUGINNAME);
    Status ret = plugin->Start();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DsoftbusOutputPluginTest, Start_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusOutputPlugin>(PLUGINNAME);
    plugin->state_ = State::PREPARED;
    Status ret = plugin->Start();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
}

HWTEST_F(DsoftbusOutputPluginTest, Stop_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusOutputPlugin>(PLUGINNAME);
    Status ret = plugin->Stop();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DsoftbusOutputPluginTest, PushData_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DsoftbusOutputPlugin>(PLUGINNAME);
    std::string inPort = "inPort_test";
    std::shared_ptr<Plugin::Buffer> buffer = nullptr;
    int64_t offset = 1;
    Status ret = plugin->PushData(inPort, buffer, offset);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

} // namespace DistributedHardware
} // namespace OHOS