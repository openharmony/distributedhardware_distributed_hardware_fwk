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

#ifndef OHOS_AV_TRANS_ENGINE_PLUGINS_INPUT_DSRCEEN_H
#define OHOS_AV_TRANS_ENGINE_PLUGINS_INPUT_DSRCEEN_H

#include <atomic>
#include <map>
#include <memory>
#include <mutex>

#include "av_sync_utils.h"
#include "av_trans_buffer.h"
#include "av_trans_errno.h"
#include "av_trans_constants.h"
#include "av_trans_log.h"
#include "av_trans_meta.h"
#include "av_trans_types.h"
#include "avtrans_input_plugin.h"
#include "foundation/osal/thread/task.h"
#include "plugin_types.h"
#include "plugin_manager.h"

namespace OHOS {
namespace DistributedHardware {

using namespace Media::Plugin;
using AVDataCallback = std::function<void(std::shared_ptr<Buffer>)>;

class DscreenInputPlugin : public AvTransInputPlugin {
public:
    explicit DscreenInputPlugin(std::string name);
    ~DscreenInputPlugin();

    Status Init() override;
    Status Deinit() override;
    Status Reset() override;
    Status Pause() override;
    Status Resume() override;
    Status GetParameter(Tag tag, ValueType &value) override;
    Status SetParameter(Tag tag, const ValueType &value) override;
    Status PushData(const std::string &inPort, std::shared_ptr<Buffer> buffer, int32_t offset) override;
    Status SetCallback(Callback *cb) override;
    Status SetDataCallback(AVDataCallback callback) override;

private:
    std::atomic<uint32_t> frameNumber_;
    Media::OSAL::Mutex operationMutes_ {};
    std::map<Tag, ValueType> paramsMap_;
    AVTransSharedMemory sharedMemory_ = AVTransSharedMemory{ 0, 0, "" };
};

} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_AV_TRANS_ENGINE_PLUGINS_INPUT_DSRCEEN_H