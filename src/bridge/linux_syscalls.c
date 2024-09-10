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
#include <tchar.h>
#include "linux_syscalls.h"

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

#define linux_syscall_helper(nr, ...) _linux_syscall_helper(nr, _REVERSE(_ARG_COUNT(__VA_ARGS__), __VA_ARGS__))

int linux_syscall(syscall_number nr, uintptr_t arg1,
                     uintptr_t arg2, uintptr_t arg3,
                     uintptr_t arg4, uintptr_t arg5) {


    int ret;
    __asm__ __volatile__ (
        "int 0x80\n\t"
        : "=a" (ret) 
        : "a" (nr), "b"(arg1), "c"(arg2),
          "d"(arg3), "S"(arg4), "D"(arg5)
        : "memory", "cc"
        );
    return ret;
}

uintptr_t _linux_syscall_helper(syscall_number nr, ...) {

    uintptr_t arg1, arg2, arg3, arg4, arg5;
    arg1 = arg2 = arg3 = arg4 = arg5 = 0;

    va_list args;
    va_start(args, nr);

    switch (nr) {
        case MMAP2:
            arg5 = va_arg(args, uintptr_t);
            arg4 = va_arg(args, uintptr_t);
            // Fall through
        case READ:
        case WRITE:
        case OPEN:
            arg3 = va_arg(args, uintptr_t);
            // Fall through
        case SOCKETCALL:
        case MUNMAP:
            arg2 = va_arg(args, uintptr_t);
            // Fall through
        case CLOSE:
            arg1 = va_arg(args, uintptr_t);
            break;
    }

    va_end(args);

    return linux_syscall(nr, arg1, arg2, arg3, arg4, arg5);
}

ssize_t linux_read(int fd, void *buf, size_t count) {
    return linux_syscall_helper(READ, fd, buf, count);
}

ssize_t linux_write(int fd, const void *buf, size_t count) {
    return linux_syscall_helper(WRITE, fd, buf, count);
}

int linux_open(const char *path, int flags, int mode) {
    return linux_syscall_helper(OPEN, path, flags, mode);
}

int linux_close(int fd) {
    return linux_syscall_helper(CLOSE, fd);
}

int linux_socket(int domain, int type, int protocol) {
    unsigned long args[3] = {0};
    args[0] = (unsigned long)domain;
    args[1] = (unsigned long)type;
    args[2] = (unsigned long)protocol;
    return linux_syscall_helper(SOCKETCALL, SOCKET, args);
}

int linux_connect(int socket, sockaddr *address, size_t address_len) {
    unsigned long args[3] = {0};
    args[0] = (unsigned long)socket;
    args[1] = (unsigned long)(uintptr_t)address;
    args[2] = (unsigned long)address_len;
    return linux_syscall_helper(SOCKETCALL, CONNECT, args);
}

void *linux_mmap2(void *addr, size_t len, int prot, int flags, int fd) {
    return (void*)linux_syscall_helper(MMAP2, addr, len, prot, flags, fd);
}

int linux_munmap(void *addr, size_t len) {
    return linux_syscall_helper(MUNMAP, addr, len);
}