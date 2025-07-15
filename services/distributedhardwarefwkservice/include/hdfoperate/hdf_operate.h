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

#ifndef OHOS_HDF_OPERATE_H
#define OHOS_HDF_OPERATE_H

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>

#include "idistributed_hardware.h"
#include "idistributed_hardware_source.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class HdfLoadRefRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit HdfLoadRefRecipient(DHType dhType) : dhType_(dhType) {}
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
private:
    DHType dhType_;
};

class HdfDeathCallbackImpl : public HdfDeathCallback {
public:
    explicit HdfDeathCallbackImpl(DHType dhType) : dhType_(dhType) {}
    virtual ~HdfDeathCallbackImpl() = default;
protected:
    void OnHdfHostDied();
private:
    DHType dhType_;
};
class HdfOperator {
public:
    explicit HdfOperator(DHType dhType, IDistributedHardwareSource *sourcePtr)
        : dhType_(dhType), sourcePtr_(sourcePtr)
    {
        hdfDeathCallback_ = std::make_shared<HdfDeathCallbackImpl>(dhType);
    }
    int32_t LoadDistributedHDF();
    int32_t UnLoadDistributedHDF();
    void ResetRefCount();
    bool IsNeedErase();
private:
    DHType dhType_ = DHType::UNKNOWN;
    int32_t hdfLoadRef_ = 0;
    std::mutex hdfLoadRefMutex_;
    IDistributedHardwareSource *sourcePtr_ = nullptr;
    std::shared_ptr<HdfDeathCallback> hdfDeathCallback_ = nullptr;
};

class HdfOperateManager {
DECLARE_SINGLE_INSTANCE(HdfOperateManager);
public:
    int32_t LoadDistributedHDF(DHType dhType);
    int32_t UnLoadDistributedHDF(DHType dhType);
    int32_t AddDeathRecipient(DHType dhType, sptr<IRemoteObject> &remote);
    int32_t RemoveDeathRecipient(DHType dhType, sptr<IRemoteObject> &remote);
    void ResetRefCount(DHType dhType);
    int32_t RigidGetSourcePtr(DHType dhType, IDistributedHardwareSource *&sourcePtr);
    int32_t RigidReleaseSourcePtr(DHType dhType);
    bool IsAnyHdfInuse();

private:
    struct SourceHandlerData {
        int32_t refCount;
        void *sourceHandler;
        IDistributedHardwareSource *sourcePtr;
    };

private:
    std::mutex hdfOperateMapMutex_;
    std::map<DHType, std::shared_ptr<HdfOperator>> hdfOperateMap_;
    sptr<HdfLoadRefRecipient> audioHdfLoadRefRecipient_ = sptr(new HdfLoadRefRecipient(DHType::AUDIO));
    sptr<HdfLoadRefRecipient> cameraHdfLoadRefRecipient_ = sptr(new HdfLoadRefRecipient(DHType::CAMERA));
    std::mutex sourceHandlerDataMapMutex_;
    std::map<DHType, SourceHandlerData> sourceHandlerDataMap_;
    int32_t hdfInuseRefCount_ = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_HDF_OPERATE_H