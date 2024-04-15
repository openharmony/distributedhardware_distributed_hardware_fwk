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

#ifndef OHOS_DISTRIBUTED_HARDWARE_AVTRANS_INPUT_TEST_H
#define OHOS_DISTRIBUTED_HARDWARE_AVTRANS_INPUT_TEST_H

#include "gtest/gtest.h"

#include "avtrans_input.h"

#include "av_sync_utils.h"
#include "av_trans_buffer.h"
#include "av_trans_constants.h"
#include "av_trans_log.h"
#include "av_trans_meta.h"
#include "av_trans_types.h"
#include "av_trans_utils.h"
#include "avtrans_input_plugin.h"
#include "plugin_manager.h"
#include "plugin_types.h"

namespace OHOS {
namespace DistributedHardware {
using namespace testing::ext;
using namespace std;
using AVDataCallback = std::function<void(std::shared_ptr<Buffer>)>;
class AvTransInputTest : public testing::Test {
public:
    AvTransInputTest() {}
    ~AvTransInputTest() {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<AvTransInput> input_ = nullptr;
};
class AvTransInputPluginTest : public AvTransInputPlugin {
public:
    explicit AvTransInputPluginTest(std::string name) : AvTransInputPlugin(std::move(name))
    {
        (void) name;
    }
    ~AvTransInputPluginTest() = default;
    Status Init() override
    {
        return Status::OK;
    }
    Status Deinit() override
    {
        return Status::OK;
    }
    Status Reset() override
    {
        return Status::OK;
    }
    Status Pause() override
    {
        return Status::OK;
    }
    Status Resume() override
    {
        return Status::OK;
    }
    Status GetParameter(Tag tag, ValueType &value) override
    {
        (void) tag;
        (void) value;
        return Status::OK;
    }
    Status SetParameter(Tag tag, const ValueType &value) override
    {
        (void) tag;
        (void) value;
        return Status::OK;
    }
    Status PushData(const std::string &inPort, std::shared_ptr<Plugin::Buffer> buffer, int32_t offset) override
    {
        (void) inPort;
        (void) buffer;
        (void) offset;
        return Status::OK;
    }
    Status SetCallback(Callback *cb) override
    {
        (void) cb;
        return Status::OK;
    }
    Status SetDataCallback(AVDataCallback callback) override
    {
        (void) callback;
        return Status::OK;
    }
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_HARDWARE_AVTRANS_INPUT_TEST_H