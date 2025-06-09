/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "avtransstubonremoterequest_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>
#include "av_trans_control_center_callback_stub.h"
#include "av_trans_control_center_callback.h"
#include "av_sync_utils.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
namespace DistributedHardware {
const uint32_t DC_MESSAGE_SIZE = 4;

std::string MarshalSharedMemory(const AVTransSharedMemory &memory)
{
    return "";
}

AVTransSharedMemory UnmarshalSharedMemory(const std::string &jsonStr)
{
    return AVTransSharedMemory{0, 0, ""};
}

void AVTransStubOnRemoteRequestFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    MessageParcel pdata;
    MessageParcel reply;
    MessageOption option;
    pdata.WriteInterfaceToken(AVTransControlCenterCallback::GetDescriptor());
    uint32_t code = *(reinterpret_cast<const uint32_t*>(data)) % DC_MESSAGE_SIZE;
    if (code == (uint32_t)IAvTransControlCenterCallbackIpcCode::COMMAND_SET_PARAMETER) {
        uint32_t tag = *(reinterpret_cast<const uint32_t*>(data));
        std::string value(reinterpret_cast<const char*>(data), size);
        pdata.WriteUint32(tag);
        pdata.WriteString(value);
    } else if (code == (uint32_t)IAvTransControlCenterCallbackIpcCode::COMMAND_SET_SHARED_MEMORY) {
        FuzzedDataProvider fdp(data, size);
        int32_t len = fdp.ConsumeIntegral<int32_t>();
        std::string name(reinterpret_cast<const char*>(data), size);
        pdata.WriteInt32(len);
        pdata.WriteString(name);
    } else if (code == (uint32_t)IAvTransControlCenterCallbackIpcCode::COMMAND_NOTIFY) {
        uint32_t type = *(reinterpret_cast<const uint32_t*>(data));
        std::string content(reinterpret_cast<const char*>(data), size);
        std::string devId(reinterpret_cast<const char*>(data), size);
        pdata.WriteUint32(type);
        pdata.WriteString(content);
        pdata.WriteString(devId);
    }

    sptr<AVTransControlCenterCallback> controlCenterCallback(new (std::nothrow) AVTransControlCenterCallback());
    if (controlCenterCallback == nullptr) {
        return;
    }
    controlCenterCallback->OnRemoteRequest(code, pdata, reply, option);
}
} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::AVTransStubOnRemoteRequestFuzzTest(data, size);
    return 0;
}