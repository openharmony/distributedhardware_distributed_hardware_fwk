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

#ifndef FILTER_TEST_H
#define FILTER_TEST_H

#include <gtest/gtest.h>

#include "filter.h"
#include "buffer/avbuffer_queue_producer.h"
#include "meta/meta.h"
#include "osal/task/condition_variable.h"
#include "osal/task/mutex.h"
#include "osal/task/task.h"

#include "pipeline_event.h"
#include "pipeline_status.h"

namespace OHOS {
namespace DistributedHardware {

using namespace Pipeline;

class FilterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};
} // namespace DistributedHardware
} // namespace OHOS

#endif // FILTER_TEST_H