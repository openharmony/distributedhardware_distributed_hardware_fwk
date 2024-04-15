/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_HISTREAMER_ABILITY_QUERIER_TEST_H
#define OHOS_DISTRIBUTED_HARDWARE_HISTREAMER_ABILITY_QUERIER_TEST_H

#include "cJSON.h"
#include <gtest/gtest.h>

namespace OHOS {
namespace DistributedHardware {
class HistreamerAbilityQuerierTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
bool IsUInt8(const cJSON *jsonObj, const std::string& key);

bool IsUInt32(const cJSON *jsonObj, const std::string& key);

bool IsString(const cJSON *jsonObj, const std::string& key);
} // namespace DistributedHardware
} // namespace OHOS
#endif