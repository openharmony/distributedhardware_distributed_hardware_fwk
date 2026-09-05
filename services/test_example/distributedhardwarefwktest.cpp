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
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <chrono>
#include <cinttypes>
#include <thread>
#include <refbase.h>

#include <securec.h>
#include "distributedhardwarefwktest.h"
#include "distributed_hardware_fwk_kit.h"
#include "distributed_hardware_log.h"
#include "distributed_hardware_errno.h"
#include "dhardware_descriptor.h"
#include "hilog/log.h"

#include "anonymous_string.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "softbus_common.h"

#include "dhfwk_sa_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "dm_device_info.h"
#include "device_manager.h"

#include "get_dh_descriptors_callback_stub.h"
#include "hardware_status_listener_stub.h"
#include "publisher_listener_stub.h"
#include "system_ability_load_callback_stub.h"
#include "ipublisher_listener.h"

using OHOS::DistributedHardware::DistributedHardwareFwkKit;
using OHOS::DistributedHardware::DHDescriptor;
using OHOS::DistributedHardware::DHType;
using OHOS::sptr;


namespace {
using namespace OHOS::DistributedHardware;

const int32_t CMD_QUIT = 0;
const int32_t CMD_ENABLE_SINK = 1;
const int32_t CMD_ENABLE_SOURCE = 2;
const int32_t CMD_DISABLE_SINK = 3;
const int32_t CMD_DISABLE_SOURCE = 4;
const int32_t CMD_INPUT_ENABLE_FIRST_TOKENID = 5;
const int32_t OHOS_PERMISSION_ENABLE_DISTRIBUTED_HARDWARE_INDEX = 0;
const int32_t OHOS_PERMISSION_DISTRIBUTED_DATASYNC_INDEX = 1;
const int32_t OHOS_PERMISSION_ACCESS_DISTRIBUTED_HARDWARE_INDEX = 2;
const int32_t OHOS_PERMISSION_ACCESS_DM_SERVICE_PERMISSION_INDEX = 3;
const int32_t SLEEP_TIME = 5000;
uint32_t g_enableFirstTokenId = 0;
static std::shared_ptr<DistributedHardwareFwkKit> g_dhFwkKit = nullptr;

static int32_t GetUserInput()
{
    int32_t res = -1;
    size_t count = 3;
    std::cout << ">>";
    int ret = scanf_s("%d", &res);
    if (ret == -1) {
        std::cout << "get input error" << std::endl;
    }
    while (std::cin.fail() && count > 0) {
        std::cin.clear();
        std::cin.ignore();
        std::cout << "invalid input, not a number! Please retry with a number." << std::endl;
        std::cout << ">>";
        ret = scanf_s("%d", &res);
        if (ret == -1) {
            std::cout << "get input error" << std::endl;
        }
        count--;
    }
    return res;
}

class TestGetDistributedHardwareCallback : public GetDhDescriptorsCallbackStub {
public:
    TestGetDistributedHardwareCallback() = default;
    virtual ~TestGetDistributedHardwareCallback() = default;
    void WaitForResult()
    {
        DHLOGI("4");
        std::unique_lock<std::mutex> lock(waitMutex_);
        DHLOGI("5");
        if (descriptors_.size() == 0) {
            DHLOGI("6");
            wait_.wait_for(lock, std::chrono::milliseconds(SLEEP_TIME));
        }
        DHLOGI("7");
    }
    std::vector<DHDescriptor> GetDescriptors()
    {
        return descriptors_;
    }
protected:
    void OnSuccess(const std::string &networkId, const std::vector<DHDescriptor> &descriptors,
        EnableStep enableStep) override;
    void OnError(const std::string &networkId, int32_t error) override;
protected:
    std::condition_variable wait_;
    std::mutex waitMutex_;
    std::vector<DHDescriptor> descriptors_;
};

void TestGetDistributedHardwareCallback::OnSuccess(
    const std::string &networkId, const std::vector<DHDescriptor> &descriptors, EnableStep enableStep)
{
    {
        std::unique_lock<std::mutex> lock(waitMutex_);
        descriptors_ = descriptors;
        wait_.notify_all();
    }

    printf("TestGetDistributedHardwareCallback::OnSuccess, networkId: %s\n", networkId.c_str());
}

void TestGetDistributedHardwareCallback::OnError(
    const std::string &networkId, int32_t error)
{
    {
        std::unique_lock<std::mutex> lock(waitMutex_);
        wait_.notify_all();
    }

    printf("TestGetDistributedHardwareCallback::OnError, networkId: %s, error: %d\n", networkId.c_str(), error);
}

class TestDmInitCallback : public DmInitCallback {
public:
    TestDmInitCallback() = default;
    virtual ~TestDmInitCallback() = default;
protected:
    void OnRemoteDied() override;
};

void TestDmInitCallback::OnRemoteDied()
{
}

static void InputEnableFirstTokenId()
{
    std::cout << "Please input enableFirstTokenId." << std::endl;
    int ret = scanf_s("%u", &g_enableFirstTokenId);
    if (ret != 1) {
        std::cout << "get input error" << std::endl;
    }
    DHLOGI("[MultiUserEnable] user input enableFirstTokenId=%{public}s",
        GetAnonyString(std::to_string(g_enableFirstTokenId)).c_str());
}

static int32_t InitTestDemo()
{
    std::cout << "**********************************************************************************" << std::endl;
    std::cout << "Distributed Hardware Fwk Test Demo Bin v1.0." << std::endl;
    std::cout << "**********************************************************************************" << std::endl;
    std::cout << std::endl;
    std::cout << "Init distributed hardware fwk kit." << std::endl;
    g_dhFwkKit = std::make_shared<DistributedHardwareFwkKit>();
    if (g_dhFwkKit == nullptr) {
        std::cout << "Distributed hardware fwk kit is null." << std::endl;
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    std::cout << "Load distributed hardware fwk kit success." << std::endl;
    return DH_FWK_SUCCESS;
}

static void SetNativeToken()
{
    uint64_t tokenId;
    int32_t numberOfPermissions = 4;
    const char *perms[numberOfPermissions];
    perms[OHOS_PERMISSION_ENABLE_DISTRIBUTED_HARDWARE_INDEX] = "ohos.permission.ENABLE_DISTRIBUTED_HARDWARE";
    perms[OHOS_PERMISSION_DISTRIBUTED_DATASYNC_INDEX] = "ohos.permission.DISTRIBUTED_DATASYNC";
    perms[OHOS_PERMISSION_ACCESS_DISTRIBUTED_HARDWARE_INDEX] = "ohos.permission.ACCESS_DISTRIBUTED_HARDWARE";
    perms[OHOS_PERMISSION_ACCESS_DM_SERVICE_PERMISSION_INDEX] = "ohos.permission.ACCESS_SERVICE_DM";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = numberOfPermissions,
        .aclsNum = 0,
        .dcaps = NULL,
        .perms = perms,
        .acls = NULL,
        .processName = "dhardware1",
        .aplStr = "system_basic",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

static void EnableSink()
{
    SetNativeToken();

    if (g_dhFwkKit == nullptr) {
        std::cout << "Distributed hardware fwk kit is null." << std::endl;
        return;
    }
    // 获取本地分布式硬件能力
    std::string networkId = "local";
    std::vector<DHDescriptor> descriptors;
    TestGetDistributedHardwareCallback *lpCallback = new TestGetDistributedHardwareCallback();
    sptr<IGetDhDescriptorsCallback> callback(lpCallback);
    g_dhFwkKit->GetDistributedHardware(networkId, EnableStep::ENABLE_SINK, callback);
    lpCallback->WaitForResult();
    descriptors = lpCallback->GetDescriptors();
    if (descriptors.empty()) {
        printf("No local distributed hardware!\n");
        return;
    }
    for (auto &descriptor : descriptors) {
        descriptor.firstCallingTokenId = g_enableFirstTokenId;
        DHLOGI("[MultiUserEnable] EnableSink enableFirstTokenId=%{public}s",
            GetAnonyString(std::to_string(g_enableFirstTokenId)).c_str());
        printf("local device info: dhType= %u, dhId = %s\n", descriptor.dhType, descriptor.id.c_str());
    }
    int32_t ret = g_dhFwkKit->EnableSink(descriptors);
    if (ret != DH_FWK_SUCCESS) {
        std::cout << "Enable sink failed, ret: " << ret << std::endl;
        return;
    }
    std::cout << "Enable sink success." << std::endl;
}

static void EnableSource()
{
    SetNativeToken();

    if (g_dhFwkKit == nullptr) {
        std::cout << "Distributed hardware fwk kit is null." << std::endl;
        return;
    }
    std::vector<DmDeviceInfo> deviceList;
    std::shared_ptr<DmInitCallback> callBack = std::make_shared<TestDmInitCallback>();
    DeviceManager::GetInstance().InitDeviceManager("ohos.dhardware.test", callBack);
    DeviceManager::GetInstance().GetTrustedDeviceList("ohos.dhardware.test", "", deviceList);
    DeviceManager::GetInstance().UnInitDeviceManager("ohos.dhardware.test");

    if (deviceList.size() == 0) {
        printf("No networking devices\n");
        return;
    }
    // 打印并使能所有分布式硬件
    for (auto &device : deviceList) {
        std::vector<DHDescriptor> descriptors;
        TestGetDistributedHardwareCallback *lpCallback = new TestGetDistributedHardwareCallback();
        sptr<IGetDhDescriptorsCallback> callback(lpCallback);
        g_dhFwkKit->GetDistributedHardware(device.networkId, EnableStep::ENABLE_SOURCE, callback);
        lpCallback->WaitForResult();
        descriptors = lpCallback->GetDescriptors();
        for (auto &descriptor : descriptors) {
            descriptor.firstCallingTokenId = g_enableFirstTokenId;
            DHLOGI("[MultiUserEnable] EnableSource enableFirstTokenId=%{public}s",
                GetAnonyString(std::to_string(g_enableFirstTokenId)).c_str());
            printf("remote device info: networkId = %s, dhType = %u, dhId = %s\n",
                device.networkId, descriptor.dhType, descriptor.id.c_str());
        }
        printf("start EnableSource task.\n");
        int32_t ret = g_dhFwkKit->EnableSource(device.networkId, descriptors);
        if (ret != DH_FWK_SUCCESS) {
            std::cout << "Enable source failed, ret: " << ret << std::endl;
            return;
        }
    }
    std::cout << "Enable source success." << std::endl;
}

static void DisableSink()
{
    SetNativeToken();

    if (g_dhFwkKit == nullptr) {
        std::cout << "Distributed hardware fwk kit is null." << std::endl;
        return;
    }
    std::string networkId = "local";
    std::vector<DHDescriptor> descriptors;
    TestGetDistributedHardwareCallback *lpCallback = new TestGetDistributedHardwareCallback();
    sptr<IGetDhDescriptorsCallback> callback(lpCallback);
    g_dhFwkKit->GetDistributedHardware(networkId, EnableStep::ENABLE_SINK, callback);
    lpCallback->WaitForResult();
    descriptors = lpCallback->GetDescriptors();

    int32_t ret = g_dhFwkKit->DisableSink(descriptors);
    if (ret != DH_FWK_SUCCESS) {
        std::cout << "Disable sink failed, ret: " << ret << std::endl;
        return;
    }
    std::cout << "Disable sink success." << std::endl;
}

static void DisableSource()
{
    SetNativeToken();
    if (g_dhFwkKit == nullptr) {
        std::cout << "Distributed hardware fwk kit is null." << std::endl;
        return;
    }

    std::vector<DmDeviceInfo> deviceList;
    std::shared_ptr<DmInitCallback> callBack = std::make_shared<TestDmInitCallback>();
    DeviceManager::GetInstance().InitDeviceManager("ohos.dhardware.test", callBack);
    DeviceManager::GetInstance().GetTrustedDeviceList("ohos.dhardware.test", "", deviceList);
    DeviceManager::GetInstance().UnInitDeviceManager("ohos.dhardware.test");

    if (deviceList.size() == 0) {
        printf("No networking devices\n");
        return;
    }
    // 去使能所有分布式硬件
    for (auto &device : deviceList) {
        // 获取对端分布式硬件能力
        std::vector<DHDescriptor> descriptors;
        TestGetDistributedHardwareCallback *lpCallback = new TestGetDistributedHardwareCallback();
        sptr<IGetDhDescriptorsCallback> callback(lpCallback);
        g_dhFwkKit->GetDistributedHardware(device.networkId, EnableStep::DISABLE_SOURCE, callback);
        lpCallback->WaitForResult();
        descriptors = lpCallback->GetDescriptors();
        printf("start DisableSource task.\n");
        g_dhFwkKit->DisableSource(device.networkId, descriptors);
    }
    std::cout << "Disable source success." << std::endl;
}

static void HandleEvent(const int32_t cmd)
{
    switch (cmd) {
        case CMD_ENABLE_SINK:
            EnableSink();
            break;
        case CMD_ENABLE_SOURCE:
            EnableSource();
            break;
        case CMD_DISABLE_SINK:
            DisableSink();
            break;
        case CMD_DISABLE_SOURCE:
            DisableSource();
            break;
        case CMD_INPUT_ENABLE_FIRST_TOKENID:
            InputEnableFirstTokenId();
            break;
        default:
            std::cout << "Unknown operation." << std::endl;
            break;
    }
}

static void PrintInteractiveUsage()
{
    std::cout << std::endl << "=============== InteractiveRunTestSelect ================" << std::endl;
    std::cout << "You can respond to instructions for corresponding option:" << std::endl;
    std::cout <<  "\t enter 1 to enable sink. " << std::endl;
    std::cout <<  "\t enter 2 to enable source. " << std::endl;
    std::cout <<  "\t enter 3 to disable sink. " << std::endl;
    std::cout <<  "\t enter 4 to disable source. " << std::endl;
    std::cout <<  "\t enter 5 to input enableFirstTokenId. " << std::endl;
    std::cout <<  "\t enter 0 to exit. " << std::endl;
}
}

int main(int argc, char *argv[])
{
    if (InitTestDemo() != DH_FWK_SUCCESS) {
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    bool quit = false;
    while (!quit) {
        PrintInteractiveUsage();
        int32_t cmd = GetUserInput();
        if (cmd == CMD_QUIT) {
            quit = true;   // 退出循环
        } else {
            HandleEvent(cmd);
        }
    }
    return 0;
}
