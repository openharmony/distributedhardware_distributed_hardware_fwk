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

import hardwareManager from "@ohos.distributedHardware.hardwareManager";
import { BusinessError } from "@ohos.base";

function test_pauseDistributedHardware()
{
    console.println(`Tag into func test_pauseDistributedHardware`);
    try {
      let description: hardwareManager.HardwareDescriptor = {
        type: hardwareManager.DistributedHardwareType.CAMERA,
        srcNetworkId: '1111'
      };
      hardwareManager.pauseDistributedHardware(description).then(() => {
        console.println('resume distributed hardware successfully');
      }).catch((error: Error) => {
        console.println('resume distributed hardware failed, cause:' + error);
      })
      console.log('resume distributed hardware successfully');
    } catch (error) {
      console.println('resume distributed hardware failed:' + error);
    }
    console.println(`Tag out func test_pauseDistributedHardware`);
}

function test_resumeDistributedHardware()
{
    console.println(`Tag into func test_resumeDistributedHardware`);
    try {
      let description: hardwareManager.HardwareDescriptor = {
        type: hardwareManager.DistributedHardwareType.CAMERA,
        srcNetworkId: '1111'
      };
      hardwareManager.resumeDistributedHardware(description).then(() => {
        console.println('resume distributed hardware successfully');
      }).catch((error: Error) => {
        console.println('resume distributed hardware failed, cause:' + error);
      })
      console.log('resume distributed hardware successfully');
    } catch (error) {
      console.println('resume distributed hardware failed:' + error);
    }
    console.println(`Tag out func test_resumeDistributedHardware`);
}

function test_stopDistributedHardware()
{
    console.println(`Tag into func test_stopDistributedHardware`);
    try {
      let description: hardwareManager.HardwareDescriptor = {
        type: hardwareManager.DistributedHardwareType.CAMERA,
        srcNetworkId: '1111'
      };
      hardwareManager.stopDistributedHardware(description).then(() => {
        console.println('resume distributed hardware successfully');
      }).catch((error: Error) => {
        console.println('resume distributed hardware failed, cause:' + error);
      })
      console.log('resume distributed hardware successfully');
    } catch (error) {
      console.println('resume distributed hardware failed:' + error);
    }
    console.println(`Tag out func test_stopDistributedHardware`);
}

function main() {
    console.println(`Tag into main`);
    console.println(`Tag *************************************************`);
    test_pauseDistributedHardware();
    console.println(`Tag *************************************************`);
    test_resumeDistributedHardware();
    console.println(`Tag *************************************************`);
    test_stopDistributedHardware();
    console.println(`Tag *************************************************`);
    console.println(`Tag out main`);
}