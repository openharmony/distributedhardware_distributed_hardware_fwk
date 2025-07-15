/*
 * Copyright (C) 2023-2025 Huawei Device Co., Ltd.
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

#include "av_sync_utils.h"

#include <sys/mman.h>
#include <securec.h>
#include <unistd.h>
#include "ashmem.h"
#include "cJSON.h"

#include "av_trans_constants.h"
#include "av_trans_errno.h"
#include "av_trans_log.h"

namespace OHOS {
namespace DistributedHardware {
AVTransSharedMemory CreateAVTransSharedMemory(const std::string &name, size_t size)
{
    int32_t fd = AshmemCreate(name.c_str(), size);
    if (fd <= 0) {
        AVTRANS_LOGE("create av trans shared memory failed, name=%{public}s, fd=%{public}" PRId32, name.c_str(), fd);
        return AVTransSharedMemory{0, 0, name, nullptr};
    }

    unsigned int prot = PROT_READ | PROT_WRITE;
    int result = AshmemSetProt(fd, static_cast<int>(prot));
    if (result < 0) {
        AVTRANS_LOGE("AshmemSetProt failed, name=%{public}s, fd=%{public}" PRId32, name.c_str(), fd);
        (void)::close(fd);
        return AVTransSharedMemory{0, 0, name, nullptr};
    }

    void *addr = ::mmap(nullptr, size, static_cast<int>(prot), MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        AVTRANS_LOGE("shared memory mmap failed, name=%{public}s, fd=%{public}" PRId32, name.c_str(), fd);
        (void)::close(fd);
        return AVTransSharedMemory{0, 0, name, nullptr};
    }

    uint8_t *base = reinterpret_cast<uint8_t*>(addr);
    if (memset_s(base, size, INVALID_VALUE_FALG, size) != EOK) {
        AVTRANS_LOGE("memset_s failed.");
        (void)::close(fd);
        (void)::munmap(addr, size);
        return AVTransSharedMemory{0, 0, name, nullptr};
    }
    AVTRANS_LOGI("create av trans shared memory success, name=%{public}s, size=%{public}zu, fd=%{public}d"
        PRId32, name.c_str(), size, fd);
    return AVTransSharedMemory{fd, size, name, addr};
}

void CloseAVTransSharedMemory(AVTransSharedMemory &memory) noexcept
{
    AVTRANS_LOGI("close shared memory, name=%{public}s, size=%{public}" PRId32 ", fd=%{public}" PRId32,
        memory.name.c_str(), memory.size, memory.fd);
    if (IsInValidSharedMemory(memory)) {
        AVTRANS_LOGE("invalid input shared memory");
        return;
    }
    if (memory.fd > 0) {
        (void)::close(memory.fd);
        memory.fd = -1;
    }
    if (memory.addr != nullptr) {
        (void)::munmap(memory.addr, memory.size);
        memory.addr = nullptr;
    }
}

int32_t WriteClockUnitToMemory(const AVTransSharedMemory &memory, AVSyncClockUnit &clockUnit)
{
    AVTRANS_LOGI("write clock unit to shared memory, name=%{public}s, size=%{public}" PRId32 ", fd=%{public}" PRId32,
        memory.name.c_str(), memory.size, memory.fd);
    TRUE_RETURN_V_MSG_E(IsInValidSharedMemory(memory), ERR_DH_AVT_INVALID_PARAM, "invalid input shared memory");

    int mSize = 12;
    TRUE_RETURN_V_MSG_E(memory.size < mSize, ERR_DH_AVT_INVALID_PARAM,
        "Memory.size is too small, future access may exceed the limit");
    AVTRANS_LOGI("clock unit index=%{public}" PRId32 ", frameNum=%{public}" PRId32 ", pts=%{public}lld",
        clockUnit.index, clockUnit.frameNum, (long long)clockUnit.pts);
    TRUE_RETURN_V_MSG_E(IsInValidClockUnit(clockUnit), ERR_DH_AVT_INVALID_PARAM, "invalid input clock unit");

    int size = AshmemGetSize(memory.fd);
    TRUE_RETURN_V_MSG_E(size != memory.size, ERR_DH_AVT_SHARED_MEMORY_FAILED, "invalid memory size = %{public}" PRId32,
        size);

    unsigned int prot = PROT_WRITE;
    int result = AshmemSetProt(memory.fd, static_cast<int>(prot));
    TRUE_RETURN_V_MSG_E(result < 0, ERR_DH_AVT_SHARED_MEMORY_FAILED, "AshmemSetProt failed");

    uint8_t *base = reinterpret_cast<uint8_t*>(memory.addr);
    size_t fOffset = (sizeof(uint32_t) + sizeof(int64_t)) * clockUnit.index;
    size_t tOffset = fOffset + sizeof(uint32_t);
    U64ToU8(base + tOffset, clockUnit.pts, NUM_EIGHT);
    U32ToU8(base + fOffset, clockUnit.frameNum, NUM_FOUR);

    clockUnit.index ++;
    if (clockUnit.index == MAX_CLOCK_UNIT_COUNT) {
        clockUnit.index = 0;
    }
    AVTRANS_LOGI("write clock unit frameNum=%{public}" PRId32 ", pts=%{public}lld to shared memory success",
        clockUnit.frameNum, (long long)(clockUnit.pts));
    return DH_AVT_SUCCESS;
}

int32_t ReadClockUnitFromMemory(const AVTransSharedMemory &memory, AVSyncClockUnit &clockUnit)
{
    AVTRANS_LOGI("read clock unit from shared memory, name=%{public}s, size=%{public}" PRId32 ", fd=%{public}" PRId32,
        memory.name.c_str(), memory.size, memory.fd);
    TRUE_RETURN_V_MSG_E(IsInValidSharedMemory(memory), ERR_DH_AVT_INVALID_PARAM, "invalid input shared memory");

    int mSize = 12;
    TRUE_RETURN_V_MSG_E(memory.size < mSize, ERR_DH_AVT_INVALID_PARAM,
        "Memory.size is too small, future access may exceed the limit");
    AVTRANS_LOGI("clock unit index=%{public}" PRId32 ", frameNum=%{public}" PRId32,
        clockUnit.index, clockUnit.frameNum);
    TRUE_RETURN_V_MSG_E((clockUnit.frameNum <= 0), ERR_DH_AVT_INVALID_PARAM, "invalid input frame number");

    int size = AshmemGetSize(memory.fd);
    TRUE_RETURN_V_MSG_E(size != memory.size, ERR_DH_AVT_SHARED_MEMORY_FAILED, "invalid memory size = %{public}" PRId32,
        size);

    unsigned int prot = PROT_WRITE;
    int result = AshmemSetProt(memory.fd, static_cast<int>(prot));
    TRUE_RETURN_V_MSG_E(result < 0, ERR_DH_AVT_SHARED_MEMORY_FAILED, "AshmemSetProt failed");
    uint8_t *base = reinterpret_cast<uint8_t*>(memory.addr);
    uint32_t firstUnit = U8ToU32(base, NUM_FOUR);
    TRUE_RETURN_V_MSG_E(firstUnit == 0, ERR_DH_AVT_MASTER_NOT_READY, "master queue not ready, clock is null.");

    uint32_t index = 0;
    int64_t latestPts = 0;
    size_t unitSize = sizeof(uint32_t) + sizeof(int64_t);
    while (index < MAX_CLOCK_UNIT_COUNT) {
        uint32_t frameNum = U8ToU32(base + (index * unitSize), NUM_FOUR);
        int64_t pts = static_cast<int64_t>(U8ToU64(base + (index * unitSize) + sizeof(uint32_t), NUM_EIGHT));
        if (pts > latestPts) {
            latestPts = pts;
            clockUnit.pts = pts;
            clockUnit.frameNum = frameNum;
        }
        index++;
    }
    AVTRANS_LOGI("read clock unit from shared memory success, frameNum=%{public}" PRId32 ", pts=%{public}lld",
        clockUnit.frameNum, (long long)clockUnit.pts);
    return DH_AVT_SUCCESS;
}

int32_t WriteFrameInfoToMemory(const AVTransSharedMemory &memory, uint32_t frameNum, int64_t timestamp)
{
    AVTRANS_LOGI("write frame info to shared memory, name=%{public}s, size=%{public}" PRId32 ", fd=%{public}" PRId32,
        memory.name.c_str(), memory.size, memory.fd);
    TRUE_RETURN_V_MSG_E(IsInValidSharedMemory(memory), ERR_DH_AVT_INVALID_PARAM, "invalid input shared memory");

    int mSize = 12;
    TRUE_RETURN_V_MSG_E(memory.size < mSize, ERR_DH_AVT_INVALID_PARAM,
        "Memory.size is too small, future access may exceed the limit");
    TRUE_RETURN_V_MSG_E((frameNum <= 0), ERR_DH_AVT_INVALID_PARAM, "invalid input frame number");

    int size = AshmemGetSize(memory.fd);
    TRUE_RETURN_V_MSG_E(size != memory.size, ERR_DH_AVT_SHARED_MEMORY_FAILED, "invalid memory size = %{public}" PRId32,
        size);

    unsigned int prot = PROT_WRITE;
    int result = AshmemSetProt(memory.fd, static_cast<int>(prot));
    TRUE_RETURN_V_MSG_E(result < 0, ERR_DH_AVT_SHARED_MEMORY_FAILED, "AshmemSetProt failed");
    uint8_t *base = reinterpret_cast<uint8_t*>(memory.addr);
    U32ToU8(base, frameNum, NUM_FOUR);
    U64ToU8(base + sizeof(uint32_t), timestamp, NUM_EIGHT);

    AVTRANS_LOGI("write frameNum=%{public}" PRId32 ", timestamp=%{public}lld to shared memory success",
        frameNum, (long long)timestamp);
    return DH_AVT_SUCCESS;
}

int32_t ReadFrameInfoFromMemory(const AVTransSharedMemory &memory, uint32_t &frameNum, int64_t &timestamp)
{
    AVTRANS_LOGI("read frame info from shared memory, name=%{public}s, size=%{public}" PRId32 ", fd=%{public}" PRId32,
        memory.name.c_str(), memory.size, memory.fd);
    TRUE_RETURN_V_MSG_E(IsInValidSharedMemory(memory), ERR_DH_AVT_INVALID_PARAM, "invalid input shared memory");
    int mSize = 12;
    TRUE_RETURN_V_MSG_E(memory.size < mSize, ERR_DH_AVT_INVALID_PARAM,
        "Memory.size is too small, future access may exceed the limit");

    int size = AshmemGetSize(memory.fd);
    TRUE_RETURN_V_MSG_E(size != memory.size, ERR_DH_AVT_SHARED_MEMORY_FAILED, "invalid memory size = %{public}" PRId32,
        size);

    unsigned int prot = PROT_WRITE;
    int result = AshmemSetProt(memory.fd, static_cast<int>(prot));
    TRUE_RETURN_V_MSG_E(result < 0, ERR_DH_AVT_SHARED_MEMORY_FAILED, "AshmemSetProt failed");
    uint8_t *base = reinterpret_cast<uint8_t*>(memory.addr);
    frameNum = U8ToU32(base, NUM_FOUR);
    timestamp = static_cast<int64_t>(U8ToU64(base + sizeof(uint32_t), NUM_EIGHT));
    TRUE_RETURN_V_MSG_E(frameNum <= 0, ERR_DH_AVT_MASTER_NOT_READY, "master queue not ready, frameNum is null.");

    AVTRANS_LOGI("read frameNum=%{public}" PRId32 ", timestamp=%{public}lld from shared memory success.", frameNum,
        (long long)timestamp);
    return DH_AVT_SUCCESS;
}

int32_t ResetSharedMemory(const AVTransSharedMemory &memory)
{
    AVTRANS_LOGI("reset shared memory, name=%{public}s, size=%{public}" PRId32 ", fd=%{public}" PRId32,
        memory.name.c_str(), memory.size, memory.fd);
    TRUE_RETURN_V_MSG_E(IsInValidSharedMemory(memory), ERR_DH_AVT_INVALID_PARAM, "invalid input shared memory");

    int size = AshmemGetSize(memory.fd);
    TRUE_RETURN_V_MSG_E(size != memory.size, ERR_DH_AVT_SHARED_MEMORY_FAILED, "invalid memory size = %{public}" PRId32,
        size);

    unsigned int prot = PROT_WRITE;
    int result = AshmemSetProt(memory.fd, static_cast<int>(prot));
    TRUE_RETURN_V_MSG_E(result < 0, ERR_DH_AVT_SHARED_MEMORY_FAILED, "AshmemSetProt failed");
    if (memset_s(reinterpret_cast<uint8_t*>(memory.addr), size, INVALID_VALUE_FALG, size) != EOK) {
        AVTRANS_LOGE("memset_s failed.");
        return ERR_DH_AVT_SHARED_MEMORY_FAILED;
    }
    AVTRANS_LOGI("reset shared memory success.");
    return DH_AVT_SUCCESS;
}

bool IsInValidSharedMemory(const AVTransSharedMemory &memory)
{
    return (memory.fd <= 0) || (memory.size <= 0) || memory.name.empty() || (memory.addr == nullptr);
}

bool IsInValidClockUnit(const AVSyncClockUnit &clockUnit)
{
    return (clockUnit.index < 0) || (clockUnit.index >= MAX_CLOCK_UNIT_COUNT) || (clockUnit.frameNum <= 0)
        || (clockUnit.pts <= 0);
}

std::string MarshalSharedMemory(const AVTransSharedMemory &memory)
{
    cJSON *memoryJson = cJSON_CreateObject();
    if (memoryJson == nullptr) {
        return "";
    }
    cJSON_AddNumberToObject(memoryJson, KEY_SHARED_MEM_FD.c_str(), memory.fd);
    cJSON_AddNumberToObject(memoryJson, KEY_SHARED_MEM_SIZE.c_str(), memory.size);
    cJSON_AddStringToObject(memoryJson, KEY_SHARED_MEM_NAME.c_str(), memory.name.c_str());

    char *data = cJSON_PrintUnformatted(memoryJson);
    if (data == nullptr) {
        cJSON_Delete(memoryJson);
        return "";
    }
    std::string jsonstr(data);
    cJSON_free(data);
    cJSON_Delete(memoryJson);
    return jsonstr;
}

AVTransSharedMemory UnmarshalSharedMemory(const std::string &jsonStr)
{
    cJSON *paramJson = cJSON_Parse(jsonStr.c_str());
    if (paramJson == nullptr) {
        return AVTransSharedMemory{0, 0, ""};
    }
    cJSON *fdObj = cJSON_GetObjectItemCaseSensitive(paramJson, KEY_SHARED_MEM_FD.c_str());
    if (fdObj == nullptr || !cJSON_IsNumber(fdObj)) {
        cJSON_Delete(paramJson);
        return AVTransSharedMemory{0, 0, ""};
    }
    int32_t fd = fdObj->valueint;
    cJSON *sizeObj = cJSON_GetObjectItemCaseSensitive(paramJson, KEY_SHARED_MEM_SIZE.c_str());
    if (sizeObj == nullptr || !cJSON_IsNumber(sizeObj)) {
        cJSON_Delete(paramJson);
        return AVTransSharedMemory{0, 0, ""};
    }
    int32_t size = sizeObj->valueint;
    cJSON *nameObj = cJSON_GetObjectItemCaseSensitive(paramJson, KEY_SHARED_MEM_NAME.c_str());
    if (nameObj == nullptr || !cJSON_IsString(nameObj)) {
        cJSON_Delete(paramJson);
        return AVTransSharedMemory{0, 0, ""};
    }
    std::string name = nameObj->valuestring;
    cJSON_Delete(paramJson);
    return AVTransSharedMemory{ fd, size, name };
}

void U32ToU8(uint8_t *arrayPtr, uint32_t value, size_t arraySize)
{
    if (arrayPtr == nullptr || arraySize < NUM_FOUR) {
        return;
    }
    for (auto i = NUM_ZERO; i < NUM_FOUR; ++i) {
        arrayPtr[i] = static_cast<uint8_t>((value >> (i * NUM_EIGHT)) & 0xff);
    }
}

void U64ToU8(uint8_t *arrayPtr, uint64_t value, size_t arraySize)
{
    if (arrayPtr == nullptr || arraySize < NUM_EIGHT) {
        return;
    }
    for (auto i = NUM_ZERO; i < NUM_EIGHT; ++i) {
        arrayPtr[i] = static_cast<uint8_t>((value >> (i * NUM_EIGHT)) & 0xff);
    }
}

uint32_t U8ToU32(const uint8_t *arrayPtr, size_t arraySize)
{
    if (arrayPtr == nullptr || arraySize < NUM_FOUR) {
        return -1;
    }
    uint32_t result = 0;
    for (auto i = NUM_ZERO; i < NUM_FOUR; ++i) {
        result |= static_cast<uint32_t>(arrayPtr[i] & 0xff) << (i * NUM_EIGHT);
    }
    return result;
}

uint64_t U8ToU64(const uint8_t *arrayPtr, size_t arraySize)
{
    if (arrayPtr == nullptr || arraySize < NUM_EIGHT) {
        return -1;
    }
    uint64_t result = 0;
    for (auto i = NUM_ZERO; i < NUM_EIGHT; ++i) {
        result |= static_cast<uint64_t>(arrayPtr[i] & 0xff) << (i * NUM_EIGHT);
    }
    return result;
}
} // namespace DistributedHardware
} // namespace OHOS