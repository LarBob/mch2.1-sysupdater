// following code from SuperUserLib3DS;
// https://github.com/delebile/SuperUserLib3DS
// MIT license follows
//
// Copyright (C) 2015 Jason Dellaluce, Steveice10 for part of his code
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include <3ds.h>
#include <stdio.h>

volatile u32 pidBackup = 0;
static const char *patch_result = "F";
static const char *unpatch_result = "F";
static u8 isNew3DS = 0;

static s32 patchPID(void) {
    // Patch PID in order access all services
    __asm__ volatile("cpsid aif");
    u8 *KProcess = (u8 *)*((u32 *)0xFFFF9004);
    pidBackup = *((u32 *)(KProcess + (isNew3DS ? 0xBC : 0xB4)));
    *((u32 *)(KProcess + (isNew3DS ? 0xBC : 0xB4))) = 0;
    patch_result = "P";
    return 0;
}

static s32 unpatchPID(void) {
    // Restore what we changed
    __asm__ volatile("cpsid aif");
    u8 *KProcess = (u8 *)*((u32 *)0xFFFF9004);
    *((u32 *)(KProcess + (isNew3DS ? 0xBC : 0xB4))) = pidBackup;
    unpatch_result = "P";
    return 0;
}

void patchServiceAccess(void) {
    APT_CheckNew3DS(&isNew3DS);

    u32 pid1;
    svcGetProcessId(&pid1, 0xFFFF8001);
    // Set the current process id (PID) to 0
    svcBackdoor(&patchPID);

    u32 pid2;
    svcGetProcessId(&pid2, 0xFFFF8001);
    // Re-initialize srv connection. It will consider this the process with id 0
    // so we will have access to any service
    srvExit();
    srvInit();

    // Once we tricked srv we can restore the real PID
    svcBackdoor(&unpatchPID);

    u32 pid3;
    svcGetProcessId(&pid3, 0xFFFF8001);
    printf("%lu=%lu=%lu %lu=0 %s %s\n", pid1, pidBackup, pid3, pid2,
           patch_result, unpatch_result);
}

// end code from SuperUserLib3DS
