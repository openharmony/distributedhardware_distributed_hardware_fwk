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

#ifndef AV_TRANSPORT_INPUT_FILTER_TEST_H
#define AV_TRANSPORT_INPUT_FILTER_TEST_H

#include <gtest/gtest.h>

#include "av_receiver_engine.h"
#include "av_trans_buffer.h"
#include "av_trans_errno.h"
#include "av_trans_constants.h"
#include "av_trans_log.h"
#include "av_trans_message.h"
#include "av_trans_types.h"
#include "av_trans_utils.h"
#include "i_av_receiver_engine.h"
#include "i_av_receiver_engine_callback.h"
#include "softbus_channel_adapter.h"
#include "distributed_hardware_fwk_kit.h"
#include "av_trans_control_center_callback.h"
#include "av_transport_input_filter.h"
#include "av_transport_output_filter.h"

// follwing head files depends on histreamer
#include "error_code.h"
#include "event.h"
#include "pipeline/core/filter.h"
#include "pipeline_core.h"
#include "audio_encoder_filter.h"
#include "video_encoder_filter.h"

namespace OHOS {
namespace DistributedHardware {

using namespace OHOS::Media;
using namespace OHOS::Media::Plugin;
using namespace OHOS::Media::Pipeline;
using AVBuffer = OHOS::Media::Plugin::Buffer;

class AvReceiverEngineTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

class ReceiverEngineCallback : public IAVReceiverEngineCallback {
public:
    ReceiverEngineCallback() = default;
    ~ReceiverEngineCallback() override = default;
    int32_t OnReceiverEvent(const AVTransEvent &event) override
    {
        (void) event;
        return DH_AVT_SUCCESS;
    }
    int32_t OnMessageReceived(const std::shared_ptr<AVTransMessage> &message) override
    {
        (void) message;
        return DH_AVT_SUCCESS;
    }
    int32_t OnDataAvailable(const std::shared_ptr<AVTransBuffer> &buffer) override
    {
        (void) buffer;
        return DH_AVT_SUCCESS;
    }
};
} // namespace DistributedHardware
} // namespace OHOS

#endif // AV_TRANSPORT_INPUT_FILTER_TEST_H