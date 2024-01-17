/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_VERSION_INFO_MANAGER_TEST_H
#define OHOS_DISTRIBUTED_HARDWARE_VERSION_INFO_MANAGER_TEST_H

#include <gtest/gtest.h>
#include <vector>

#include "db_adapter.h"
#include "distributed_hardware_errno.h"
#include "version_info.h"

namespace OHOS {
namespace DistributedHardware {
class VersionInfoManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class MockDBAdapter : public DBAdapter {
public:
MockDBAdapter(const std::string &appId, const std::string &storeId,
    const std::shared_ptr<DistributedKv::KvStoreObserver> &changeListener)
    : DBAdapter(appId, storeId, changeListener) {}

int32_t RemoveCapabilityInfoByKey(const std::string &key)
{
    (void)key;
    return DH_FWK_SUCCESS;
}

int32_t GetDataByKey(const std::string &key, std::string &data)
{
    (void)key;
    (void)data;
    return DH_FWK_SUCCESS;
}

int32_t RemoveDataByKey(const std::string &key)
{
    (void)key;
    return DH_FWK_SUCCESS;
}

int32_t GetDataByKeyPrefix(const std::string &keyPrefix, std::vector<std::string> &values)
{
    (void)keyPrefix;
    (void)values;
    return DH_FWK_SUCCESS;
}
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
