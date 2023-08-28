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

#ifndef OHOS_AV_TRANS_CONTROL_CENTER_TEST_H
#define OHOS_AV_TRANS_CONTROL_CENTER_TEST_H

#include <gtest/gtest.h>

#define private public
#include "av_trans_control_center.h"
#undef private
#include "av_trans_control_center_callback_stub.h"
#include "av_trans_types.h"
#include "av_sync_utils.h"

namespace OHOS {
namespace DistributedHardware {
class AVTransControlCenterTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<AVTransControlCenter> center_ = nullptr;
};
class CenterCallback : public AVTransControlCenterCallbackStub {
public:
    CenterCallback() = default;
    ~CenterCallback() override = default;
    int32_t SetParameter(AVTransTag tag, const std::string &value) override
    {
        value_ = value;
        return DH_AVT_SUCCESS;
    }
    int32_t SetSharedMemory(const AVTransSharedMemory &memory) override
    {
        memory_ = memory;
        return DH_AVT_SUCCESS;
    }
    int32_t Notify(const AVTransEvent &event) override
    {
        return DH_AVT_SUCCESS;
    }
    std::string value_;
    AVTransSharedMemory memory_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
