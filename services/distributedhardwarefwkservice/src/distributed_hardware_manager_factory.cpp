/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "distributed_hardware_manager_factory.h"

#include <dlfcn.h>
#include <memory>

#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
const char* LIB_NAME = "libdistributedhardwarefwksvr_impl.z.so";
const std::string FUNC_GET_INSTANCE = "GetDistributedHardwareManagerInstance";

#undef DH_LOG_TAG
#define DH_LOG_TAG "DistributedHardwareManagerFactory"

using GetMgrFunc = IDistributedHardwareManager *(*)();

IMPLEMENT_SINGLE_INSTANCE(DistributedHardwareManagerFactory);

bool DistributedHardwareManagerFactory::Init()
{
    DHLOGI("start");

    auto loadResult = LoadLibrary();
    if (loadResult != DH_FWK_SUCCESS) {
        DHLOGE("LoadLibrary failed, errCode = %d", loadResult);
        return false;
    }

    auto initResult = distributedHardwareMgrPtr_->Initialize();
    if (initResult != DH_FWK_SUCCESS) {
        DHLOGE("Initialize failed, errCode = %d", initResult);
        return false;
    }
    DHLOGD("success");

    return true;
}

void DistributedHardwareManagerFactory::UnInit()
{
    DHLOGI("start");

    // release all the resources synchronously
    distributedHardwareMgrPtr_->Release();

    CloseLibrary();
    DHLOGD("success");
}

bool DistributedHardwareManagerFactory::IsInit()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (distributedHardwareMgrPtr_ == nullptr) {
        DHLOGE("distributedHardwareMgr is not Initialize");
        return false;
    }
    return true;
}

int32_t DistributedHardwareManagerFactory::SendOnLineEvent(const std::string &networkId, const std::string &uuid,
    uint16_t deviceType)
{
    if (networkId.empty()) {
        DHLOGE("networkId is empty");
        return ERR_DH_FWK_REMOTE_NETWORK_ID_IS_EMPTY;
    }

    if (uuid.empty()) {
        DHLOGE("uuid is empty");
        return ERR_DH_FWK_REMOTE_DEVICE_ID_IS_EMPTY;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (distributedHardwareMgrPtr_ == nullptr && !Init()) {
        DHLOGE("distributedHardwareMgr is null");
        return ERR_DH_FWK_HARDWARE_MANAGER_LOAD_IMPL_FAILED;
    }

    auto onlineResult = distributedHardwareMgrPtr_->SendOnLineEvent(networkId, uuid, deviceType);
    if (onlineResult != DH_FWK_SUCCESS) {
        DHLOGE("online failed, errCode = %d", onlineResult);
        return onlineResult;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareManagerFactory::SendOffLineEvent(const std::string &networkId, const std::string &uuid,
    uint16_t deviceType)
{
    if (networkId.empty()) {
        DHLOGE("networkId is empty");
        return ERR_DH_FWK_REMOTE_NETWORK_ID_IS_EMPTY;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (distributedHardwareMgrPtr_ == nullptr) {
        DHLOGE("distributedHardwareMgr is null");
        return ERR_DH_FWK_HARDWARE_MANAGER_LOAD_IMPL_FAILED;
    }

    auto offlineResult = distributedHardwareMgrPtr_->SendOffLineEvent(networkId, uuid, deviceType);
    if (offlineResult != DH_FWK_SUCCESS) {
        DHLOGE("offline failed, errCode = %d", offlineResult);
        return offlineResult;
    }

    if (distributedHardwareMgrPtr_->GetOnLineCount() == 0) {
        DHLOGI("all devices are offline, start to free the resource");
        UnInit();
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareManagerFactory::LoadLibrary()
{
    DHLOGI("start.");
    if (handler_ != nullptr && distributedHardwareMgrPtr_ != nullptr) {
        DHLOGE("DistributedHardwareManager handler has loaded.");
        return DH_FWK_SUCCESS;
    }

    handler_ = dlopen(LIB_NAME, RTLD_NOW | RTLD_NODELETE);
    if (handler_ == nullptr) {
        DHLOGE("open %s failed.", LIB_NAME);
        return ERR_DH_FWK_HARDWARE_MANAGER_LIB_IMPL_OPEN_FAILED;
    }

    auto getManager = reinterpret_cast<GetMgrFunc>(dlsym(handler_, FUNC_GET_INSTANCE.c_str()));
    if (getManager == nullptr) {
        DHLOGE("can not find %s.", FUNC_GET_INSTANCE.c_str());
        CloseLibrary();
        return ERR_DH_FWK_HARDWARE_MANAGER_LIB_IMPL_GET_INSTANCE_FAILED;
    }

    distributedHardwareMgrPtr_ = getManager();
    if (distributedHardwareMgrPtr_ == nullptr) {
        DHLOGE("distributedHardwareMgrPtr is null.");
        CloseLibrary();
        return ERR_DH_FWK_HARDWARE_MANAGER_LIB_IMPL_IS_NULL;
    }
    DHLOGI("load %s success.", LIB_NAME);
    return DH_FWK_SUCCESS;
}

void DistributedHardwareManagerFactory::CloseLibrary()
{
    if (handler_ == nullptr) {
        DHLOGI("%s is already closed.", LIB_NAME);
        return;
    }
    distributedHardwareMgrPtr_ = nullptr;
    dlclose(handler_);
    handler_ = nullptr;
    DHLOGI("%s is closed.", LIB_NAME);
}

int32_t DistributedHardwareManagerFactory::GetComponentVersion(std::unordered_map<DHType, std::string> &versionMap)
{
    DHLOGI("start");
    std::lock_guard<std::mutex> lock(mutex_);
    if (distributedHardwareMgrPtr_ == nullptr) {
        DHLOGE("distributedHardwareMgr is null");
        return ERR_DH_FWK_HARDWARE_MANAGER_LIB_IMPL_IS_NULL;
    }
    return distributedHardwareMgrPtr_->GetComponentVersion(versionMap);
}
} // namespace DistributedHardware
} // namespace OHOS
