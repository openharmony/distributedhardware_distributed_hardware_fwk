/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_DATA_SYNC_TRIGGER_LISTENER_H
#define OHOS_DISTRIBUTED_HARDWARE_DATA_SYNC_TRIGGER_LISTENER_H

#include "idistributed_hardware_source.h"

namespace OHOS {
namespace DistributedHardware {
class DHDataSyncTriggerListener : public DataSyncTriggerListener {
public:
    explict DHDataSyncTriggerListener();
    virtual ~DHDataSyncTriggerListener();
    void OnDataSyncTrigger(const std::string &uuid) override;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif