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

#ifndef OHOS_TIME_STATISTICIAN_H
#define OHOS_TIME_STATISTICIAN_H

#include "plugin_buffer.h"
#include <memory>
namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::Media;
using namespace OHOS::Media::Plugin;
class TimeStatistician {
public:
    virtual ~TimeStatistician() = default;
    virtual void CalProcessTime(const std::shared_ptr<Plugin::Buffer>& data);
    void CalAverPushInterval(const int64_t pushTime);
    void CalAverTimeStampInterval(const int64_t timeStamp);

public:
    int64_t GetAverPushInterval();
    int64_t GetAverTimeStampInterval();
    int64_t GetPushInterval();
    int64_t GetTimeStampInterval();
    void ClearStatistics();

public:
    uint32_t pushIndex_ = 0;
    int64_t averPushInterval_ = 0;
    int64_t lastPushTime_ = 0;
    int64_t pushTime_ = 0;
    int64_t pushIntervalSum_ = 0;
    int64_t pushInterval_ = 0;

    uint32_t timeStampIndex_ = 0;
    int64_t averTimeStampInterval_ = 0;
    int64_t lastTimeStamp_ = 0;
    int64_t timeStamp_ = 0;
    int64_t timeStampIntervalSum_ = 0;
    int64_t timeStampInterval_ = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_TIME_STATISTICIAN_H