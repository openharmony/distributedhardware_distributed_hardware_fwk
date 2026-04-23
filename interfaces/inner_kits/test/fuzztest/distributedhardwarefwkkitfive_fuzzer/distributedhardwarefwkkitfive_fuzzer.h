/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef TEST_DISTRIBUTEDHARDWAREFWK_KIT_FIVE_FUZZER_H
#define TEST_DISTRIBUTEDHARDWAREFWK_KIT_FIVE_FUZZER_H

#define FUZZ_PROJECT_NAME "distributedhardwarefwkkitfive_fuzzer"

#include <string>
#include "hardware_status_listener_stub.h"
#include "iav_trans_control_center_callback.h"

namespace OHOS {
namespace DistributedHardware {
class TestHDSinkStatusListener : public HDSinkStatusListenerStub {
public:
    TestHDSinkStatusListener() = default;
    virtual ~TestHDSinkStatusListener() = default;
protected:
    void OnEnable(const DHDescriptor &dhDescriptor) override;
    void OnDisable(const DHDescriptor &dhDescriptor) override;
};

class TestAVTransControlCenterCallback : public IAvTransControlCenterCallback {
public:
    TestAVTransControlCenterCallback() = default;
    virtual ~TestAVTransControlCenterCallback() = default;
protected:
    int32_t SetParameter(uint32_t tag, const std::string &value) override;
    int32_t Notify(const AVTransEventExt& event) override;
    sptr<IRemoteObject> AsObject() override
    {
    return nullptr;
    }
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
