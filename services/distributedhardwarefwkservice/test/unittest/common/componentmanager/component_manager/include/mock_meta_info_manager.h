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

#ifndef OHOS_DISTRIBUTED_HARDWARE_MOCK_META_INFO_MANAGER_H
#define OHOS_DISTRIBUTED_HARDWARE_MOCK_META_INFO_MANAGER_H

#include <gmock/gmock.h>

#include "meta_info_manager.h"

namespace OHOS {
namespace DistributedHardware {
class IMetaInfoManager {
public:
    virtual ~IMetaInfoManager() = default;

    virtual int32_t GetMetaDataByDHType(const DHType dhType, MetaCapInfoMap &metaInfoMap);
    virtual int32_t GetMetaCapInfo(const std::string &udidHash,
        const std::string &dhId, std::shared_ptr<MetaCapabilityInfo> &metaCapPtr);

    static std::shared_ptr<IMetaInfoManager> GetOrCtreateInstance();
    static void ReleaseInstance();
private:
    static std::shared_ptr<IMetaInfoManager> metaInfoManagerInstance_;
};

class MockMetaInfoManager : public IMetaInfoManager {
public:
    MOCK_METHOD(int32_t, GetMetaDataByDHType, (const DHType, MetaCapInfoMap &));
    MOCK_METHOD(int32_t, GetMetaCapInfo,
        (const std::string &, const std::string &, std::shared_ptr<MetaCapabilityInfo> &));
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_HARDWARE_MOCK_META_INFO_MANAGER_H
