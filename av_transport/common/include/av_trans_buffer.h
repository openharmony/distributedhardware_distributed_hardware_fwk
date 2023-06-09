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

#ifndef OHOS_AV_TRANSPORT_BUFFER_H
#define OHOS_AV_TRANSPORT_BUFFER_H

#include <map>
#include <memory>
#include <vector>

#include "av_trans_types.h"

namespace OHOS {
namespace DistributedHardware {
constexpr size_t INVALID_POSITION = -1;

enum struct MetaType : uint32_t {
    AUDIO,
    VIDEO,
};

/**
* @brief BufferData description. Only manager the basic memory information.
*/
class BufferData {
public:
    explicit BufferData(size_t capacity);
    BufferData(size_t capacity, std::shared_ptr<uint8_t> bufData);
    virtual ~BufferData() = default;

    size_t GetSize();
    size_t GetCapacity();
    uint8_t *GetAddress() const;
    size_t Write(const uint8_t* in, size_t writeSize, size_t position = INVALID_POSITION);
    size_t Read(uint8_t* out, size_t readSize, size_t position = INVALID_POSITION);
    void Reset();
    void SetSize(size_t size);

private:
    size_t capacity_;
    size_t size_;
    std::shared_ptr<uint8_t> address_;
};

/**
 * @brief BufferMeta for buffer. Base class that describes various metadata.
 */
class BufferMeta {
public:
    explicit BufferMeta(MetaType type);
    virtual ~BufferMeta() = default;

    MetaType GetMetaType() const;
    bool GetMetaItem(AVTransTag tag, std::string &value);
    void SetMetaItem(AVTransTag tag, const std::string &value);

private:
    MetaType type_;
    std::map<AVTransTag, std::string> tagMap_;
};

/**
* @brief AVTransBuffer base class.
* Contains the data storage and metadata information of the buffer (buffer description information).
*/
class AVTransBuffer {
public:
    explicit AVTransBuffer(MetaType type = MetaType::VIDEO);
    ~AVTransBuffer() = default;

    std::shared_ptr<BufferData> GetBufferData(uint32_t index = 0);
    std::shared_ptr<BufferData> CreateBufferData(size_t capacity);
    std::shared_ptr<BufferData> WrapBufferData(const uint8_t* data, size_t capacity, size_t size);
    std::shared_ptr<BufferMeta> GetBufferMeta();
    void UpdateBufferMeta(std::shared_ptr<BufferMeta> bufferMeta);
    uint32_t GetDataCount();
    void Reset();
    bool IsEmpty();

private:
    std::vector<std::shared_ptr<BufferData>> data_ {};
    std::shared_ptr<BufferMeta> meta_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif  // OHOS_AV_TRANSPORT_BUFFER_H