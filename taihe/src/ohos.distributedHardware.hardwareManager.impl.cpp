/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "ohos.distributedHardware.hardwareManager.proj.hpp"
#include "ohos.distributedHardware.hardwareManager.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include <string>

#include "ipc_skeleton.h"
#include "device_type.h"
#include "distributed_hardware_log.h"
#include "distributed_hardware_fwk_kit.h"
#include "tokenid_kit.h"
#include "accesstoken_kit.h"

using namespace OHOS::DistributedHardware;
#undef DH_LOG_TAG
#define DH_LOG_TAG "DistributedHardwareManagerImpl"

namespace {
// To be implemented.

constexpr int32_t ERR_NO_PERMISSION = 201;
constexpr int32_t ERR_NOT_SYSTEM_APP = 202;

bool IsSystemApp()
{
    uint64_t tokenId = OHOS::IPCSkeleton::GetSelfTokenID();
    return OHOS::Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(tokenId);
}

bool HasAccessDHPermission()
{
    OHOS::Security::AccessToken::AccessTokenID callerToken = OHOS::IPCSkeleton::GetCallingTokenID();
    const std::string permissionName = "ohos.permission.ACCESS_DISTRIBUTED_HARDWARE";
    int32_t result = OHOS::Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken,
        permissionName);
    return (result == OHOS::Security::AccessToken::PERMISSION_GRANTED);
}

void PauseDistributedHardwareSync(::ohos::distributedHardware::hardwareManager::HardwareDescriptor const& description)
{
    DHLOGI("PauseDistributedHardware in");
    if (!IsSystemApp()) {
        taihe::set_business_error(ERR_NOT_SYSTEM_APP, "The caller is not a system application.");
        return;
    }
    if (!HasAccessDHPermission()) {
        taihe::set_business_error(ERR_NO_PERMISSION, "Permission verify failed.");
    }
    int32_t hardwareType = description.type;
    DHType dhType = DHType::UNKNOWN;
    DHSubtype dhSubtype = static_cast<DHSubtype>(hardwareType);
    if (dhSubtype == DHSubtype::AUDIO_MIC || dhSubtype == DHSubtype::AUDIO_SPEAKER) {
        dhType = DHType::AUDIO;
    } else if (dhSubtype == DHSubtype::CAMERA) {
        dhType = DHType::CAMERA;
    }
    std::string srcNetworkId;
    if (description.srcNetworkId.has_value()) {
        srcNetworkId = description.srcNetworkId.value();
    }

    std::shared_ptr<DistributedHardwareFwkKit> dhFwkKit = std::make_shared<DistributedHardwareFwkKit>();
    int32_t ret = dhFwkKit->PauseDistributedHardware(dhType, std::string(srcNetworkId));
    if (ret != 0) {
        DHLOGE("PauseDistributedHardware for DHType: %{public}u filed", (uint32_t)dhType);
    }
    return;
}

void ResumeDistributedHardwareSync(::ohos::distributedHardware::hardwareManager::HardwareDescriptor const& description)
{
    DHLOGI("ResumeDistributedHardware in");
    if (!IsSystemApp()) {
        taihe::set_business_error(ERR_NOT_SYSTEM_APP, "The caller is not a system application.");
        return;
    }
    if (!HasAccessDHPermission()) {
        taihe::set_business_error(ERR_NO_PERMISSION, "Permission verify failed.");
    }
    int32_t hardwareType = description.type;
    DHType dhType = DHType::UNKNOWN;
    DHSubtype dhSubtype = static_cast<DHSubtype>(hardwareType);
    if (dhSubtype == DHSubtype::AUDIO_MIC || dhSubtype == DHSubtype::AUDIO_SPEAKER) {
        dhType = DHType::AUDIO;
    } else if (dhSubtype == DHSubtype::CAMERA) {
        dhType = DHType::CAMERA;
    }
    std::string srcNetworkId;
    if (description.srcNetworkId.has_value()) {
        srcNetworkId = description.srcNetworkId.value();
    }

    std::shared_ptr<DistributedHardwareFwkKit> dhFwkKit = std::make_shared<DistributedHardwareFwkKit>();
    int32_t ret = dhFwkKit->ResumeDistributedHardware(dhType, std::string(srcNetworkId));
    if (ret != 0) {
        DHLOGE("ResumeDistributedHardware for DHType: %{public}u filed", (uint32_t)dhType);
    }
    return;
}

void StopDistributedHardwareSync(::ohos::distributedHardware::hardwareManager::HardwareDescriptor const& description)
{
    DHLOGI("StopDistributedHardware in");
    if (!IsSystemApp()) {
        taihe::set_business_error(ERR_NOT_SYSTEM_APP, "The caller is not a system application.");
        return;
    }
    if (!HasAccessDHPermission()) {
        taihe::set_business_error(ERR_NO_PERMISSION, "Permission verify failed.");
    }
    int32_t hardwareType = description.type;
    DHType dhType = DHType::UNKNOWN;
    DHSubtype dhSubtype = static_cast<DHSubtype>(hardwareType);
    if (dhSubtype == DHSubtype::AUDIO_MIC || dhSubtype == DHSubtype::AUDIO_SPEAKER) {
        dhType = DHType::AUDIO;
    } else if (dhSubtype == DHSubtype::CAMERA) {
        dhType = DHType::CAMERA;
    }
    std::string srcNetworkId;
    if (description.srcNetworkId.has_value()) {
        srcNetworkId = description.srcNetworkId.value();
    }

    std::shared_ptr<DistributedHardwareFwkKit> dhFwkKit = std::make_shared<DistributedHardwareFwkKit>();
    int32_t ret = dhFwkKit->StopDistributedHardware(dhType, std::string(srcNetworkId));
    if (ret != 0) {
        DHLOGE("StopDistributedHardware for DHType: %{public}u filed", (uint32_t)dhType);
    }
    return;
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_PauseDistributedHardwareSync(PauseDistributedHardwareSync);
TH_EXPORT_CPP_API_ResumeDistributedHardwareSync(ResumeDistributedHardwareSync);
TH_EXPORT_CPP_API_StopDistributedHardwareSync(StopDistributedHardwareSync);
// NOLINTEND
