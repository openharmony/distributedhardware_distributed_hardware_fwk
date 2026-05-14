/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <iostream>

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "json_object.h"
#include "refbase.h"

#include "authorization_result_callback_stub.h"
#include "distributed_hardware_fwk_kit.h"
#include "iauthorization_result_callback.h"
using OHOS::DistributedHardware::DHTopic;

namespace OHOS {
namespace DistributedHardware {

class AuthorizationResultCallbackTest : public AuthorizationResultCallbackStub {
public:
    AuthorizationResultCallbackTest() = default;
    virtual ~AuthorizationResultCallbackTest() = default;
    void OnAuthorizationResult(const std::string &networkId, const std::string &requestId) override
    {
        std::cout << "OnAuthorizationResult: true" << std::endl;
        auto dhfwkPtr = std::make_shared<DistributedHardwareFwkKit>();
        dhfwkPtr->SetAuthorizationResult(DHType::CAMERA, requestId, true);
    }
};

int32_t GetUserInput()
{
    int32_t res = -1;
    size_t count = 3;
    std::cout << ">>" << std::endl;
    std::cin >> res;
    while (std::cin.fail() && count > 0) {
        std::cin.clear();
        std::cin.ignore();
        std::cout << "invalid input, not a number! Please retry with a number." << std::endl;
        std::cout << ">>";
        std::cin >> res;
        count--;
    }
    return res;
}

void Register()
{
    std::cout << "Start register callback." << std::endl;
    auto dhfwkPtr = std::make_shared<DistributedHardwareFwkKit>();
    std::string pkgName = "testDemo";
    sptr<IAuthorizationResultCallback> callback = new AuthorizationResultCallbackTest();
    int32_t ret = dhfwkPtr->RegisterHardwareAccessListener(DHType::CAMERA, callback, 3, pkgName);
    std::cout << "Register callback result:" << ret << std::endl;
}

void Unregister()
{
    std::cout << "Start unregister callback." << std::endl;
    auto dhfwkPtr = std::make_shared<DistributedHardwareFwkKit>();
    std::string pkgName = "testDemo";
    sptr<IAuthorizationResultCallback> callback = new AuthorizationResultCallbackTest();
    int32_t ret = dhfwkPtr->UnregisterHardwareAccessListener(DHType::CAMERA, callback, pkgName);
    std::cout << "Unregister callback result:" << ret << std::endl;
}

void HandleUserInputEvent(const int32_t cmd)
{
    switch (cmd) {
        case 1:
            Register();
            break;
        case 2: // 2: unregister
            Unregister();
            break;
        default:
            std::cout << "Unkown opeartion." << std::endl;
            break;
    }
}

void PrintInteractiveUsage()
{
    std::cout << std::endl << "=============== InteractiveRunTestSelect ================" << std::endl;
    std::cout << "You can respond to instructions for corresponding option:" << std::endl;
    std::cout << "enter 0 to exit." << std::endl;
    std::cout << "enter 1 to register" << std::endl;
    std::cout << "enter 2 to unregister" << std::endl;
}

void SetDMAccessPermission()
{
    const int32_t permsNum = 3;
    const int32_t indexZero = 0;
    const int32_t indexOne = 1;
    const int32_t indexTwo = 2;
    uint64_t tokenId;
    const char *perms[permsNum];
    perms[indexZero] = "ohos.permission.DISTRIBUTED_SOFTBUS_CENTER";
    perms[indexOne] = "ohos.permission.DISTRIBUTED_DATASYNC";
    perms[indexTwo] = "ohos.permission.ACCESS_SERVICE_DM";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = permsNum,
        .aclsNum = 0,
        .dcaps = NULL,
        .perms = perms,
        .acls = NULL,
        .processName = "dhardware",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}
}
}

int main()
{
    OHOS::DistributedHardware::SetDMAccessPermission();
    OHOS::DistributedHardware::PrintInteractiveUsage();
    for (int i = 0; i < 10; i++) { // 10: max loop times
        int32_t cmd = OHOS::DistributedHardware::GetUserInput();
        if (cmd == 0) {
            break;
        }
        OHOS::DistributedHardware::HandleUserInputEvent(cmd);
    }
    return 0;
}