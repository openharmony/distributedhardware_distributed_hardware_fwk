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

#include "event_trance_test.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void EventTraceTest::SetUpTestCase(void)
{
}

void EventTraceTest::TearDownTestCase(void)
{
}

void EventTraceTest::SetUp()
{
}

void EventTraceTest::TearDown()
{
}

/**
 * @tc.name: HiSysEventWriteReleaseMsg_001
 * @tc.desc: Verify the HiSysEventWriteReleaseMsg function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(EventTraceTest, HiSysEventWriteReleaseMsg_001, TestSize.Level0)
{
    std::string status;
    int32_t errCode = 0;
    std::string msg;
    HiSysEventWriteErrCodeMsg(status, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, errCode, msg);
    EXPECT_EQ(true, msg.empty());
}

/**
 * @tc.name: DHCompMgrTraceStart_001
 * @tc.desc: Verify the DHCompMgrTraceStart function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK0
 */
HWTEST_F(EventTraceTest, DHCompMgrTraceStart_001, TestSize.Level0)
{
    std::string anonyNetworkId;
    std::string anonyDHId;
    std::string msg;
    DHCompMgrTraceStart(anonyNetworkId, anonyDHId, msg);
    EXPECT_EQ(true, msg.empty());
}
} // namespace DistributedHardware
} // namespace OHOS
