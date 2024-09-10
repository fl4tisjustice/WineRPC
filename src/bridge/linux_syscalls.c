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

#include "linux_syscalls.h"


#define __ARG_COUNT(_10, _9, _8, _7, _6, _5, _4, _3, _2, _1, N, ...) N
#define _ARG_COUNT(...) __ARG_COUNT(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define _REVERSE_1(arg1) arg1
#define _REVERSE_2(arg1, arg2) arg2, arg1
#define _REVERSE_3(arg1, ...) _REVERSE_2(__VA_ARGS__), arg1

#define __REVERSE(N, ...) _REVERSE_ ## N(__VA_ARGS__)
#define _REVERSE(N, ...) __REVERSE(N, __VA_ARGS__)

#define linux_syscall_helper(nr, ...) _linux_syscall_helper(nr, _REVERSE(_ARG_COUNT(__VA_ARGS__), __VA_ARGS__))


int linux_syscall(syscall_number nr, void *arg1, void *arg2, void *arg3) {
    int ret;
    asm volatile (
        "int 0x80\n\t"
        : "=a" (ret) 
        : "a" (nr), "b" (arg1), "c" (arg2), "d"(arg3)
        : "memory", "cc"
        );
    return ret;
}

int _linux_syscall_helper(syscall_number nr, ...) {

    void *arg1, *arg2, *arg3;
    arg1 = arg2 = arg3 = NULL;

    va_list args;
    va_start(args, nr);

    switch (nr) {
        case READ:
        case WRITE:
        case OPEN:
            arg3 = va_arg(args, void*);
            // Fall through
        case SOCKETCALL:
            arg2 = va_arg(args, void*);
            // Fall through
        case CLOSE:
            arg1 = va_arg(args, void*);
            break;
    }

    va_end(args);

    return linux_syscall(nr, arg1, arg2, arg3);
}

ssize_t linux_read(int fd, void *buf, size_t count) {
    return linux_syscall_helper(READ, (void*)fd, (void*)buf, (void*)count);
}

ssize_t linux_write(int fd, const void *buf, size_t count) {
    return linux_syscall_helper(WRITE, (void*)fd, (void*)buf, (void*)count);
}

int linux_open(const char *path, int flags, int mode) {
    return linux_syscall_helper(OPEN, (void*)path, (void*)flags, (void*)mode);
}

int linux_close(int fd) {
    return linux_syscall_helper(CLOSE, (void*)fd);
}

int linux_socket(int domain, int type, int protocol) {
    void *args[3];
    args[0] = (void*)domain;
    args[1] = (void*)type;
    args[2] = (void*)protocol;
    return linux_syscall_helper(SOCKETCALL, SOCKET, args);
}

int linux_connect(int socket, struct sockaddr *address, size_t address_len) {
    void *args[3];
    args[0] = (void*)socket;
    args[1] = (void*)address;
    args[2] = (void*)address_len;
    return linux_syscall_helper(SOCKETCALL, CONNECT, args);
}
