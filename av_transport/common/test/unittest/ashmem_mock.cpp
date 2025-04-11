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

#include "ashmem_mock.h"

namespace OHOS {
std::shared_ptr<IAshmem> IAshmem::ashmem_;

constexpr int32_t ASH_MEM_SIZE = 100;
constexpr int32_t FD_FOR_TEST = 50;
constexpr int32_t ASH_MEM_SET_PORTT = 1;

std::shared_ptr<IAshmem> IAshmem::GetOrCreateInstance()
{
    if (!ashmem_) {
        ashmem_ = std::make_shared<MockAshmem>();
    }
    return ashmem_;
}

void IAshmem::ReleaseInstance()
{
    ashmem_.reset();
    ashmem_ = nullptr;
}

int AshmemGetSize(int fd)
{
    if (fd >= FD_FOR_TEST) {
        return fd;
    }
    return ASH_MEM_SIZE;
}
int AshmemSetProt(int fd, int prot)
{
    return ASH_MEM_SET_PORTT;
}
                                                        
} // namespace OHOS
