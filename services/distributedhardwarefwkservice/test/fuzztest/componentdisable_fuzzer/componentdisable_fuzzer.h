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

#ifndef TEST_COMPONENTDISABLE_FUZZER_H
#define TEST_COMPONENTDISABLE_FUZZER_H

#define FUZZ_PROJECT_NAME "componentdisable_fuzzer"

#include <string>
#include "idistributed_hardware_source.h"

namespace OHOS {
namespace DistributedHardware {

class FuzzDistributedHardwareSource : public IDistributedHardwareSource {
public:
    FuzzDistributedHardwareSource() = default;
    virtual ~FuzzDistributedHardwareSource() = default;

    int32_t InitSource(const std::string& networkId) override;
    int32_t ReleaseSource() override;
    int32_t RegisterDistributedHardware(const std::string& networkId, const std::string& dhId, const EnableParam& param,
        std::shared_ptr<RegisterCallback> callback) override;
    int32_t UnregisterDistributedHardware(const std::string& networkId, const std::string& dhId,
        std::shared_ptr<UnregisterCallback> callback) override;
    int32_t ConfigDistributedHardware(const std::string& networkId, const std::string& dhId, const std::string& key,
        const std::string& value) override;
    void RegisterDistributedHardwareStateListener(std::shared_ptr<DistributedHardwareStateListener> listener) override;
    void UnregisterDistributedHardwareStateListener() override{}
    void RegisterDataSyncTriggerListener(std::shared_ptr<DataSyncTriggerListener> listener) override;
    void UnregisterDataSyncTriggerListener() override{}
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
