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

#include <sys/prctl.h>
#include <cstdlib>
#include "output_controller.h"
#include "av_trans_constants.h"
#include "av_trans_errno.h"

namespace OHOS {
namespace DistributedHardware {
OutputController::~OutputController()
{
    ReleaseControl();
}

void OutputController::PushData(std::shared_ptr<Plugin::Buffer>& data)
{
    int64_t pushTime = GetCurrentTime();
    TRUE_RETURN((GetControlStatus() != ControlStatus::START),
        "Control status wrong, push data failed.");
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        if (dataQueue_.size() > QUEUE_MAX_SIZE) {
            AVTRANS_LOGE("DataQueue is greater than QUEUE_MAX_SIZE %zu", QUEUE_MAX_SIZE);
            dataQueue_.pop();
        }
        CheckSyncInfo(data);
        data->GetBufferMeta()->SetMeta(Tag::USER_PUSH_DATA_TIME, pushTime);
        dataQueue_.push(data);
    }
    if (GetControlMode() == ControlMode::SYNC) {
        clockCon_.notify_one();
    }
    controlCon_.notify_one();
}

OutputController::ControlStatus OutputController::StartControl()
{
    TRUE_RETURN_V_MSG_E((GetControlStatus() == ControlStatus::START), ControlStatus::STARTED,
        "Control status is started.");
    SetControlStatus(ControlStatus::START);
    if (!handlerThread_) {
        AVTRANS_LOGD("Init handler thread.");
        handlerThread_ = std::make_unique<std::thread>(&OutputController::LooperHandle, this);
        std::unique_lock<std::mutex> lock(handlerMutex_);
        handlerCon_.wait(lock, [this] {
            return handler_;
        });
    }
    if (!controlThread_) {
        AVTRANS_LOGD("Init control thread.");
        controlThread_ = std::make_unique<std::thread>(&OutputController::LooperControl, this);
    }
    AVTRANS_LOGI("Start control success.");
    return ControlStatus::START;
}

void OutputController::PrepareControl()
{
    if (GetControlMode() == ControlMode::SMOOTH) {
        PrepareSmooth();
    } else {
        PrepareSync();
    }
    InitTimeStatistician();
    AVTRANS_LOGI("Prepare control success.");
}

OutputController::ControlStatus OutputController::StopControl()
{
    TRUE_RETURN_V_MSG_E((GetControlStatus() == ControlStatus::STOP), ControlStatus::STOPPED,
        "Control status is stopped.");
    SetControlStatus(ControlStatus::STOP);
    ClearQueue(dataQueue_);
    AVTRANS_LOGI("Stop control success.");
    return ControlStatus::STOP;
}

OutputController::ControlStatus OutputController::ReleaseControl()
{
    TRUE_RETURN_V_MSG_E((GetControlStatus() == ControlStatus::RELEASE), ControlStatus::RELEASED,
        "Control status is released.");
    SetControlStatus(ControlStatus::RELEASE);
    statistician_ = nullptr;
    UnregisterListener();
    controlCon_.notify_one();
    sleepCon_.notify_one();
    clockCon_.notify_one();
    if (handler_) {
        handler_->GetEventRunner()->Stop();
    }
    if (handlerThread_) {
        handlerThread_->join();
        handlerThread_ = nullptr;
    }
    if (controlThread_) {
        controlThread_->join();
        controlThread_ = nullptr;
    }
    ClearQueue(dataQueue_);
    paramsMap_.clear();
    AVTRANS_LOGI("Release control success.");
    return ControlStatus::RELEASE;
}

Status OutputController::GetParameter(Tag tag, ValueType& value)
{
    {
        std::lock_guard<std::mutex> lock(paramMapMutex_);
        auto iter = paramsMap_.find(tag);
        if (iter != paramsMap_.end()) {
            value = iter->second;
            return Status::OK;
        }
        return Status::ERROR_NOT_EXISTED;
    }
}

Status OutputController::SetParameter(Tag tag, const ValueType& value)
{
    {
        std::lock_guard<std::mutex> lock(paramMapMutex_);
        switch (tag) {
            case Tag::USER_AV_SYNC_GROUP_INFO: {
                std::string jsonStr = Plugin::AnyCast<std::string>(value);
                AVTRANS_LOGD("Set parameter USER_AV_SYNC_GROUP_INFO: %s", jsonStr.c_str());
                break;
            }
            case Tag::USER_SHARED_MEMORY_FD: {
                std::string jsonStr = Plugin::AnyCast<std::string>(value);
                sharedMem_ = UnmarshalSharedMemory(jsonStr);
                AVTRANS_LOGD("Set parameter USER_SHARED_MEMORY_FD: %s, unmarshal sharedMem fd: %d, " +
                    "size: %d, name: %s", jsonStr.c_str(), sharedMem_.fd, sharedMem_.size,
                    sharedMem_.name.c_str());
                break;
            }
            case Tag::USER_TIME_SYNC_RESULT: {
                std::string jsonStr = Plugin::AnyCast<std::string>(value);
                int32_t devClockDiff = atoi(jsonStr.c_str());
                SetDevClockDiff(devClockDiff);
                AVTRANS_LOGD("Set parameter USER_TIME_SYNC_RESULT: %s, devClockDiff is %d.",
                    jsonStr.c_str(), devClockDiff);
                break;
            }
            default:
                AVTRANS_LOGE("Invalid tag.");
        }
        paramsMap_.insert(std::make_pair(tag, value));
        return Status::OK;
    }
}

void OutputController::PrepareSync()
{
    AVTRANS_LOGI("PrepareSync.");
    SetProcessDynamicBalanceState(true);
    SetTimeInitState(false);
    SetBaselineInitState(false);
    SetAllowControlState(true);
    SetBufferTime(0);
    SetAdjustSleepFactor(0);
    SetWaitClockThre(0);
    SetTrackClockThre(0);
    SetSleepThre(0);
}

void OutputController::PrepareSmooth()
{
    AVTRANS_LOGI("PrepareSmooth.");
    SetProcessDynamicBalanceState(false);
    SetTimeInitState(false);
    SetBaselineInitState(false);
    SetAllowControlState(true);
    SetBufferTime(0);
    SetAdjustSleepFactor(0);
    SetWaitClockFactor(0);
    SetTrackClockFactor(0);
    SetSleepThre(0);
    SetDynamicBalanceThre(0);
    SetAverIntervalDiffThre(0);
    SetPushOnceDiffThre(0);
}

void OutputController::LooperHandle()
{
    prctl(PR_SET_NAME, OUTPUT_HANDLE_THREAD_NAME.c_str());
    auto runner = AppExecFwk::EventRunner::Create(false);
    {
        std::lock_guard<std::mutex> lock(handlerMutex_);
        handler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    handlerCon_.notify_one();
    runner->Run();
}

void OutputController::LooperControl()
{
    prctl(PR_SET_NAME, LOOPER_CONTROL_THREAD_NAME.c_str());
    while (GetControlStatus() != ControlStatus::RELEASE) {
        std::shared_ptr<Plugin::Buffer> data = nullptr;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            controlCon_.wait(lock, [this] {
                return ((!dataQueue_.empty() && GetControlStatus() == ControlStatus::START)
                    || (GetControlStatus() == ControlStatus::RELEASE));
            });
            if (GetControlStatus() == ControlStatus::RELEASE) {
                continue;
            }
            data = dataQueue_.front();
        }
        HandleControlResult(data, ControlOutput(data));
    }
}

int32_t OutputController::ControlOutput(const std::shared_ptr<Plugin::Buffer>& data)
{
    enterTime_ = GetCurrentTime();
    int64_t timeStamp = data->pts;
    TRUE_RETURN_V_MSG_E((timeStamp == INVALID_TIMESTAMP || !CheckIsAllowControl()), OUTPUT_FRAME,
        "Direct output.");
    if (CheckIsClockInvalid(data) || !CheckIsProcessInDynamicBalance(data)) {
        CalProcessTime(data);
        RecordTime(enterTime_, timeStamp);
        return OUTPUT_FRAME;
    }
    CalProcessTime(data);
    if (!CheckIsTimeInit()) {
        InitTime(enterTime_, timeStamp);
    }
    if (!CheckIsBaselineInit()) {
        InitBaseline(timeStamp, GetClockTime());
    }
    int64_t interval = timeStamp - lastTimeStamp_;
    int64_t elapse = enterTime_ - leaveTime_;
    int64_t render = enterTime_ - lastEnterTime_;
    int64_t delta = render - sleep_ - elapse;
    delta_ += delta;
    sleep_ = interval - elapse;
    AVTRANS_LOGD("Control frame pts: %lld, interval: %lld, elapse: %lld, render: %lld, delta: %lld," +
        "delat count: %lld, sleep: %lld.", timeStamp, interval, elapse, render, delta, delta_, sleep_);
    AdjustSleepTime(interval);
    SyncClock(interval, data);
    {
        std::unique_lock<std::mutex> lock(sleepMutex_);
        sleepCon_.wait_for(lock, std::chrono::nanoseconds(sleep_),
            [this] { return (GetControlStatus() != ControlStatus::START); });
    }
    RecordTime(enterTime_, timeStamp);
    return OUTPUT_FRAME;
}

void OutputController::CheckSyncInfo(const std::shared_ptr<Plugin::Buffer>& data)
{
    auto bufferMeta = data->GetBufferMeta();
    if (GetControlMode() == ControlMode::SYNC &&
        (!bufferMeta->IsExist(Tag::AUDIO_SAMPLE_PER_FRAME) || (!bufferMeta->IsExist(Tag::MEDIA_START_TIME)))) {
        ClearQueue(dataQueue_);
        sleepCon_.notify_one();
        clockCon_.notify_one();
        SetControlMode(ControlMode::SMOOTH);
        AVTRANS_LOGI("Stop sync and start smooth.");
        return;
    }
    if (GetControlMode() == ControlMode::SMOOTH &&
        bufferMeta->IsExist(Tag::AUDIO_SAMPLE_PER_FRAME) && !bufferMeta->IsExist(Tag::MEDIA_START_TIME)) {
        ClearQueue(dataQueue_);
        sleepCon_.notify_one();
        clockCon_.notify_one();
        SetControlMode(ControlMode::SYNC);
        AVTRANS_LOGI("Stop smooth and start sync.");
    }
}

void OutputController::CalProcessTime(const std::shared_ptr<Plugin::Buffer>& data)
{
    if (statistician_) {
        statistician_->CalProcessTime(data);
    }
}

void OutputController::InitTimeStatistician()
{
    if (!statistician_) {
        AVTRANS_LOGD("Init time statistician.");
        statistician_ = std::make_shared<TimeStatistician>();
    }
    statistician_->ClearStatistics();
    AVTRANS_LOGI("Start time statistics.");
}

void OutputController::RecordTime(const int64_t enterTime, const int64_t timeStamp)
{
    lastEnterTime_ = enterTime;
    lastTimeStamp_ = timeStamp;
    leaveTime_ = GetCurrentTime();
    SetTimeInitState(true);
}

void OutputController::InitTime(const int64_t enterTime, const int64_t timeStamp)
{
    delta_ = 0;
    sleep_ = 0;
    lastEnterTime_ = enterTime;
    lastTimeStamp_ = timeStamp;
    leaveTime_ = enterTime;
    SetTimeInitState(true);
}

bool OutputController::CheckIsClockInvalid(const std::shared_ptr<Plugin::Buffer>& data)
{
    if (GetControlMode() == ControlMode::SMOOTH) {
        SetClockTime(enterTime_);
        AVTRANS_LOGD("Control mode is smooth, clock is valid.");
        return false;
    }
    int32_t ret = AcquireSyncClockTime(data);
    if (ret == ERR_DH_AVT_MASTER_NOT_READY) {
        AVTRANS_LOGD("Master clock not ready, wait reread clock.");
        return WaitRereadClockFailed(data);
    }
    TRUE_RETURN_V_MSG_E((ret != DH_AVT_SUCCESS), true, "Read unit clock is invalid.");
    return false;
}

int32_t OutputController::AcquireSyncClockTime(const std::shared_ptr<Plugin::Buffer>& data)
{
    AVTransSharedMemory sharedMem;
    sharedMem.fd = sharedMem_.fd;
    sharedMem.size = sharedMem_.size;
    sharedMem.name = sharedMem_.name;
    AVSyncClockUnit clockUnit;
    auto bufferMeta = data->GetBufferMeta();
    clockUnit.frameNum = Plugin::AnyCast<uint32_t>(bufferMeta->GetMeta(Tag::AUDIO_SAMPLE_PER_FRAME));
    int32_t ret = ReadClockUnitFromMemory(sharedMem, clockUnit);
    if (ret == DH_AVT_SUCCESS) {
        clockUnit_.cts = clockUnit.cts;
        SetClockTime(clockUnit_.cts);
        AVTRANS_LOGD("Acquire sync clock success, cts: %lld.", clockUnit_.cts);
    }
    return ret;
}

bool OutputController::WaitRereadClockFailed(const std::shared_ptr<Plugin::Buffer>& data)
{
    const uint32_t halfQueueSize = QUEUE_MAX_SIZE / 2;
    while (GetQueueSize() < QUEUE_MAX_SIZE) {
        if (GetQueueSize() < halfQueueSize) {
            {
                AVTRANS_LOGD("Dataqueue size is less than half size, wait notify.");
                std::unique_lock<std::mutex> lock(clockMutex_);
                clockCon_.wait(lock);
            }
        } else if (GetQueueSize() >= halfQueueSize) {
            {
                AVTRANS_LOGD("Dataqueue size is greater than half size, scheduled %lld query.", WAIT_REREAD_TIME);
                std::unique_lock<std::mutex> lock(clockMutex_);
                clockCon_.wait_for(lock, std::chrono::nanoseconds(WAIT_REREAD_TIME),
                    [this] { return (GetControlStatus() != ControlStatus::START); });
            }
        }
        int32_t ret = AcquireSyncClockTime(data);
        if (ret == DH_AVT_SUCCESS) {
            AVTRANS_LOGD("Wait reread clock success.");
            return false;
        }
    }
    return true;
}

bool OutputController::CheckIsProcessInDynamicBalance(const std::shared_ptr<Plugin::Buffer>& data)
{
    TRUE_RETURN_V_MSG_E((GetControlMode() == ControlMode::SYNC || GetProcessDynamicBalanceState()), true,
        "Process in dynamic balance.");
    if (CheckIsProcessInDynamicBalanceOnce(data)) {
        dynamicBalanceCount_++;
    } else {
        dynamicBalanceCount_ = 0;
    }
    if (dynamicBalanceCount_ >= dynamicBalanceThre_) {
        AVTRANS_LOGD("Process meet dynamic balance condition.");
        SetProcessDynamicBalanceState(true);
        return true;
    }
    return false;
}

bool OutputController::CheckIsProcessInDynamicBalanceOnce(const std::shared_ptr<Plugin::Buffer>& data)
{
    TRUE_RETURN_V_MSG_E((!statistician_), false, "Statistician is nullptr.");
    int64_t pushInterval = statistician_->GetPushInterval();
    int64_t averPushInterval = statistician_->GetAverPushInterval();
    int64_t averTimeStamapInterval = statistician_->GetAverTimeStampInterval();
    int64_t averIntervalDiff = averPushInterval - averTimeStamapInterval;
    int64_t pushOnceDiff = pushInterval - averPushInterval;
    return (averPushInterval != 0) && (averTimeStamapInterval != 0) &&
        (llabs(averIntervalDiff) < averIntervalDiffThre_) && (llabs(pushOnceDiff) < pushOnceDiffThre_);
}

bool OutputController::CheckIsBaselineInit()
{
    return GetBaselineInitState();
}

bool OutputController::CheckIsTimeInit()
{
    return GetTimeInitState();
}

bool OutputController::CheckIsAllowControl()
{
    return GetAllowControlState();
}

void OutputController::InitBaseline(const int64_t timeStampBaseline, const int64_t clockBaseline)
{
    SetTimeStampBaseline(timeStampBaseline);
    SetClockBaseline(clockBaseline + bufferTime_);
    SetBaselineInitState(true);
}

void OutputController::AdjustSleepTime(const int64_t interval)
{
    TRUE_RETURN((interval == INVALID_INTERVAL), "Interverl is Invalid.");
    const int64_t adjustThre = interval * adjustSleepFactor_;
    if (delta_ > adjustThre && sleep_ > 0) {
        int64_t sleep = sleep_ - adjustThre;
        delta_ -= (sleep < 0) ? sleep_ : adjustThre;
        sleep_ = sleep;
        AVTRANS_LOGD("Delta greater than thre, adjust sleep to %lld.", sleep_);
    } else if (delta_ < -adjustThre) {
        sleep_ += delta_;
        delta_ = 0;
        AVTRANS_LOGD("Delta less than negative thre, adjust sleep to %lld.", sleep_);
    }
}

void OutputController::SyncClock(const int64_t interval, const std::shared_ptr<Plugin::Buffer>& data)
{
    if (GetControlMode() == ControlMode::SMOOTH) {
        HandleSmoothTime(interval, data);
    } else {
        HandleSyncTime(data);
    }
    if (sleep_ > sleepThre_) {
        sleep_ = sleepThre_;
        AVTRANS_LOGD("Sleep is more than sleepThre %lld, adjust sleep to %lld", sleepThre_, sleep_);
    }
    if (sleep_ < 0) {
        sleep_ = 0;
        AVTRANS_LOGD("Sleep less than zero, adjust sleep to zero.");
    }
    AVTRANS_LOGD("After sync clock, sleep is %lld.", sleep_);
}

void OutputController::HandleSmoothTime(const int64_t timeStampInterval, const std::shared_ptr<Plugin::Buffer>& data)
{
    TRUE_RETURN((timeStampInterval == INVALID_INTERVAL), "Interverl is Invalid.");
    int64_t vTimeStamp = data->pts;
    int64_t offset = vTimeStamp - timeStampBaseline_ - sleep_ - (GetClockTime() - clockBaseline_);
    int64_t averTimeStampInterval = statistician_->GetAverTimeStampInterval();
    int64_t waitClockThre = averTimeStampInterval * waitClockFactor_;
    int64_t trackClockThre = averTimeStampInterval * trackClockFactor_;
    AVTRANS_LOGD("Smooth vTimeStamp: %lld, offset: %lld, waitClockThre: %lld, trackClockThre: %lld.",
        vTimeStamp, offset, averTimeStampInterval, waitClockThre, trackClockThre);
    if (offset > waitClockThre || offset < -trackClockThre) {
        sleep_ += offset;
        AVTRANS_LOGD("Smooth offset %lld is over than thre, adjust sleep to %lld.",
            offset, sleep_);
    }
}

void OutputController::HandleSyncTime(const std::shared_ptr<Plugin::Buffer>& data)
{
    auto bufferMeta = data->GetBufferMeta();
    int64_t vTimeStamp = data->pts;
    uint32_t vFrameNumber = Plugin::AnyCast<uint32_t>(bufferMeta->GetMeta(Tag::USER_FRAME_NUMBER));
    int64_t vcts = (sleep_ > 0) ? (enterTime_ + sleep_) : enterTime_;
    int64_t aTimeStamp = Plugin::AnyCast<int64_t>(bufferMeta->GetMeta(Tag::MEDIA_START_TIME));
    uint32_t aFrameNumber = Plugin::AnyCast<uint32_t>(bufferMeta->GetMeta(Tag::AUDIO_SAMPLE_PER_FRAME));
    int64_t acts = GetClockTime();
    int64_t ctsDiff = vcts - acts;
    int64_t offset = vcts - (v1_ + a1_ - (vTimeStamp - aTimeStamp)) - acts + (v2_ - a2_) - GetDevClockDiff();
    AVTRANS_LOGD("Sync vTimeStamp: %lld, vFrameNumber: %" PRIu32 " vcts: %lld, aTimeStamp: %lld, " +
        "aFrameNumber: %" PRIu32 " acts: %lld, ctsDiff: %lld, offset: %lld", vTimeStamp, vFrameNumber,
        vcts, aTimeStamp, aFrameNumber, acts, ctsDiff, offset);
    if (offset < -waitClockThre_) {
        const int64_t append = (trackClockThre_ + waitClockThre_) / 2;
        sleep_ = (-waitClockThre_ - offset) + append;
        AVTRANS_LOGD("Sync offset %lld is over than wait thre %lld, adjust sleep to %lld.",
            offset, -waitClockThre_, sleep_);
    } else if (offset > trackClockThre_) {
        sleep_ = 0;
        AVTRANS_LOGD("Sync offset %lld is over than track thre %lld, adjust sleep to %lld.",
            offset, trackClockThre_, sleep_);
    }
}

void OutputController::HandleControlResult(const std::shared_ptr<Plugin::Buffer>& data, int32_t result)
{
    switch (result) {
        case OUTPUT_FRAME: {
            int32_t ret = PostOutputEvent(data);
            TRUE_RETURN((ret == HANDLE_FAILED), "Handle result OUTPUT_FRAME failed.");
            {
                std::lock_guard<std::mutex> lock(queueMutex_);
                if (data == dataQueue_.front() && !dataQueue_.empty()) {
                    dataQueue_.pop();
                }
            }
            break;
        }
        case DROP_FRAME: {
            {
                std::lock_guard<std::mutex> lock(queueMutex_);
                if (data == dataQueue_.front() && !dataQueue_.empty()) {
                    dataQueue_.pop();
                }
            }
            break;
        }
        case REPEAT_FREAM: {
            int32_t ret = PostOutputEvent(data);
            TRUE_RETURN((ret == HANDLE_FAILED), "Handle result REPEAT_FREAM failed.");
            break;
        }
        default:
            AVTRANS_LOGE("Invalid result.");
    }
}

int32_t OutputController::PostOutputEvent(const std::shared_ptr<Plugin::Buffer>& data)
{
    TRUE_RETURN_V_MSG_E((!handler_), HANDLE_FAILED, "Handler is nullptr.");
    auto outputFunc = [this, data]() {
        int32_t ret = NotifyOutput(data);
        TRUE_RETURN_V_MSG_E((ret == NOTIFY_FAILED), HANDLE_FAILED, "Notify failed.");
        return HANDLE_SUCCESS;
    };
    TRUE_RETURN_V_MSG_E((!handler_->PostTask(outputFunc)), HANDLE_FAILED, "Handler post task failed.");
    return HANDLE_SUCCESS;
}

int32_t OutputController::NotifyOutput(const std::shared_ptr<Plugin::Buffer>& data)
{
    TRUE_RETURN_V_MSG_E((!listener_), NOTIFY_FAILED, "Listener is nullptr.");
    return listener_->OnOutput(data);
}

void OutputController::RegisterListener(const std::shared_ptr<OutputControllerListener>& listener)
{
    listener_ = listener;
}

void OutputController::UnregisterListener()
{
    listener_ = nullptr;
}

void OutputController::SetBufferTime(const uint32_t time)
{
    bufferTime_ = time;
}

uint32_t OutputController::GetBufferTime()
{
    return bufferTime_;
}

void OutputController::SetTimeInitState(const bool state)
{
    isTimeInit_.store(state);
}

bool OutputController::GetTimeInitState()
{
    return isTimeInit_.load();
}

void OutputController::SetBaselineInitState(const bool state)
{
    isBaselineInit_.store(state);
}

bool OutputController::GetBaselineInitState()
{
    return isBaselineInit_.load();
}

void OutputController::SetProcessDynamicBalanceState(const bool state)
{
    isInDynamicBalance_.store(state);
}

bool OutputController::GetProcessDynamicBalanceState()
{
    return isInDynamicBalance_.load();
}

void OutputController::SetAllowControlState(const bool state)
{
    isAllowControl_.store(state);
}

bool OutputController::GetAllowControlState()
{
    return isAllowControl_.load();
}

void OutputController::SetClockBaseline(const int64_t clockBaseline)
{
    clockBaseline_ = clockBaseline;
}

void OutputController::SetTimeStampBaseline(const int64_t timeStmapBaseline)
{
    timeStampBaseline_ = timeStmapBaseline;
}

void OutputController::SetAverIntervalDiffThre(const uint32_t thre)
{
    averIntervalDiffThre_ = thre;
}

void OutputController::SetDynamicBalanceThre(const uint8_t thre)
{
    dynamicBalanceThre_ = thre;
}

void OutputController::SetPushOnceDiffThre(const uint32_t thre)
{
    pushOnceDiffThre_ = thre;
}

void OutputController::SetAdjustSleepFactor(const float factor)
{
    adjustSleepFactor_ = factor;
}

void OutputController::SetWaitClockFactor(const float factor)
{
    waitClockFactor_ = factor;
}

void OutputController::SetTrackClockFactor(const float factor)
{
    trackClockFactor_ = factor;
}

void OutputController::SetWaitClockThre(const int64_t thre)
{
    waitClockThre_ = thre;
}

void OutputController::SetTrackClockThre(const int64_t thre)
{
    trackClockThre_ = thre;
}

void OutputController::SetSleepThre(const int64_t thre)
{
    sleepThre_ = thre;
}

void OutputController::SetClockTime(const int64_t clockTime)
{
    clockTime_.store(clockTime);
}

int64_t OutputController::GetClockTime()
{
    return clockTime_.load();
}

void OutputController::SetDevClockDiff(int32_t diff)
{
    devClockDiff_.store(diff);
}

int32_t OutputController::GetDevClockDiff()
{
    return devClockDiff_.load();
}

OutputController::ControlMode OutputController::GetControlMode()
{
    return mode_.load();
}

OutputController::ControlStatus OutputController::GetControlStatus()
{
    return status_.load();
}

void OutputController::SetControlMode(ControlMode mode)
{
    mode_.store(mode);
    PrepareControl();
}

void OutputController::SetControlStatus(ControlStatus status)
{
    status_.store(status);
}

size_t OutputController::GetQueueSize()
{
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        return dataQueue_.size();
    }
}

void OutputController::ClearQueue(std::queue<std::shared_ptr<Plugin::Buffer>>& queue)
{
    std::queue<std::shared_ptr<Plugin::Buffer>> empty;
    swap(empty, queue);
}
} // namespace DistributedHardware
} // namespace OHOS