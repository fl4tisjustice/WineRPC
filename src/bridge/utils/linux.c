/* =======================================================================
    This file is part of WineRPC.
    Copyright (C) 2024  Leah Santos  <leahsantos@proton.me>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

    The full license is available in the LICENSE file distributed with
    the source code in the root of the project.
 ====================================================================== */

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <stdint.h>

#include "bridge/utils/linux.h"
#include "bridge/log.h"

#define __ARG_COUNT(_10, _9, _8, _7, _6, _5, _4, _3, _2, _1, N, ...) N
#define _ARG_COUNT(...) __ARG_COUNT(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define _REVERSE_1(arg1) arg1
#define _REVERSE_2(arg1, arg2) arg2, arg1
#define _REVERSE_3(arg1, ...) _REVERSE_2(__VA_ARGS__), arg1
#define _REVERSE_4(arg1, ...) _REVERSE_3(__VA_ARGS__), arg1
#define _REVERSE_5(arg1, ...) _REVERSE_4(__VA_ARGS__), arg1
#define _REVERSE_6(arg1, ...) _REVERSE_5(__VA_ARGS__), arg1

#define __REVERSE(N, ...) _REVERSE_ ## N(__VA_ARGS__)
#define _REVERSE(N, ...) __REVERSE(N, __VA_ARGS__)

#define linux_syscall(nr, ...) _linux_syscall(nr, _REVERSE(_ARG_COUNT(__VA_ARGS__), __VA_ARGS__))

enum syscall_nr {
    NR_READ        = 0x03,
    NR_WRITE       = 0x04,
    NR_OPEN        = 0x05,
    NR_CLOSE       = 0x06,
    NR_SOCKETCALL  = 0x66,
    NR_MMAP2       = 0xC0,
    NR_MUNMAP      = 0x5B
};

enum socketcall_type{
    SC_SOCKET  = 0x01,
    SC_CONNECT = 0x03
};

inline uint32_t __linux_syscall(enum syscall_nr nr, uint32_t arg1, uint32_t arg2,
                                     uint32_t arg3, uint32_t arg4, uint32_t arg5) {
    uint32_t ret;

    __asm__ __volatile__ (
        "int 0x80\n\t"
        : "=a" (ret) 
        : "a" (nr),  "b"(arg1), "c"(arg2),
          "d"(arg3), "S"(arg4), "D"(arg5)
        : "memory", "cc"
    );
    
    return ret;
}

uint32_t _linux_syscall(enum syscall_nr nr, ...) {
    uint32_t arg1, arg2, arg3, arg4, arg5;
    arg1 = arg2 = arg3 = arg4 = arg5 = 0;

    va_list args;
    va_start(args, nr);

    switch (nr) {
        case NR_MMAP2:
            arg5 = va_arg(args, uint32_t);
            arg4 = va_arg(args, uint32_t);
            // Fall through
        case NR_READ:
        case NR_WRITE:
        case NR_OPEN:
            arg3 = va_arg(args, uint32_t);
            // Fall through
        case NR_SOCKETCALL:
        case NR_MUNMAP:
            arg2 = va_arg(args, uint32_t);
            // Fall through
        case NR_CLOSE:
            arg1 = va_arg(args, uint32_t);
            break;
    }

    va_end(args);

    return __linux_syscall(nr, arg1, arg2, arg3, arg4, arg5);
}

ssize_t linux_read(int fd, void *buf, size_t count) {
    bridge_log(LL_TRACE, "%s(%d, 0x%08X, %lu)\n", __func__, fd, (uint32_t)(uintptr_t)buf, (unsigned long)count);
    return linux_syscall(NR_READ, fd, buf, count);
}

ssize_t linux_write(int fd, const void *buf, size_t count) {
    bridge_log(LL_TRACE, "%s(%d, 0x%08X, %lu)\n", __func__, fd, (uint32_t)(uintptr_t)buf, (unsigned long)count);
    return linux_syscall(NR_WRITE, fd, buf, count);
}

int linux_open(const char *path, int flags, int mode) {
    bridge_log(LL_TRACE, "%s(%s, %d, %d)\n", __func__, path, flags, mode);
    return linux_syscall(NR_OPEN, path, flags, mode);
}

int linux_close(int fd) {
    bridge_log(LL_TRACE, "%s(%d)\n", __func__, fd);
    return linux_syscall(NR_CLOSE, fd);
}

int linux_socket(int domain, int type, int protocol) {
    bridge_log(LL_TRACE, "%s(%d, %d, %d)\n", __func__, domain, type, protocol);
    uint32_t args[] = { domain, type, protocol };
    return linux_syscall(NR_SOCKETCALL, SC_SOCKET, args);
}

int linux_connect(int socket, sockaddr *address, size_t address_len) {
    bridge_log(LL_TRACE, "%s(%d, 0x%08X, %lu)\n", __func__, socket, (uint32_t)(uintptr_t)address, (unsigned long)address_len);
    uint32_t args[] = { socket, (uintptr_t)address, address_len };
    return linux_syscall(NR_SOCKETCALL, SC_CONNECT, args);
}

void *linux_mmap2(void *addr, size_t len, int prot, int flags, int fd) {
    bridge_log(LL_TRACE, "%s(0x%08X, %lu, %d, %d, %d)\n", __func__, (uint32_t)(uintptr_t)addr, (unsigned long)len, prot, flags, fd);
    return (void*)(uintptr_t)linux_syscall(NR_MMAP2, addr, len, prot, flags, fd);
}

int linux_munmap(void *addr, size_t len) {
    bridge_log(LL_TRACE, "%s(0x%08X, %lu)\n", __func__, (uint32_t)(uintptr_t)addr, (unsigned long)len);
    return linux_syscall(NR_MUNMAP, addr, len);
}

