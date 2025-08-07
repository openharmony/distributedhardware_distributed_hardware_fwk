/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "hdf_operate.h"

#include <dlfcn.h>

#include "component_loader.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "task_board.h"
#include "task_executor.h"
#include "task_factory.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "HdfOperator"

namespace OHOS {
namespace DistributedHardware {
using GetSourceHardwareClass = IDistributedHardwareSource *(*)();

int32_t HdfOperator::LoadDistributedHDF()
{
    DHLOGI("Load hdf impl begin, dhType = %{public}#X!", dhType_);
    std::unique_lock<std::mutex> loadRefLocker(hdfLoadRefMutex_);
    if (hdfLoadRef_ > 0) {
        hdfLoadRef_++;
        DHLOGI("The hdf impl has been loaded, just inc ref, dhType = %{public}#X!", dhType_);
        return DH_FWK_SUCCESS;
    }
    auto ret = sourcePtr_->LoadDistributedHDF(hdfDeathCallback_);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Source handler load hdf failed, dhType = %{public}#X, ret = %{public}d.", dhType_, ret);
        return ret;
    }
    hdfLoadRef_++;
    DHLOGI("Load hdf impl end, dhType = %{public}#X!", dhType_);
    return DH_FWK_SUCCESS;
}

int32_t HdfOperator::UnLoadDistributedHDF()
{
    DHLOGI("UnLoad hdf impl begin, dhType = %{public}#X!", dhType_);
    std::unique_lock<std::mutex> loadRefLocker(hdfLoadRefMutex_);
    if (hdfLoadRef_ <= 0) {
        DHLOGI("The hdf impl has been unloaded, dhType = %{public}#X!", dhType_);
        return DH_FWK_SUCCESS;
    }
    if (hdfLoadRef_ > 1) {
        hdfLoadRef_--;
        DHLOGI("The hdf impl has been loaded, just dec ref, dhType = %{public}#X!", dhType_);
        return DH_FWK_SUCCESS;
    }
    auto ret = sourcePtr_->UnLoadDistributedHDF();
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Source handler unload hdf failed, dhType = %{public}#X, ret = %{public}d.", dhType_, ret);
        return ret;
    }
    hdfLoadRef_--;
    DHLOGI("UnLoad hdf impl end, dhType = %{public}#X!", dhType_);
    return DH_FWK_SUCCESS;
}

void HdfOperator::ResetRefCount()
{
    DHLOGI("Reset reference count, dhType = %{public}#X!", dhType_);
    std::unique_lock<std::mutex> loadRefLocker(hdfLoadRefMutex_);
    hdfLoadRef_ = 0;
}

bool HdfOperator::IsNeedErase()
{
    std::unique_lock<std::mutex> loadRefLocker(hdfLoadRefMutex_);
    return hdfLoadRef_ <= 0;
}

IMPLEMENT_SINGLE_INSTANCE(HdfOperateManager);

