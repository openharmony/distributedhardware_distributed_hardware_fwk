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

#ifndef PIPELINE_TEST_H
#define PIPELINE_TEST_H

#include <string>
#include <vector>
#include <memory>
#include <gtest/gtest.h>
#include "osal/task/mutex.h"

#include "pipeline_status.h"
#include "gmock/gmock.h"
#include "filter.h"
#include "pipeline/include/pipeline.h"

namespace OHOS {
namespace DistributedHardware {
namespace Pipeline {

class PipelineTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};
} // namespace Pipeline
} // namespace DistributedHardware
} // namespace OHOS

#endif // PIPELINE_TEST_H