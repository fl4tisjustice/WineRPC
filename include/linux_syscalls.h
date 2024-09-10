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
 
#pragma once

#include <unistd.h>
#include <fcntl.h>

typedef enum {
    READ        = 0x03,
    WRITE       = 0x04,
    OPEN        = 0x05,
    CLOSE       = 0x06,
    SOCKETCALL  = 0x66,
} syscall_number;

typedef enum {
    SOCKET  = 0x01,
    CONNECT = 0x03
} socketcall_type;

struct sockaddr {
    unsigned short  sa_family;      /* Address family */
    char            sa_data[];      /* Socket address */
};

ssize_t linux_read(int fd, void *buf, size_t count);
ssize_t linux_write(int fd, const void *buf, size_t count);
int linux_open(const char *path, int flags, int mode);
int linux_close(int fd);
int linux_socket(int domain, int type, int protocol);
int linux_connect(int socket, struct sockaddr *address, size_t address_len);

