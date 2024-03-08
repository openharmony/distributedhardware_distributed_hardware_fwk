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
#include "time_statistician.h"
#include "av_trans_log.h"

namespace OHOS {
namespace DistributedHardware {
void TimeStatistician::CalProcessTime(const std::shared_ptr<Plugin::Buffer>& data)
{
    auto bufferMeta = data->GetBufferMeta();
    if (!bufferMeta->IsExist(Tag::USER_PUSH_DATA_TIME)) {
        ClearStatistics();
        return;
    }
    int64_t pushTime = Plugin::AnyCast<int64_t>(bufferMeta->GetMeta(Tag::USER_PUSH_DATA_TIME));
    int64_t timeStamp = data->pts;
    CalAverPushInterval(pushTime);
    CalAverTimeStampInterval(timeStamp);
}
void TimeStatistician::ClearStatistics()
{
    pushIndex_ = 0;
    averPushInterval_ = 0;
    lastPushTime_ = 0;
    pushTime_ = 0;
    pushIntervalSum_ = 0;
    pushInterval_ = 0;
    timeStampIndex_ = 0;
    averTimeStampInterval_ = 0;
    lastTimeStamp_ = 0;
    timeStamp_ = 0;
    timeStampIntervalSum_ = 0;
    timeStampInterval_ = 0;
}

void TimeStatistician::CalAverPushInterval(const int64_t pushTime)
{
    pushTime_ = pushTime;
    pushInterval_ = pushTime_ - lastPushTime_;
    if (lastPushTime_ == 0) {
        lastPushTime_ = pushTime_;
        return;
    }
    pushIndex_++;
    pushIntervalSum_ += pushInterval_;
    averPushInterval_ = pushIntervalSum_ / pushIndex_;
    lastPushTime_ = pushTime_;
    AVTRANS_LOGD("Statistic pushInterval: %{public}lld, pushIndex: %{public}" PRIu32 ", averPushInterval: %{public}lld",
        pushInterval_, pushIndex_, averPushInterval_);
}

void TimeStatistician::CalAverTimeStampInterval(const int64_t timeStamp)
{
    timeStamp_ = timeStamp;
    timeStampInterval_ = timeStamp_ - lastTimeStamp_;
    if (lastTimeStamp_ == 0) {
        lastTimeStamp_ = timeStamp_;
        return;
    }
    timeStampIndex_++;
    timeStampIntervalSum_ += timeStampInterval_;
    averTimeStampInterval_ = timeStampIntervalSum_ / timeStampIndex_;
    lastTimeStamp_ = timeStamp_;
    AVTRANS_LOGD("Statistic timeStampInterval: %{public}lld, timeStampIndex: %{public}" PRIu32
        ", averTimeStampInterval: %{public}lld", timeStampInterval_, timeStampIndex_, averTimeStampInterval_);
}

int64_t TimeStatistician::GetAverPushInterval()
{
    return averPushInterval_;
}

int64_t TimeStatistician::GetAverTimeStampInterval()
{
    return averTimeStampInterval_;
}

int64_t TimeStatistician::GetPushInterval()
{
    return pushInterval_;
}

int64_t TimeStatistician::GetTimeStampInterval()
{
    return timeStampInterval_;
}
} // namespace DistributedHardware
} // namespace OHOS