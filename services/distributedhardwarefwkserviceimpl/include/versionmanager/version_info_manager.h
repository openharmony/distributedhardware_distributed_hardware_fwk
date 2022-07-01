/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_CAPABILITY_INFO_MANAGER_H
#define OHOS_DISTRIBUTED_HARDWARE_CAPABILITY_INFO_MANAGER_H

#include <condition_variable>
#include <map>
#include <set>

#include "kvstore_observer.h"

#include "capability_info.h"
#include "capability_info_event.h"
#include "capability_utils.h"
#include "db_adapter.h"
#include "event.h"
#include "eventbus_handler.h"
#include "event_bus.h"
#include "event_sender.h"
#include "single_instance.h"

class DBAdapter;
namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr int32_t MANUAL_SYNC_TIMEOUT = 1;
}
class CapabilityInfoManager : public std::enable_shared_from_this<CapabilityInfoManager>,
                              public EventSender,
                              public DistributedKv::KvStoreObserver,
                              public EventBusHandler<CapabilityInfoEvent> {
public:

private:
    DHVersion dhVersion_;
};
}