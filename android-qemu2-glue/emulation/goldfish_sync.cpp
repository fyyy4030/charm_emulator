/* Copyright 2016 The Android Open Source Project
**
** This software is licensed under the terms of the GNU General Public
** License version 2, as published by the Free Software Foundation, and
** may be copied, distributed, and modified under those terms.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
*/
#include "android-qemu2-glue/emulation/goldfish_sync.h"

// Glue code between the virtual goldfish sync device, and the
// host-side sync service implementation.

#include "android/base/Log.h"
#include "android/emulation/GoldfishSyncCommandQueue.h"
#include "android/emulation/goldfish_sync.h"

extern "C" {
#include "hw/misc/goldfish_sync.h"
}  // extern "C"

static trigger_wait_fn_t sTriggerWaitFn = nullptr;

// These callbacks are called from the host sync service to operate
// on the virtual device (.doHostCommand) or register a trigger-waiting
// callback that will be invoked later from the device
// (.registerTriggerWait).
static GoldfishSyncDeviceInterface kSyncDeviceInterface = {
    .doHostCommand = goldfish_sync_send_command,
    .registerTriggerWait = [](trigger_wait_fn_t fn) {
        sTriggerWaitFn = fn;
    },
};

// These callbacks are called from the virtual device to send
// data to the host sync service.
static const GoldfishSyncServiceOps kSyncServiceOps = {
    .receive_hostcmd_result = goldfish_sync_receive_hostcmd_result,
    .trigger_host_wait = [](uint64_t glsync_ptr,
                            uint64_t thread_ptr,
                            uint64_t timeline) {
        if (sTriggerWaitFn) {
            sTriggerWaitFn(glsync_ptr, thread_ptr, timeline);
        }
    }
};

bool qemu_android_sync_init(android::VmLock* vmLock) {
    goldfish_sync_set_service_ops(&kSyncServiceOps);
    goldfish_sync_set_hw_funcs(&kSyncDeviceInterface);
    android::GoldfishSyncCommandQueue::initThreading(vmLock);
    return true;
}
