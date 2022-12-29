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

#ifndef OHOS_PUBLISHER_ITEM_TEST_H
#define OHOS_PUBLISHER_ITEM_TEST_H

#include <gtest/gtest.h>
#include <iremote_broker.h>

#define private public
#include "publisher_item.h"
#undef private

namespace OHOS {
namespace DistributedHardware {
class PublisherItemTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class MockIPublisherListener : public IPublisherListener {
public:
sptr<IRemoteObject> AsObject()
{
    return nullptr;
}

void OnMessage(const DHTopic topic, const std::string& message)
{
    (void)topic;
    (void)message;
}
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
