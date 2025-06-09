/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "av_trans_control_center_callback_stub_test.h"
#include "av_trans_errno.h"
#include "av_trans_control_center_callback_proxy.h"

using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
void AVTransControlCenterCallbackStubTest::SetUpTestCase(void)
{
}

void AVTransControlCenterCallbackStubTest::TearDownTestCase(void)
{
}

void AVTransControlCenterCallbackStubTest::SetUp()
{
}

void AVTransControlCenterCallbackStubTest::TearDown()
{
}

int32_t AVTransControlCenterCallbackStubTest::TestControlCenterCallbackStub::SetParameter(
    uint32_t tag, const std::string &value)
{
    (void) tag;
    (void) value;
    return DH_AVT_SUCCESS;
}
int32_t AVTransControlCenterCallbackStubTest::TestControlCenterCallbackStub::SetSharedMemory(
    const AVTransSharedMemoryExt &memory)
{
    (void) memory;
    return DH_AVT_SUCCESS;
}
int32_t AVTransControlCenterCallbackStubTest::TestControlCenterCallbackStub::Notify(
    const AVTransEventExt &event)
{
    (void) event;
    return DH_AVT_SUCCESS;
}

/**
 * @tc.name: set_parameter_001
 * @tc.desc: set parameter function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterCallbackStubTest, set_parameter_001, TestSize.Level0)
{
    sptr<IRemoteObject> CallbackStubPtr(new TestControlCenterCallbackStub());
    AvTransControlCenterCallbackProxy callbackProxy(CallbackStubPtr);

    AVTransTag tag = AVTransTag::STOP_AV_SYNC;
    std::string value = "value";
    int32_t ret = callbackProxy.SetParameter((uint32_t)tag, value);
    EXPECT_EQ(NO_ERROR, ret);
}

/**
 * @tc.name: set_shared_memory_001
 * @tc.desc: set shared memory function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterCallbackStubTest, set_shared_memory_001, TestSize.Level1)
{
    sptr<IRemoteObject> CallbackStubPtr(new TestControlCenterCallbackStub());
    AvTransControlCenterCallbackProxy callbackProxy(CallbackStubPtr);

    AVTransSharedMemoryExt memory;
    memory.name = "AVTransSharedMemory";
    int32_t ret = callbackProxy.SetSharedMemory(memory);
    EXPECT_EQ(NO_ERROR, ret);
}

/**
 * @tc.name: notify_001
 * @tc.desc: notify function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterCallbackStubTest, notify_001, TestSize.Level0)
{
    sptr<IRemoteObject> CallbackStubPtr(new TestControlCenterCallbackStub());
    AvTransControlCenterCallbackProxy callbackProxy(CallbackStubPtr);

    AVTransEventExt event;
    event.content = "content";
    int32_t ret = callbackProxy.Notify(event);
    EXPECT_EQ(NO_ERROR, ret);
}
} // namespace DistributedHardware
} // namespace OHOS