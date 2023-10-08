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

#ifndef OHOS_AV_TRANS_ENGINE_PLUGINS_INPUT_DSOFTBUS_H
#define OHOS_AV_TRANS_ENGINE_PLUGINS_INPUT_DSOFTBUS_H

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

#include "av_trans_buffer.h"
#include "av_trans_errno.h"
#include "av_trans_constants.h"
#include "av_trans_log.h"
#include "av_trans_meta.h"
#include "av_trans_types.h"
#include "av_trans_utils.h"
#include "avtrans_input_plugin.h"
#include "nlohmann/json.hpp"
#include "foundation/osal/thread/task.h"
#include "plugin_types.h"
#include "plugin_manager.h"
#include "softbus_channel_adapter.h"

namespace OHOS {
namespace DistributedHardware {

using namespace Media::Plugin;

using json = nlohmann::json;
using AVDataCallback = std::function<void(std::shared_ptr<Buffer>)>;

class DsoftbusInputPlugin : public AvTransInputPlugin,
                            public ISoftbusChannelListener,
                            public std::enable_shared_from_this<DsoftbusInputPlugin> {
public:
    explicit DsoftbusInputPlugin(std::string name);
    ~DsoftbusInputPlugin();

    Status Init() override;
    Status Deinit() override;
    Status Prepare() override;
    Status Reset() override;
    Status Start() override;
    Status Stop() override;
    Status Pause() override;
    Status Resume() override;
    Status GetParameter(Tag tag, ValueType &value) override;
    Status SetParameter(Tag tag, const ValueType &value) override;
    Status PushData(const std::string &inPort, std::shared_ptr<Buffer> buffer, int32_t offset) override;
    Status SetCallback(Callback *cb) override;
    Status SetDataCallback(AVDataCallback callback) override;
    bool GetDumpFlag();
    void SetDumpFlagFalse();
    bool GetReDumpFlag();
    void SetReDumpFlagFalse();

    // interface from ISoftbusChannelListener
    void OnChannelEvent(const AVTransEvent &event) override;
    void OnStreamReceived(const StreamData *data, const StreamData *ext) override;

private:
    void HandleData();
    void DataEnqueue(std::shared_ptr<Buffer> &buffer);
    void DataQueueClear(std::queue<std::shared_ptr<Buffer>> &queue);
    std::shared_ptr<Buffer> CreateBuffer(uint32_t metaType, const StreamData *data, const json &resMsg);

private:
    std::atomic<bool> dumpFlag_ ;
    std::atomic<bool> reDumpFlag_ ;
    std::string ownerName_;
    std::string sessionName_;
    std::string peerDevId_;
    std::condition_variable dataCond_;
    std::shared_ptr<Media::OSAL::Task> bufferPopTask_;
    std::mutex dataQueueMtx_;
    Media::OSAL::Mutex operationMutes_ {};
    std::queue<std::shared_ptr<Buffer>> dataQueue_;
    std::map<Tag, ValueType> paramsMap_;
    State state_ {State::CREATED};
    Callback* eventsCb_ = nullptr;
    AVDataCallback dataCb_;
};
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_AV_TRANS_ENGINE_PLUGINS_OUTPUT_DSOFTBUS_H