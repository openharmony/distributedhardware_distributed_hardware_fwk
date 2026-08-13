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

#ifndef OHOS_OUTPUT_CONTROLLER_H
#define OHOS_OUTPUT_CONTROLLER_H
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <memory>
#include "output_controller_listener.h"
#include "time_statistician.h"
#include "output_controller_constants.h"
#include "plugin_buffer.h"
#include "plugin_types.h"
#include "av_trans_log.h"
#include "av_trans_utils.h"
#include "av_sync_utils.h"
#include "event_handler.h"

namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::Media;
using namespace OHOS::Media::Plugin;
class OutputController {
public:
    virtual ~OutputController();
    virtual void PrepareSmooth();
    virtual void PrepareSync();
    virtual void InitBaseline(const int64_t timeStampBaseline, const int64_t clockBaseline);
    virtual void InitTimeStatistician();
    virtual int32_t NotifyOutput(const std::shared_ptr<Plugin::Buffer>& data);

public:
    enum class ControlStatus {
        START,
        STOP,
        RELEASE,
        STARTED,
        STOPPED,
        RELEASED,
    };
    enum class ControlMode {
        SMOOTH,
        SYNC,
    };
    void PushData(std::shared_ptr<Plugin::Buffer>& data);
    ControlStatus StartControl();
    void PrepareControl();
    ControlStatus StopControl();
    ControlStatus ReleaseControl();
    void RegisterListener(const std::shared_ptr<OutputControllerListener>& listener);
    void UnregisterListener();

    void SetBufferTime(const uint32_t time);
    void SetTimeInitState(const bool state);
    void SetBaselineInitState(const bool state);
    void SetProcessDynamicBalanceState(const bool state);
    void SetAllowControlState(const bool state);

    uint32_t GetBufferTime();
    bool GetTimeInitState();
    bool GetBaselineInitState();
    bool GetProcessDynamicBalanceState();
    bool GetAllowControlState();

    void SetClockBaseline(const int64_t clockBaseline);
    void SetTimeStampBaseline(const int64_t timeStmapBaseline);

    void SetAverIntervalDiffThre(const uint32_t thre);
    void SetDynamicBalanceThre(const uint8_t thre);
    void SetPushOnceDiffThre(const uint32_t thre);
    void SetTimeStampOnceDiffThre(const uint32_t thre);
    void SetAdjustSleepFactor(const float factor);
    void SetWaitClockFactor(const float factor);
    void SetTrackClockFactor(const float factor);
    void SetWaitClockThre(const int64_t thre);
    void SetTrackClockThre(const int64_t thre);
    void SetSleepThre(const int64_t thre);
    void SetVideoFrontTime(const int64_t time);
    void SetVideoBackTime(const int64_t time);
    void SetAudioFrontTime(const int64_t time);
    void SetAudioBackTime(const int64_t time);

    Status GetParameter(Tag tag, ValueType& value);
    Status SetParameter(Tag tag, const ValueType& value);

private:
    ControlMode GetControlMode();
    ControlStatus GetControlStatus();
    void SetControlMode(ControlMode mode);
    void SetControlStatus(ControlStatus status);

    void InitTime(const int64_t enterTime, const int64_t timeStamp);
    void RecordTime(const int64_t enterTime, const int64_t timeStamp);
    void CheckSyncInfo(const std::shared_ptr<Plugin::Buffer>& data);
    void CalProcessTime(const std::shared_ptr<Plugin::Buffer>& data);
    void SetClockTime(const int64_t clockTime);
    int64_t GetClockTime();
    void SetDevClockDiff(int32_t diff);
    int32_t GetDevClockDiff();
    int32_t AcquireSyncClockTime(const std::shared_ptr<Plugin::Buffer>& data);
    size_t GetQueueSize();

    void ClearQueue(std::queue<std::shared_ptr<Plugin::Buffer>>& queue);

