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

#ifndef OHOS_DISTRIBUTED_HARDWARE_IPC_SKELETON_MOCK_H
#define OHOS_DISTRIBUTED_HARDWARE_IPC_SKELETON_MOCK_H

#include <gmock/gmock.h>

#include "accesstoken_kit.h"
#include "ipc_skeleton.h"

using OHOS::Security::AccessToken::AccessTokenID;

namespace OHOS {
namespace DistributedHardware {
class IPCSkeletonInterface {
public:
    IPCSkeletonInterface() = default;
    virtual ~IPCSkeletonInterface() = default;

    virtual bool IsLocalCalling() = 0;
    virtual AccessTokenID GetCallingTokenID() = 0;
    virtual uint64_t GetCallingFullTokenID() = 0;

    static IPCSkeletonInterface* GetOrCreateIPCSkeletonMock();
    static void ReleaseIPCSkeletonMock();
private:
    static IPCSkeletonInterface* instance_;
};

class IPCSkeletonMock : public IPCSkeletonInterface {
public:
    IPCSkeletonMock() = default;
    ~IPCSkeletonMock() override = default;

    MOCK_METHOD(bool, IsLocalCalling, ());
    MOCK_METHOD(AccessTokenID, GetCallingTokenID, ());
    MOCK_METHOD(uint64_t, GetCallingFullTokenID, ());
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_HARDWARE_IPC_SKELETON_MOCK_H
