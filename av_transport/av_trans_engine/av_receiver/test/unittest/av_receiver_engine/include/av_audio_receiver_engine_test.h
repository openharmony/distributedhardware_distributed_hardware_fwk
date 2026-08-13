/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef AV_AUDIO_RECEIVER_ENGINE_TEST_H
#define AV_AUDIO_RECEIVER_ENGINE_TEST_H

#include <gtest/gtest.h>

#include "av_audio_receiver_engine.h"
#include "pipeline_event.h"

namespace OHOS {
namespace DistributedHardware {

using namespace OHOS::Media;

class AvAudioReceiverEngineTest : public testing::Test {
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

#endif // AV_AUDIO_RECEIVER_ENGINE_TEST_H