    bool CheckIsTimeInit();
    bool CheckIsBaselineInit();
    bool CheckIsAllowControl();
    bool CheckIsClockInvalid(const std::shared_ptr<Plugin::Buffer>& data);
    bool WaitRereadClockFailed(const std::shared_ptr<Plugin::Buffer>& data);
    bool CheckIsProcessInDynamicBalance(const std::shared_ptr<Plugin::Buffer>& data);
    bool CheckIsProcessInDynamicBalanceOnce(const std::shared_ptr<Plugin::Buffer>& data);

    void LooperHandle();
    void LooperControl();
    int32_t ControlOutput(const std::shared_ptr<Plugin::Buffer>& data);
    int32_t PostOutputEvent(const std::shared_ptr<Plugin::Buffer>& data);
    void HandleControlResult(const std::shared_ptr<Plugin::Buffer>& data, int32_t result);
    void CalSleepTime(const int64_t timeStamp);
    void SyncClock(const std::shared_ptr<Plugin::Buffer>& data);
    void HandleSmoothTime(const std::shared_ptr<Plugin::Buffer>& data);
    void HandleSyncTime(const std::shared_ptr<Plugin::Buffer>& data);

protected:
    std::queue<std::shared_ptr<Plugin::Buffer>> dataQueue_;
    std::map<Tag, ValueType> paramsMap_;
    std::shared_ptr<TimeStatistician> statistician_ = nullptr;
    std::shared_ptr<OutputControllerListener> listener_ = nullptr;

private:
    std::atomic<ControlStatus> status_ {ControlStatus::RELEASE};
    std::atomic<ControlMode> mode_ {ControlMode::SMOOTH};
    std::unique_ptr<std::thread> controlThread_ = nullptr;
    std::unique_ptr<std::thread> handlerThread_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;
    std::condition_variable controlCon_;
    std::condition_variable sleepCon_;
    std::condition_variable clockCon_;
    std::condition_variable handlerCon_;
    std::mutex handlerMutex_;
    std::mutex queueMutex_;
    std::mutex sleepMutex_;
    std::mutex stateMutex_;
    std::mutex modeMutex_;
    std::mutex clockMutex_;
    std::mutex paramMapMutex_;
    std::atomic<bool> isInDynamicBalance_ = true;
    std::atomic<bool> isBaselineInit_ = false;
    std::atomic<bool> isTimeInit_ = false;
    std::atomic<bool> isAllowControl_ = true;

    const uint32_t QUEUE_MAX_SIZE = 100;
    const int64_t GREATER_HALF_REREAD_TIME = 5 * NS_ONE_MS;
    const int64_t LESS_HALF_REREAD_TIME = 3 * GREATER_HALF_REREAD_TIME;
    int64_t waitClockThre_ = 0;
    int64_t trackClockThre_ = 0;
    float adjustSleepFactor_ = 0;
    float waitClockFactor_ = 0;
    float trackClockFactor_ = 0;
    uint8_t dynamicBalanceThre_ = 0;
    uint8_t dynamicBalanceCount_ = 0;
    uint32_t averIntervalDiffThre_ = 0;
    uint32_t pushOnceDiffThre_ = 0;
    uint32_t timeStampOnceDiffThre_ = 0;
    uint32_t bufferTime_ = 0;
    int64_t enterTime_ = 0;
    int64_t lastEnterTime_ = 0;
    int64_t lastTimeStamp_ = 0;
    int64_t leaveTime_ = 0;
    int64_t timeStampBaseline_ = 0;
    std::atomic<int64_t> clockTime_ = 0;
    int64_t clockBaseline_ = 0;
    int64_t delta_ = 0;
    int64_t sleep_ = 0;
    int64_t sleepThre_ = 0;
    AVTransSharedMemory sharedMem_ = { 0, 0, "" };
    AVSyncClockUnit clockUnit_ = { 0, 0, 0 };
    std::atomic<int32_t> devClockDiff_ = 0;
    int64_t aFront_ = 0;
    int64_t aBack_ = 0;
    int64_t vFront_ = 0;
    int64_t vBack_ = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_OUTPUT_CONTROLLER_H