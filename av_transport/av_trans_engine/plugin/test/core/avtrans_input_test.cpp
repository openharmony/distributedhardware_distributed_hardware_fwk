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

#include "avtrans_input_test.h"

namespace OHOS {
namespace DistributedHardware {
void AvTransInputTest::SetUpTestCase() {}

void AvTransInputTest::TearDownTestCase() {}

void AvTransInputTest::SetUp()
{
    uint32_t pkgVer = 2;
    uint32_t apiVer = 2;
    std::string name = "name";
    std::shared_ptr<AvTransInputPlugin> plugin = std::make_shared<AvTransInputPluginTest>(name);
    input_ = std::make_shared<AvTransInput>(pkgVer, apiVer, plugin);
}

void AvTransInputTest::TearDown() {}

HWTEST_F(AvTransInputTest, Pause_001, TestSize.Level0)
{
    Status ret = input_->Pause();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransInputTest, Resume_001, TestSize.Level0)
{
    Status ret = input_->Resume();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransInputTest, PushData_001, TestSize.Level0)
{
    std::string inPort = "inPort";
    std::shared_ptr<Plugin::Buffer> buffer = nullptr;
    int32_t offset = 0;
    Status ret = input_->PushData(inPort, buffer, offset);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransInputTest, SetDataCallback_001, TestSize.Level0)
{
    std::function<void(std::shared_ptr<Plugin::Buffer>)> callback = nullptr;
    Status ret = input_->SetDataCallback(callback);
    EXPECT_EQ(Status::OK, ret);
}
} // namespace DistributedHardware
} // namespace OHOS