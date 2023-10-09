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

#ifndef OHOS_AV_TRANS_CONTROL_CENTER_CALLBACK_TEST_H
#define OHOS_AV_TRANS_CONTROL_CENTER_CALLBACK_TEST_H

#include <gtest/gtest.h>

#include <vector>

#define private public
#include "av_trans_control_center_callback.h"
#undef private
#include "av_trans_types.h"
#include "av_sync_utils.h"
#include "i_av_receiver_engine.h"
#include "i_av_receiver_engine_callback.h"
#include "i_av_sender_engine.h"
#include "i_av_sender_engine_callback.h"
#include "av_trans_message.h"
#include "av_trans_buffer.h"
namespace OHOS {
namespace DistributedHardware {
class AVTransControlCenterCallbackTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<AVTransControlCenterCallback> callBack_ = nullptr;
};
class ReceiverEngineTest : public IAVReceiverEngine {
public:
    ReceiverEngineTest() = default;
    ~ReceiverEngineTest() override = default;
    int32_t Initialize() override
    {
        return DH_AVT_SUCCESS;
    }

    int32_t Release() override
    {
        return DH_AVT_SUCCESS;
    }

    int32_t Start() override
    {
        return DH_AVT_SUCCESS;
    }

    int32_t Stop() override
    {
        return DH_AVT_SUCCESS;
    }

    int32_t SetParameter(AVTransTag tag, const std::string &value) override
    {
        (void) tag;
        (void) value;
        return DH_AVT_SUCCESS;
    }

    int32_t SendMessage(const std::shared_ptr<AVTransMessage> &message) override
    {
        (void) message;
        return DH_AVT_SUCCESS;
    }

    int32_t CreateControlChannel(const std::vector<std::string> &dstDevIds,
        const ChannelAttribute &attribution) override
    {
        (void) dstDevIds;
        (void) attribution;
        return DH_AVT_SUCCESS;
    }

    int32_t RegisterReceiverCallback(const std::shared_ptr<IAVReceiverEngineCallback> &callback) override
    {
        (void) callback;
        return DH_AVT_SUCCESS;
    }

    bool StartDumpMediaData() override
    {
        return false;
    }

    bool StopDumpMediaData() override
    {
        return false;
    }

    bool ReStartDumpMediaData() override
    {
        return false;
    }
};
class SenderEngineTest : public IAVSenderEngine {
public:
    SenderEngineTest() = default;
    ~SenderEngineTest() override = default;
    int32_t Initialize() override
    {
        return DH_AVT_SUCCESS;
    }

    int32_t Release() override
    {
        return DH_AVT_SUCCESS;
    }

    int32_t Start() override
    {
        return DH_AVT_SUCCESS;
    }

    int32_t Stop() override
    {
        return DH_AVT_SUCCESS;
    }

    int32_t PushData(const std::shared_ptr<AVTransBuffer> &buffer) override
    {
        (void) buffer;
        return DH_AVT_SUCCESS;
    }

    int32_t SetParameter(AVTransTag tag, const std::string &value) override
    {
        (void) tag;
        (void) value;
        return DH_AVT_SUCCESS;
    }

    int32_t SendMessage(const std::shared_ptr<AVTransMessage> &message) override
    {
        (void) message;
        return DH_AVT_SUCCESS;
    }

    int32_t CreateControlChannel(const std::vector<std::string> &dstDevIds,
        const ChannelAttribute &attribution) override
    {
        (void) dstDevIds;
        (void) attribution;
        return DH_AVT_SUCCESS;
    }

    int32_t RegisterSenderCallback(const std::shared_ptr<IAVSenderEngineCallback> &callback) override
    {
        (void) callback;
        return DH_AVT_SUCCESS;
    }

    bool StartDumpMediaData() override
    {
        return false;
    }

    bool StopDumpMediaData() override
    {
        return false;
    }

    bool ReStartDumpMediaData() override
    {
        return false;
    }
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
