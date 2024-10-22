/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"

#include "dh_modem_context_ext.h"
#include "distributed_hardware_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {

class DHModemContextExtTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DHModemContextExtTest::SetUp() {}

void DHModemContextExtTest::TearDown() {}

void DHModemContextExtTest::SetUpTestCase() {}

void DHModemContextExtTest::TearDownTestCase() {}

#ifdef DHARDWARE_OPEN_MODEM_EXT
    HWTEST_F(DHModemContextExtTest, GetHandler_001, TestSize.Level1)
    {
        int32_t ret = DHModemContextExt::GetInstance().GetHandler();
        EXPECT_EQ(ret, DH_FWK_SUCCESS);
    }

    HWTEST_F(DHModemContextExtTest, GetModemExtInstance_001, TestSize.Level1)
    {
        DHModemContextExt::GetInstance().soHandle_ = nullptr;
        DHModemContextExt::GetInstance().GetModemExtInstance();
        EXPECT_NE(DHModemContextExt::GetInstance().distributedModemExt_, nullptr);
    }

    HWTEST_F(DHModemContextExtTest, GetModemExtInstance_002, TestSize.Level1)
    {
        int32_t ret = DHModemContextExt::GetInstance().GetHandler();
        EXPECT_EQ(ret, DH_FWK_SUCCESS);
        DHModemContextExt::GetInstance().GetModemExtInstance();
        EXPECT_NE(DHModemContextExt::GetInstance().distributedModemExt_, nullptr);
    }

    HWTEST_F(DHModemContextExtTest, UnInit_001, TestSize.Level1)
    {
        DHModemContextExt::GetInstance().soHandle_ = nullptr;
        int32_t ret = DHModemContextExt::GetInstance().UnInit();
        EXPECT_EQ(ret, ERR_DH_FWK_LOADER_HANDLER_IS_NULL);
    }

    HWTEST_F(DHModemContextExtTest, UnInit_002, TestSize.Level1)
    {
        int32_t ret = DHModemContextExt::GetInstance().GetHandler();
        EXPECT_EQ(ret, DH_FWK_SUCCESS);
        ret = DHModemContextExt::GetInstance().UnInit();
        EXPECT_EQ(ret, DH_FWK_SUCCESS);
    }
#endif
}
}
}