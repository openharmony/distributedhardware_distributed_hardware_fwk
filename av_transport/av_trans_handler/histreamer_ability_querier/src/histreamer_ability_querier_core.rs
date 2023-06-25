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

//! rust dynamic lib for query histreamer ability

use std::os::raw::c_char;
use std::os::raw::c_int;

static AUDIO_ABILITY: &str = "{ \"AAC\" }";
static VIDEO_ABILITY: &str = "{ \"H264\", \"H265\" }";

/// function Query Ablility by ability type
#[no_mangle]
#[allow(clippy::not_unsafe_ptr_arg_deref)]
pub extern "C" fn Query(ability_type: u8, out: *mut c_char) -> c_int
{
    match ability_type {
        1 => {
            let addr: *mut u8 = AUDIO_ABILITY.as_ptr() as *mut u8;
            let len = AUDIO_ABILITY.len();
            unsafe { std::ptr::copy(addr, out, AUDIO_ABILITY.len()) };
            len as c_int
        }
        2 => {
            let addr: *mut u8 = VIDEO_ABILITY.as_ptr() as *mut u8;
            let len = VIDEO_ABILITY.len();
            unsafe { std::ptr::copy(addr, out, VIDEO_ABILITY.len()) };
            len as c_int
        }
        _ => { 0 }
    }
}