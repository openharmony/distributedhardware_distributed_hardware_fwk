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

#ifndef OHOS_DISTRIBUTED_HARDWARE_ASHMEM_MOCK
#define OHOS_DISTRIBUTED_HARDWARE_ASHMEM_MOCK

#include <gmock/gmock.h>

#include "ashmem.h"

namespace OHOS {
class IAshmem {
public:
    static std::shared_ptr<IAshmem> GetOrCreateInstance();
    static void ReleaseInstance();
public:
    static std::shared_ptr<IAshmem> ashmem_;
};

class MockAshmem : public IAshmem {
public:
    virtual ~MockAshmem() = default;
};
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_HARDWARE_ASHMEM_MOCK