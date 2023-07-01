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

#ifndef OHOS_AV_TRANSPORT_CONSTANTS_H
#define OHOS_AV_TRANSPORT_CONSTANTS_H

#include <string>
#include <unistd.h>

namespace OHOS {
namespace DistributedHardware {
const int32_t AV_LOG_MAX_LEN = 4096;
const int32_t LINK_TYPE_MAX = 4;
const int32_t BASE_ENGINE_ID = 1000;
const int32_t DEVICE_ID_LENGTH = 65;
const int32_t INVALID_ENGINE_ID = -1;
const int32_t INVALID_SESSION_ID = -1;
const int32_t SESSION_WAIT_SECONDS = 5;
const int32_t INTERCERT_STRING_LENGTH = 20;
const int32_t PLUGIN_RANK = 100;

const uint32_t VIDEO_H264_LEVEL = 32;
const uint32_t MAX_DEVICE_ID_LEN = 100;
const uint32_t MAX_SESSION_NAME_LEN = 100;
const uint32_t MAX_MESSAGES_LEN = 40 * 1024 * 1024;
const uint32_t DSOFTBUS_INPUT_MAX_RECV_EXT_LEN = 104857600;
const uint32_t DSOFTBUS_INPUT_MAX_RECV_DATA_LEN = 104857600;
const uint32_t DEFAULT_FRAME_NUMBER = 100;
const uint32_t AUDIO_CHANNEL_LAYOUT_MONO = 1;
const uint32_t AUDIO_CHANNEL_LAYOUT_STEREO = 2;

const int64_t DEFAULT_PTS = 100;

const std::string EMPTY_STRING = "";
const std::string SENDER_CONTROL_SESSION_NAME_SUFFIX = "sender.avtrans.control";
const std::string RECEIVER_CONTROL_SESSION_NAME_SUFFIX = "receiver.avtrans.control";
const std::string SENDER_DATA_SESSION_NAME_SUFFIX = "sender.avtrans.data";
const std::string RECEIVER_DATA_SESSION_NAME_SUFFIX = "receiver.avtrans.data";
const std::string AV_SYNC_SENDER_CONTROL_SESSION_NAME = "ohos.dhardware.av.sync.sender.control";
const std::string AV_SYNC_RECEIVER_CONTROL_SESSION_NAME = "ohos.dhardware.av.sync.receiver.control";

const std::string AVINPUT_NAME = "builtin.avtransport.avinput";
const std::string AVOUTPUT_NAME = "builtin.avtransport.avoutput";
const std::string AENCODER_NAME = "builtin.recorder.audioencoder";
const std::string VENCODER_NAME = "builtin.recorder.videoencoder";
const std::string ADECODER_NAME = "builtin.player.audiodecoder";
const std::string VDECODER_NAME = "builtin.player.videodecoder";

const std::string AVT_DATA_META_TYPE = "avtrans_data_meta_type";
const std::string AVT_DATA_PARAM = "avtrans_data_param";
const std::string AVT_PARAM_BUFFERMETA_DATATYPE = "avtrans_param_buffermeta_datatype";
const std::string AV_TRANS_SPECIAL_DEVICE_ID = "av_trans_special_device_id";

const std::string KEY_MY_DEV_ID = "myDevId";
const std::string KEY_SCENE_TYPE = "sceneType";
const std::string KEY_PEER_DEV_ID = "peerDevId";
const std::string KEY_SESSION_ID = "sessionId";
const std::string KEY_SESSION_NAME = "sessionName";
const std::string KEY_AV_SYNC_FLAG = "avSyncFlag";
const std::string KEY_START_FRAME_NUM = "startFrameNum";
const std::string KEY_GROUP_INFO_ARRAY = "groupInfoArray";
const std::string KEY_SHARED_MEM_FD = "sharedMemoryFd";
const std::string KEY_SHARED_MEM_SIZE = "sharedMemorySize";
const std::string KEY_SHARED_MEM_NAME = "sharedMemoryName";

const uint8_t DATA_WAIT_SECONDS = 1;
const size_t DATA_QUEUE_MAX_SIZE = 1000;
constexpr const char *SEND_CHANNEL_EVENT = "SendChannelEvent";
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AV_TRANSPORT_CONSTANTS_H