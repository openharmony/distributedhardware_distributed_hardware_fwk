/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "socket.h"

int32_t Socket(SocketInfo info)
{
    (void)info;
    return 0;
}

int32_t Listen(int32_t socket, const QosTV qos[], uint32_t qosCount, const ISocketListener *listener)
{
    (void)socket;
    (void)qos;
    (void)qosCount;
    (void)listener;
    return 0;
}

int32_t Bind(int32_t socket, const QosTV qos[], uint32_t qosCount, const ISocketListener *listener)
{
    (void)socket;
    (void)qos;
    (void)qosCount;
    (void)listener;
    return 0;
}

int32_t SendBytes(int32_t socket, const void *data, uint32_t len)
{
    (void)socket;
    (void)data;
    (void)len;
    return 0;
}

int32_t SendMessage(int32_t socket, const void *data, uint32_t len)
{
    (void)socket;
    (void)data;
    (void)len;
    return 0;
}

int32_t SendStream(int32_t socket, const StreamData *data, const StreamData *ext, const StreamFrameInfo *param)
{
    (void)socket;
    (void)data;
    (void)ext;
    (void)param;
    return 0;
}

int32_t SendFile(int32_t socket, const char *sFileList[], const char *dFileList[], uint32_t fileCnt)
{
    (void)socket;
    (void)sFileList;
    (void)dFileList;
    (void)fileCnt;
    return 0;
}

void Shutdown(int32_t socket)
{
    (void)socket;
}

int32_t EvaluateQos(const char *peerNetworkId, TransDataType dataType, const QosTV *qos, uint32_t qosCount)
{
    (void)peerNetworkId;
    (void)dataType;
    (void)qos;
    (void)qosCount;
    return 0;
}