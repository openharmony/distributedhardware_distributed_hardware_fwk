/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ipc_skeleton_mock.h"

using namespace OHOS::DistributedHardware;

namespace OHOS {
bool IPCSkeleton::IsLocalCalling()
{
    return IPCSkeletonInterface::GetOrCreateIPCSkeletonMock()->IsLocalCalling();
}

AccessTokenID IPCSkeleton::GetCallingTokenID()
{
    return IPCSkeletonInterface::GetOrCreateIPCSkeletonMock()->GetCallingTokenID();
}

uint64_t IPCSkeleton::GetCallingFullTokenID()
{
    return IPCSkeletonInterface::GetOrCreateIPCSkeletonMock()->GetCallingFullTokenID();
}

namespace DistributedHardware {
IPCSkeletonInterface* IPCSkeletonInterface::instance_ = nullptr;

IPCSkeletonInterface* IPCSkeletonInterface::GetOrCreateIPCSkeletonMock()
{
    if (!instance_) {
        instance_ = new IPCSkeletonMock();
    }
    return instance_;
}

void IPCSkeletonInterface::ReleaseIPCSkeletonMock()
{
    if (instance_ != nullptr) {
        delete instance_;
        instance_ = nullptr;
    }
}
} // namespace DistributedHardware
} // namespace OHOS
