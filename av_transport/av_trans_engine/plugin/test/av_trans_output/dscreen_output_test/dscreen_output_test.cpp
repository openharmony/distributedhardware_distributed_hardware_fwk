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

#include "dscreen_output_test.h"

#include "dscreen_output_plugin.h"

namespace OHOS {
namespace DistributedHardware {
const std::string PLUGINNAME = "dscreen_output";

void DscreenOutputTest::SetUpTestCase(void) {}

void DscreenOutputTest::TearDownTestCase(void) {}

void DscreenOutputTest::SetUp(void) {}

void DscreenOutputTest::TearDown(void) {}

class DscreenOutputPluginCallback : public Callback {
public:
    void OnEvent(const PluginEvent &event)
    {
        (void)event;
    }
};

HWTEST_F(DscreenOutputTest, Reset_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    Status ret = plugin->Reset();
    EXPECT_EQ(Status::OK, ret);
    plugin->InitOutputController();
    ret = plugin->Reset();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DscreenOutputTest, Prepare_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->state_ = State::PREPARED;
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(DscreenOutputTest, Prepare_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->state_ = State::INITIALIZED;
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(DscreenOutputTest, Prepare_003, TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->InitOutputController();
    plugin->state_ = State::INITIALIZED;
    Status ret = plugin->Prepare();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DscreenOutputTest, SetParameter_001, TestSize.Level0)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    std::string value = "dscreen_output_test";
    Status ret = plugin->SetParameter(Tag::USER_FRAME_NUMBER, value);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    plugin->InitOutputController();
    ret = plugin->SetParameter(Tag::USER_FRAME_NUMBER, value);
    EXPECT_EQ(Status::OK, ret);
}


HWTEST_F(DscreenOutputTest, GetParameter_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    std::string value = "dscreen_output_test";
    ValueType val;
    Status ret = plugin->GetParameter(Tag::USER_FRAME_NUMBER, val);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(DscreenOutputTest, GetParameter_002, TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->InitOutputController();
    ValueType val;
    Status ret = plugin->GetParameter(Tag::USER_FRAME_NUMBER, val);
    EXPECT_EQ(Status::ERROR_NOT_EXISTED, ret);
}

HWTEST_F(DscreenOutputTest, GetParameter_003, TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->InitOutputController();
    std::string value = "dscreen_output_test";
    plugin->SetParameter(Tag::USER_FRAME_NUMBER, value);
    plugin->InitOutputController();
    ValueType val;
    Status ret = plugin->GetParameter(Tag::USER_FRAME_NUMBER, val);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DscreenOutputTest, Start_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->state_ = State::INITIALIZED;
    Status ret = plugin->Start();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);

    plugin->state_ = State::PREPARED;
    ret = plugin->Start();
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    plugin->InitOutputController();
    ret = plugin->Start();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DscreenOutputTest, Stop_001, TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->state_ = State::INITIALIZED;
    Status ret = plugin->Stop();
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);

    plugin->state_ = State::RUNNING;
    ret = plugin->Stop();
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    plugin->InitOutputController();
    ret = plugin->Stop();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DscreenOutputTest, PushData_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    std::string inPort = "inPort_test";
    std::shared_ptr<Plugin::Buffer> buffer = nullptr;
    int64_t offset = 1;
    Status ret = plugin->PushData(inPort, buffer, offset);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    plugin->InitOutputController();
    std::shared_ptr<Plugin::Buffer> data = std::make_shared<AVBuffer>();
    plugin->controller_->PushData(data);

    plugin->controller_->SetControlStatus(DScreenOutputController::ControlStatus::START);
    plugin->controller_->PushData(data);

    plugin->controller_->SetControlMode(DScreenOutputController::ControlMode::SYNC);
    plugin->controller_->PushData(data);
}

HWTEST_F(DscreenOutputTest, StartControl_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->InitOutputController();
    OutputController::ControlStatus ret = plugin->controller_->StartControl();
    EXPECT_EQ(OutputController::ControlStatus::START, ret);

    plugin->controller_->SetControlStatus(OutputController::ControlStatus::START);
    ret = plugin->controller_->StartControl();
    EXPECT_EQ(OutputController::ControlStatus::STARTED, ret);
}

HWTEST_F(DscreenOutputTest, StopControl_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->InitOutputController();
    OutputController::ControlStatus ret = plugin->controller_->StopControl();
    EXPECT_EQ(OutputController::ControlStatus::STOP, ret);

    plugin->controller_->SetControlStatus(OutputController::ControlStatus::STOP);
    ret = plugin->controller_->StopControl();
    EXPECT_EQ(OutputController::ControlStatus::STOPPED, ret);
}

HWTEST_F(DscreenOutputTest, ReleaseControl_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->InitOutputController();

    OutputController::ControlStatus ret = plugin->controller_->ReleaseControl();
    EXPECT_EQ(OutputController::ControlStatus::RELEASED, ret);
}

HWTEST_F(DscreenOutputTest, SetParameter_002, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->InitOutputController();
    std::string value = "dscreen_output_test";
    Status ret = plugin->controller_->SetParameter(Tag::USER_AV_SYNC_GROUP_INFO, value);
    EXPECT_EQ(Status::OK, ret);

    ret = plugin->controller_->SetParameter(Tag::USER_SHARED_MEMORY_FD, value);
    EXPECT_EQ(Status::OK, ret);

    ret = plugin->controller_->SetParameter(Tag::USER_TIME_SYNC_RESULT, value);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DscreenOutputTest, ControlOutput_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->InitOutputController();
    std::shared_ptr<Plugin::Buffer> data = std::make_shared<AVBuffer>();
    plugin->controller_->SetAllowControlState(false);
    int32_t ret = plugin->controller_->ControlOutput(data);
    EXPECT_EQ(OUTPUT_FRAME, ret);

    plugin->controller_->SetAllowControlState(true);
    data->pts = 100;
    ret = plugin->controller_->ControlOutput(data);
    EXPECT_EQ(OUTPUT_FRAME, ret);

    plugin->controller_->CheckSyncInfo(data);
    plugin->controller_->PrepareControl();
    plugin->controller_->SetControlMode(OutputController::ControlMode::SYNC);
    plugin->controller_->CalProcessTime(data);
}

HWTEST_F(DscreenOutputTest, CheckIsClockInvalid_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->InitOutputController();
    std::shared_ptr<Plugin::Buffer> data;
    bool ret = plugin->controller_->CheckIsClockInvalid(data);
    EXPECT_EQ(false, ret);

    plugin->controller_->SetControlMode(OutputController::ControlMode::SYNC);
    ret = plugin->controller_->CheckIsClockInvalid(data);
    EXPECT_EQ(true, ret);
}

HWTEST_F(DscreenOutputTest, AcquireSyncClockTime_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->InitOutputController();
    std::shared_ptr<Plugin::Buffer> data;
    int32_t ret = plugin->controller_->AcquireSyncClockTime(data);
    EXPECT_EQ(ERR_DH_AVT_INVALID_PARAM, ret);
}

HWTEST_F(DscreenOutputTest, WaitRereadClockFailed_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->InitOutputController();
    std::shared_ptr<Plugin::Buffer> data;
    bool ret = plugin->controller_->WaitRereadClockFailed(data);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DscreenOutputTest, CheckIsProcessInDynamicBalance_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->InitOutputController();
    std::shared_ptr<Plugin::Buffer> data;
    plugin->controller_->SetControlMode(OutputController::ControlMode::SYNC);
    bool ret = plugin->controller_->CheckIsProcessInDynamicBalance(data);
    EXPECT_EQ(true, ret);

    plugin->controller_->SetControlMode(OutputController::ControlMode::SMOOTH);
    plugin->controller_->SetProcessDynamicBalanceState(false);
    ret = plugin->controller_->CheckIsProcessInDynamicBalance(data);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DscreenOutputTest, CheckIsProcessInDynamicBalanceOnce_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->InitOutputController();
    std::shared_ptr<Plugin::Buffer> data;
    bool ret = plugin->controller_->CheckIsProcessInDynamicBalanceOnce(data);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DscreenOutputTest, PostOutputEvent_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->InitOutputController();
    std::shared_ptr<Plugin::Buffer> data;
    int32_t ret = plugin->controller_->PostOutputEvent(data);
    EXPECT_EQ(HANDLE_FAILED, ret);

    data = std::make_shared<AVBuffer>();
    plugin->controller_->SyncClock(data);
}

HWTEST_F(DscreenOutputTest, NotifyOutput_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    plugin->InitOutputController();
    std::shared_ptr<Plugin::Buffer> data;
    plugin->controller_->UnregisterListener();
    int32_t ret = plugin->controller_->NotifyOutput(data);
    EXPECT_EQ(NOTIFY_FAILED, ret);

    int32_t result = 0;
    plugin->controller_->HandleControlResult(data, result);

    result = 1;
    plugin->controller_->HandleControlResult(data, result);

    result = 2;
    plugin->controller_->HandleControlResult(data, result);

    result = 3;
    plugin->controller_->HandleControlResult(data, result);
}

HWTEST_F(DscreenOutputTest, SetCallback_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    Status ret = plugin->SetCallback(nullptr);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    DscreenOutputPluginCallback cb {};
    ret = plugin->SetCallback(&cb);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(DscreenOutputTest, SetDataCallback_001, testing::ext::TestSize.Level1)
{
    auto plugin = std::make_shared<DscreenOutputPlugin>(PLUGINNAME);
    AVDataCallback cbk;
    Status ret = plugin->SetDataCallback(cbk);
    EXPECT_EQ(Status::OK, ret);
}
} // namespace DistributedHardware
} // namespace OHOS