int32_t HdfOperateManager::LoadDistributedHDF(DHType dhType)
{
    DHLOGI("HdfOperateManager load hdf, dhType = %{public}#X!", dhType);
    std::unique_lock<std::mutex> hdfOperateMapLocker(hdfOperateMapMutex_);
    auto itHdfOperate = hdfOperateMap_.find(dhType);
    if (itHdfOperate == hdfOperateMap_.end()) {
        IDistributedHardwareSource *sourcePtr = nullptr;
        auto ret = ComponentLoader::GetInstance().GetSource(dhType, sourcePtr);
        if (ret != DH_FWK_SUCCESS) {
            DHLOGE("GetSource failed, compType = %{public}#X, ret = %{public}d.", dhType, ret);
            ret = RigidGetSourcePtr(dhType, sourcePtr);
            if (ret != DH_FWK_SUCCESS) {
                DHLOGE("RigidGetSourcePtr failed, compType = %{public}#X, ret = %{public}d.", dhType, ret);
                return ret;
            }
        }
        auto result = hdfOperateMap_.insert(
            std::pair<DHType, std::shared_ptr<HdfOperator>>(
                dhType, std::make_shared<HdfOperator>(dhType, sourcePtr)));
        itHdfOperate = result.first;
    }
    auto hdfOperate = itHdfOperate->second;
    if (hdfOperate == nullptr) {
        DHLOGE("Get hdf operator is nullptr, dhType = %{public}#X.", dhType);
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    auto ret = hdfOperate->LoadDistributedHDF();
    if (ret == DH_FWK_SUCCESS) {
        hdfInuseRefCount_++;
    }
    return ret;
}

int32_t HdfOperateManager::UnLoadDistributedHDF(DHType dhType)
{
    DHLOGI("HdfOperateManager unload hdf, dhType = %{public}#X!", dhType);
    std::unique_lock<std::mutex> hdfOperateMapLocker(hdfOperateMapMutex_);
    auto itHdfOperate = hdfOperateMap_.find(dhType);
    if (itHdfOperate == hdfOperateMap_.end()) {
        DHLOGI("The hdf operate has not been created yet, dhType = %{public}#X!", dhType);
        return DH_FWK_SUCCESS;
    }
    auto hdfOperate = itHdfOperate->second;
    if (hdfOperate == nullptr) {
        DHLOGE("Get hdf operator is nullptr, dhType = %{public}#X.", dhType);
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    auto ret = hdfOperate->UnLoadDistributedHDF();
    if (ret == DH_FWK_SUCCESS) {
        if (hdfOperate->IsNeedErase()) {
            hdfOperateMap_.erase(itHdfOperate);
        }
        if (RigidReleaseSourcePtr(dhType) != DH_FWK_SUCCESS) {
            DHLOGE("RigidReleaseSourcePtr failed, dhType = %{public}#X.", dhType);
        }
        hdfInuseRefCount_--;
        if (hdfInuseRefCount_ <= 0) {
            DHLOGI("hdf inuse refcount less than 0, create exit dfwk task!");
            TaskParam taskParam;
            auto task = TaskFactory::GetInstance().CreateTask(TaskType::EXIT_DFWK, taskParam, nullptr);
            TaskExecutor::GetInstance().PushTask(task);
        }
    }
    return ret;
}

int32_t HdfOperateManager::AddDeathRecipient(DHType dhType, sptr<IRemoteObject> &remote)
{
    if (remote == nullptr) {
        DHLOGE("remote ptr is null.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    DHLOGI("Add death recipient begin, dhType = %{public}#X!", dhType);
    bool ret = false;
    switch (dhType) {
        case DHType::AUDIO:
            ret = remote->AddDeathRecipient(audioHdfLoadRefRecipient_);
            break;
        case DHType::CAMERA:
            ret = remote->AddDeathRecipient(cameraHdfLoadRefRecipient_);
            break;
        default:
            DHLOGE("No hdf support, dhType = %{public}#X.", dhType);
            return ERR_DH_FWK_NO_HDF_SUPPORT;
    }
    if (ret == false) {
        DHLOGE("call AddDeathRecipient failed, dhType = %{public}#X.", dhType);
        return ERR_DH_FWK_ADD_DEATH_FAIL;
    }
    DHLOGI("Add death recipient end, dhType = %{public}#X!", dhType);
    return DH_FWK_SUCCESS;
}

int32_t HdfOperateManager::RemoveDeathRecipient(DHType dhType, sptr<IRemoteObject> &remote)
{
    if (remote == nullptr) {
        DHLOGE("remote ptr is null.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    DHLOGI("Remove death recipient begin, dhType = %{public}#X!", dhType);
    bool ret = false;
    switch (dhType) {
        case DHType::AUDIO:
            ret = remote->RemoveDeathRecipient(audioHdfLoadRefRecipient_);
            break;
        case DHType::CAMERA:
            ret = remote->RemoveDeathRecipient(cameraHdfLoadRefRecipient_);
            break;
        default:
            DHLOGE("No hdf support, dhType = %{public}#X.", dhType);
            return ERR_DH_FWK_NO_HDF_SUPPORT;
    }
    if (ret == false) {
        DHLOGE("call RemoveDeathRecipient failed, dhType = %{public}#X.", dhType);
        return ERR_DH_FWK_REMOVE_DEATH_FAIL;
    }
    DHLOGI("Remove death recipient end, dhType = %{public}#X!", dhType);
    return DH_FWK_SUCCESS;
}

void HdfOperateManager::ResetRefCount(DHType dhType)
{
    DHLOGI("Reset ref count begin, dhType = %{public}#X!", dhType);
    std::unique_lock<std::mutex> hdfOperateMapLocker(hdfOperateMapMutex_);
    auto itHdfOperate = hdfOperateMap_.find(dhType);
    if (itHdfOperate == hdfOperateMap_.end()) {
        DHLOGI("The hdf operate has not been created yet, dhType = %{public}#X!", dhType);
        return;
    }
    auto hdfOperate = itHdfOperate->second;
    hdfOperate->ResetRefCount();
    DHLOGI("reset ref count end, dhType = %{public}#X!", dhType);
}

int32_t HdfOperateManager::RigidGetSourcePtr(DHType dhType, IDistributedHardwareSource *&sourcePtr)
{
    DHLOGI("Rigid get source ptr begin, dhType = %{public}#X!", dhType);
    std::lock_guard<std::mutex> locker(sourceHandlerDataMapMutex_);
    auto itSourceHandlerData = sourceHandlerDataMap_.find(dhType);
    if (itSourceHandlerData != sourceHandlerDataMap_.end()) {
        auto &sourceHandlerData = itSourceHandlerData->second;
        sourcePtr = sourceHandlerData.sourcePtr;
        sourceHandlerData.refCount++;
        DHLOGI("Source ptr already exists, only increase reference count, dhType = %{public}#X!", dhType);
        return DH_FWK_SUCCESS;
    }
    std::string compSourceLoc;
    switch (dhType) {
        case DHType::AUDIO:
            compSourceLoc = "libdistributed_audio_source_sdk.z.so";
            break;
        case DHType::CAMERA:
            compSourceLoc = "libdistributed_camera_source_sdk.z.so";
            break;
        default:
            DHLOGE("No hdf support, dhType = %{public}#X.", dhType);
            return ERR_DH_FWK_NO_HDF_SUPPORT;
    }
    void *sourceHandler = dlopen(compSourceLoc.c_str(), RTLD_LAZY | RTLD_NODELETE);
    if (sourceHandler == nullptr) {
        DHLOGE("dlopen failed, failed reason: %{public}s, dhType: %{public}" PRIu32, dlerror(), (uint32_t)dhType);
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }
    auto getSourceHardClassHandler = (GetSourceHardwareClass)dlsym(
        sourceHandler, COMPONENT_LOADER_GET_SOURCE_HANDLER.c_str());
    if (getSourceHardClassHandler == nullptr) {
        DHLOGE("get getSourceHardClassHandler is null, failed reason : %{public}s", dlerror());
        dlclose(sourceHandler);
        sourceHandler = nullptr;
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }
    sourcePtr = getSourceHardClassHandler();
    if (sourcePtr == nullptr) {
        DHLOGE("getSourceHardClassHandler return null!");
        dlclose(sourceHandler);
        sourceHandler = nullptr;
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }
    sourceHandlerDataMap_[dhType] = SourceHandlerData {
        .refCount = 1, .sourceHandler = sourceHandler, .sourcePtr = sourcePtr
    };
    DHLOGI("Rigid get source ptr end, dhType = %{public}#X!", dhType);
    return DH_FWK_SUCCESS;
}

int32_t HdfOperateManager::RigidReleaseSourcePtr(DHType dhType)
{
    DHLOGI("Rigid release source ptr begin, dhType = %{public}#X!", dhType);
    std::lock_guard<std::mutex> locker(sourceHandlerDataMapMutex_);
    auto itSourceHandlerData = sourceHandlerDataMap_.find(dhType);
    if (itSourceHandlerData == sourceHandlerDataMap_.end()) {
        DHLOGI("The source ptr has not been obtained yet, dhType = %{public}#X!", dhType);
        return DH_FWK_SUCCESS;
    }
    auto &sourceHandlerData = itSourceHandlerData->second;
    if (sourceHandlerData.refCount > 1) {
        sourceHandlerData.refCount--;
        DHLOGI("Source ptr is still in use, only reducing reference count, dhType = %{public}#X!", dhType);
        return DH_FWK_SUCCESS;
    }
    if (dlclose(sourceHandlerData.sourceHandler) != 0) {
        DHLOGE("dlclose failed, failed reason: %{public}s, dhType: %{public}" PRIu32, dlerror(), (uint32_t)dhType);
        return ERR_DH_FWK_LOADER_DLCLOSE_FAIL;
    }
    sourceHandlerDataMap_.erase(itSourceHandlerData);
    DHLOGI("Rigid release source ptr end, dhType = %{public}#X!", dhType);
    return DH_FWK_SUCCESS;
}

bool HdfOperateManager::IsAnyHdfInuse()
{
    std::unique_lock<std::mutex> hdfOperateMapLocker(hdfOperateMapMutex_);
    return hdfInuseRefCount_ > 0;
}

void HdfLoadRefRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    DHLOGI("On remote died, dhType = %{public}#X!", dhType_);
    HdfOperateManager::GetInstance().UnLoadDistributedHDF(dhType_);
}

void HdfDeathCallbackImpl::OnHdfHostDied()
{
    DHLOGI("On hdf died, dhType = %{public}#X!", dhType_);
    HdfOperateManager::GetInstance().ResetRefCount(dhType_);
}
} // namespace DistributedHardware
} // namespace OHOS