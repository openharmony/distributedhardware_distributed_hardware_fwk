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

#include "mock_meta_info_manager.h"

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<IMetaInfoManager> IMetaInfoManager::metaInfoManagerInstance_;

std::shared_ptr<IMetaInfoManager> IMetaInfoManager::GetOrCreateInstance()
{
    if (!metaInfoManagerInstance_) {
        metaInfoManagerInstance_ = std::make_shared<MockMetaInfoManager>();
    }
    return metaInfoManagerInstance_;
}

void IMetaInfoManager::ReleaseInstance()
{
    metaInfoManagerInstance_.reset();
    metaInfoManagerInstance_ = nullptr;
}

int32_t MetaInfoManager::GetMetaDataByDHType(const DHType dhType, MetaCapInfoMap &metaInfoMap)
{
    return IMetaInfoManager::GetOrCreateInstance()->GetMetaDataByDHType(dhType, metaInfoMap);
}

int32_t MetaInfoManager::GetMetaCapInfo(const std::string &udidHash,
    const std::string &dhId, std::shared_ptr<MetaCapabilityInfo> &metaCapPtr)
{
    return IMetaInfoManager::GetOrCreateInstance()->GetMetaCapInfo(udidHash, dhId, metaCapPtr);
}
} // namespace DistributedHardware
} // namespace OHOS
