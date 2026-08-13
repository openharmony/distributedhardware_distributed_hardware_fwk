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

#include "get_dh_descriptors_callback_proxy.h"
#include "constants.h"
#include "distributed_hardware_errno.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "iremote_object.h"
#include "refbase.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS;
using namespace OHOS::DistributedHardware;

class MockRemoteObjectForGetDh : public IRemoteObject {
public:
    MockRemoteObjectForGetDh() : IRemoteObject(u"") {}
    MOCK_METHOD4(SendRequest, int32_t(uint32_t, MessageParcel&, MessageParcel&, MessageOption&));
    MOCK_METHOD0(GetObjectRefPtr, sptr<IRemoteObject>());
    MOCK_METHOD0(GetObjectRefCount, int32_t());
    MOCK_METHOD2(Dump, int(int fd, const std::vector<std::u16string>& args));
    MOCK_METHOD1(AddDeathRecipient, bool(const sptr<IRemoteObject::DeathRecipient>&));
    MOCK_METHOD1(RemoveDeathRecipient, bool(const sptr<IRemoteObject::DeathRecipient>&));
};

class GetDhDescriptorsCallbackProxyTest : public testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(GetDhDescriptorsCallbackProxyTest, test_write_descriptors_success, TestSize.Level1)
{
    MessageParcel data;
    std::vector<DHDescriptor> descriptors;
    descriptors.push_back({.id = "dh_id_1", .dhType = DHType::CAMERA});
    descriptors.push_back({.id = "dh_id_2", .dhType = DHType::CAMERA});
    sptr<MockRemoteObjectForGetDh> remote = new MockRemoteObjectForGetDh();
    auto proxy = std::make_shared<GetDhDescriptorsCallbackProxy>(remote);
    int32_t ret = proxy->WriteDescriptors(data, descriptors);
    EXPECT_EQ(ret, NO_ERROR);
}

HWTEST_F(GetDhDescriptorsCallbackProxyTest, test_write_descriptors_over_size, TestSize.Level1)
{
    MessageParcel data;
    std::vector<DHDescriptor> descriptors;
    for (uint32_t i = 0; i < MAX_DH_DESCRIPTOR_ARRAY_SIZE + 1; i++) {
        descriptors.push_back({.id = "dh_id_" + std::to_string(i), .dhType = DHType::CAMERA});
    }
    sptr<MockRemoteObjectForGetDh> remote = new MockRemoteObjectForGetDh();
    auto proxy = std::make_shared<GetDhDescriptorsCallbackProxy>(remote);
    int32_t ret = proxy->WriteDescriptors(data, descriptors);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
}

HWTEST_F(GetDhDescriptorsCallbackProxyTest, test_write_descriptors_empty, TestSize.Level1)
{
    MessageParcel data;
    std::vector<DHDescriptor> descriptors;
    sptr<MockRemoteObjectForGetDh> remote = new MockRemoteObjectForGetDh();
    auto proxy = std::make_shared<GetDhDescriptorsCallbackProxy>(remote);
    int32_t ret = proxy->WriteDescriptors(data, descriptors);
    EXPECT_EQ(ret, NO_ERROR);
}