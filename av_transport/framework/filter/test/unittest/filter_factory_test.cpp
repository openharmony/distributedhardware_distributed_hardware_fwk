/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "filter.h"
#include "filter_factory_test.h"
#include "cpp_ext/type_cast_ext.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware;
using namespace std;

namespace OHOS {
namespace DistributedHardware {

void FilterFactoryTest::SetUp()
{
}

void FilterFactoryTest::TearDown()
{
}

void FilterFactoryTest::SetUpTestCase()
{
}

void FilterFactoryTest::TearDownTestCase()
{
}
HWTEST_F(FilterFactoryTest, CreateFilterPriv_001, testing::ext::TestSize.Level1)
{
    auto& filterFactory = Pipeline::FilterFactory::Instance();
    std::string registeredFilterName = "registeredFilterOut";
    Pipeline::FilterType registeredType = Pipeline::FilterType::FILTERTYPE_SOURCE;
    std::shared_ptr<Pipeline::Filter> registeredFilter =
        filterFactory.CreateFilterPriv(registeredFilterName, registeredType);
    EXPECT_EQ(registeredFilter, nullptr);
}
} // namespace DistributedHardware
} // namespace OHOS