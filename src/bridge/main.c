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

// Based on https://github.com/0e4ef622/wine-discord-ipc-bridge/blob/master/main.c

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "linux_syscalls.h"

#define ARRLEN(arr) (sizeof(arr) / sizeof(arr[0]))
#define AF_UNIX     1
#define SOCK_STREAM 1
#define BUFSIZE     2048 // size of read/write buffers

static HANDLE hPipe;
static int sock_fd;

static const char* get_sock_parent_path() {
    const char *env_tmp_paths[] = {"XDG_RUNTIME_DIR", "TMPDIR", "TMP", "TEMP"};
    char *path;

    for (size_t i = 0; i < ARRLEN(env_tmp_paths); i++)
        if ((path = getenv(env_tmp_paths[i])))
            return path;
    
    return "/tmp";
}

DWORD WINAPI winwrite_thread() {

    while (TRUE) {
        char buf[BUFSIZE];
        ssize_t bytes_read = linux_read(sock_fd, buf, BUFSIZE);
        if (bytes_read < 0) {
            printf("Failed to read from socket\n");
            linux_close(sock_fd);
            return 1;
        } else if (bytes_read == 0) {
            printf("EOF\n");
            break;
        }

        printf("%zu bytes l->w\n", bytes_read);

        DWORD total_written = 0, cbWritten = 0;

        while (total_written < (DWORD)bytes_read) {
            BOOL fSuccess = WriteFile(
                    hPipe,        // handle to pipe
                    buf + total_written,     // buffer to write from
                    bytes_read - total_written, // number of bytes to write
                    &cbWritten,   // number of bytes written
                    NULL);        // not overlapped I/O
            if (!fSuccess) {
                printf("Failed to write to pipe\n");
                return 1;
            }
            total_written += cbWritten;
            cbWritten = 0;
        }
    }
    return 1;
}


int _tmain() {

    BOOL    fConnected   = FALSE;
    DWORD   dwThreadId   = 0;
    HANDLE  hThread      = NULL;
    LPCTSTR lpszPipename = TEXT("\\\\.\\pipe\\discord-ipc-0");

    _tprintf(TEXT("Pipe Server: Main thread awaiting client connection on %s\n"), lpszPipename);

    // https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createnamedpipea
    hPipe = CreateNamedPipe(        
            lpszPipename,           // pipe name
            PIPE_ACCESS_DUPLEX,     // read/write access
            PIPE_TYPE_BYTE |        // message type pipe
            PIPE_READMODE_BYTE |    // message-read mode
            PIPE_WAIT,              // blocking mode
            1,                      // max. instances
            BUFSIZE,                // output buffer size
            BUFSIZE,                // input buffer size
            0,                      // client time-out
            NULL                    // default security attribute
        );

    if (hPipe == INVALID_HANDLE_VALUE) {
        _tprintf(TEXT("CreateNamedPipe failed, GLE=%lu.\n"), GetLastError());
        return -1;
    }

    // Wait for the client to connect; if it succeeds,
    // the function returns a nonzero value. If the function
    // returns zero, GetLastError returns ERROR_PIPE_CONNECTED.

    fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

    if (fConnected) {
        printf("Client connected\n");
        printf("Creating socket\n");

        if ((sock_fd = linux_socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
            printf("Failed to create socket: %s\n", strerror(-sock_fd));
            return 1;
        }

        sockaddr_un sock_addr = {0};
        sock_addr.sun_family = AF_UNIX;

        const char *const temp_path = get_sock_parent_path();

        char *sock_path_templates[] = {
            "%s/discord-ipc-%d",
            "%s/app/com.discordapp.Discord/discord-ipc-%d",
            "%s/snap.discord-canary/discord-ipc-%d",
            "%s/snap.discord/discord-ipc-%d"
        };

        int error = 0;

        for (size_t i = 0; i < ARRLEN(sock_path_templates); i++) {
            for (int pipe = 0; pipe <= 9; pipe++) {

                snprintf(sock_addr.sun_path, sizeof(sock_addr.sun_path), sock_path_templates[i], temp_path, pipe);
                printf("Attempting to connect to %s\n", sock_addr.sun_path);

                if ((error = linux_connect(sock_fd, (sockaddr*)&sock_addr, sizeof(sock_addr))) < 0)
                    printf("Failed to connect: %s\n", strerror(-error));
                else
                    goto breakout;
            }
        }


breakout:

        if (error) {
            printf("Could not connect to discord client\n");
            return 1;
        }

        printf("Connected successfully\n");

        hThread = CreateThread(
                NULL,              // no security attribute
                0,                 // default stack size
                winwrite_thread,    // thread proc
                (LPVOID) NULL,    // thread parameter
                0,                 // not suspended
                &dwThreadId);      // returns thread ID

        if (hThread == NULL)
        {
            _tprintf(TEXT("CreateThread failed, GLE=%lu.\n"), GetLastError());
            return 1;
        }


        while (TRUE) {
            char buf[BUFSIZE];
            DWORD bytes_read = 0;
            BOOL fSuccess = ReadFile(
                    hPipe,          // handle to pipe
                    buf,            // buffer to receive data
                    BUFSIZE,        // size of buffer
                    &bytes_read,    // number of bytes read
                    NULL            // not overlapped I/O
                );

            if (!fSuccess) {
                if (GetLastError() == ERROR_BROKEN_PIPE) {
                    printf("winread EOF\n");
                    return 0;
                } else {
                    printf("Failed to read from pipe\n");
                    return 1;
                }
            }

            printf("%lu bytes w->l\n", bytes_read);

            long unsigned int total_written = 0;
            int written = 0;

            while (total_written < bytes_read) {
                written = linux_write(sock_fd, buf + total_written, bytes_read - total_written);
                if (written < 0) {
                    printf("Failed to write to socket\n");
                    return 1;
                }
                total_written += written;
                written = 0;
            }
        }
    }
    else
        // The client could not connect, so close the pipe.
        CloseHandle(hPipe);

    return 0;


}