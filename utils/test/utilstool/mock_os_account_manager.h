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

#ifndef OHOS_DISTRIBUTED_HARDWARE_MOCK_OS_ACCOUNT_MANAGER_H
#define OHOS_DISTRIBUTED_HARDWARE_MOCK_OS_ACCOUNT_MANAGER_H

#include <gmock/gmock.h>
#include <memory>

#include "os_account_manager.h"

namespace OHOS {
namespace AccountSA {

class IOsAccountManager {
public:
    IOsAccountManager() = default;
    virtual ~IOsAccountManager() = default;
    virtual ErrCode GetForegroundOsAccountLocalId(int32_t& userId) = 0;
    virtual ErrCode GetOsAccountDomainInfo(int32_t userId, AccountSA::DomainAccountInfo& info) = 0;
public:
    static inline std::shared_ptr<AccountSA::IOsAccountManager> ParamDataSyncMgrInstance_;
};

class MockIOsAccountManager : public IOsAccountManager {
public:
    MockIOsAccountManager() = default;
    ~MockIOsAccountManager() override = default;
    MOCK_METHOD(ErrCode, GetForegroundOsAccountLocalId, (int32_t& userId));
    MOCK_METHOD(ErrCode, GetOsAccountDomainInfo, (int32_t userId, AccountSA::DomainAccountInfo& info));
};
} // namespace AccountSA
} // namespace OHOS

#endif // OHOS_DISTRIBUTED_HARDWARE_MOCK_OS_ACCOUNT_MANAGER_H
