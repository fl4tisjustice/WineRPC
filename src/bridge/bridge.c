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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "bridge/utils/linux_utils.h"
#include "bridge/utils/windows_utils.h"
#include "bridge/utils/arg_parser.h"
#include "bridge/log.h"

#define ARRLEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define AF_UNIX     1
#define SOCK_STREAM 1
#define BUFSIZE     2048 // size of read/write buffers

static HANDLE hPipe;
static int sock_fd;

enum log_level g_log_level = _INVALID;

static const char* get_sock_parent_path();
DWORD WINAPI winwrite_thread();

int main(int argc, char *argv[])  {
    int exit_code = EXIT_SUCCESS;

    parse_args(argc, argv);
    
    if (g_log_level == _INVALID) {
        g_log_level = LL_WARNING;
        bridge_log(LL_WARNING, "Log level not set, assuming \"none\".\n");
        g_log_level = LL_NONE;
    }

    DWORD   dwThreadId          = 0;
    DWORD   dwThreadExitCode    = 0;
    HANDLE  hThread             = NULL;
    LPCSTR  lpszPipename        = "\\\\.\\pipe\\discord-ipc-0";

    bridge_log(LL_INFO, "Creating named pipe for connection to RPC client at \"%s\".\n", lpszPipename);

    // https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createnamedpipea
    hPipe = CreateNamedPipeA(
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
        LPTSTR lpBuffer = GetLastErrorAsString();
        bridge_log(LL_ERROR, "Failed to create named pipe: %s", lpBuffer); 
        LocalFree(lpBuffer);
        exit_code = EXIT_FAILURE; goto exit;
    }

    bridge_log(LL_INFO, "Successfully created named pipe.\n");
    bridge_log(LL_INFO, "Awaiting connection from RPC client.\n");

    BOOL fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

    if (!fConnected) {
        LPTSTR lpBuffer = GetLastErrorAsString();
        bridge_log(LL_ERROR, "Failed to connect to RPC client: %s", lpBuffer);
        LocalFree(lpBuffer);
        exit_code = EXIT_FAILURE; goto close_pipe;
    }

    bridge_log(LL_INFO, "Successfully connected to RPC client.\n");
    bridge_log(LL_INFO, "Creating socket to Discord client.\n");

    if ((sock_fd = linux_socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        bridge_log(LL_ERROR, "Failed to create socket: %s.\n", strerror(-sock_fd));
        exit_code = EXIT_FAILURE; goto close_pipe;
    }

    sockaddr_un sock_addr = {0};
    sock_addr.sun_family = AF_UNIX;

    const char *temp_path = get_sock_parent_path();

    const char *sock_path_templates[] = {
        "%s/discord-ipc-%d",
        "%s/app/com.discordapp.Discord/discord-ipc-%d",
        "%s/snap.discord-canary/discord-ipc-%d",
        "%s/snap.discord/discord-ipc-%d"
    };

    int error = 0;

    for (size_t i = 0; i < ARRLEN(sock_path_templates); i++) {
        for (int pipe = 0; pipe <= 9; pipe++) {
            snprintf(sock_addr.sun_path, sizeof(sock_addr.sun_path), sock_path_templates[i], temp_path, pipe);
            bridge_log(LL_INFO, "Attempting to connect to socket at \"%s\".\n", sock_addr.sun_path);
            if ((error = linux_connect(sock_fd, (sockaddr*)&sock_addr, sizeof(sock_addr))) < 0)
                bridge_log(LL_WARNING, "Failed to connect to socket: %s\n", strerror(-error));
            else
                goto breakout;
        }
    }


breakout:

    if (error) {
        bridge_log(LL_ERROR, "Could not connect to a Discord client.\n");
        exit_code = EXIT_FAILURE; goto close_socket;
    }

    bridge_log(LL_INFO, "Successfully connected to Discord client.\n");

    hThread = CreateThread(
        NULL,               // no security attribute
        0,                  // default stack size
        winwrite_thread,    // thread proc
        (LPVOID) NULL,      // thread parameter
        0,                  // not suspended
        &dwThreadId         // returns thread ID
    );

    if (hThread == NULL) {
        LPTSTR lpBuffer = GetLastErrorAsString();
        bridge_log(LL_ERROR, "Failed to create thread: %s", lpBuffer); 
        LocalFree(lpBuffer);
        exit_code = EXIT_FAILURE; goto close_socket;
    }

    while (TRUE) {
        char buf[BUFSIZE];
        DWORD bytes_read = 0;

        // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfile
        BOOL fSuccess = ReadFile(
            hPipe,          // Pipe handle
            buf,            // Buffer to receive data
            BUFSIZE,        // Buffer size
            &bytes_read,    // Pointer to receive number of bytes read
            NULL            // Not asynchronous
        );

        if (!fSuccess) {
            DWORD dwError = GetLastError();
            if (dwError == ERROR_BROKEN_PIPE) {
                bridge_log(LL_WARNING, "Connection closed by RPC client.\n");
                goto cleanup;
            } else if (dwError != ERROR_OPERATION_ABORTED) {
                LPTSTR lpBuffer = GetLastErrorAsString();
                bridge_log(LL_ERROR, "Failed to read from named pipe: %s");
                LocalFree(lpBuffer);
                exit_code = EXIT_FAILURE;
            }
            goto cleanup;
        }

        bridge_log(LL_INFO, "%lu bytes written from RPC client to Discord client.\n", bytes_read);

        long unsigned int total_written = 0;
        int written = 0;

        while (total_written < bytes_read) {
            written = linux_write(sock_fd, buf + total_written, bytes_read - total_written);
            if (written < 0) {
                bridge_log(LL_ERROR, "Failed to write to socket: %s.\n", strerror(-written));
                exit_code = EXIT_FAILURE; goto cleanup;
            }
            total_written += written;
            written = 0;
        }
    }

cleanup:
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobject
    // Thread will always exit as both it and the main thread rely on functioning socket and pipe
    // If either breaks both will exit, but we wait out of courtesy
    (VOID)WaitForSingleObject(hPipe, INFINITE);

    // https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getexitcodethread
    // Already exited, so just grab the code
    (VOID)GetExitCodeThread(hThread, &dwThreadExitCode);

    // Only exit with a return code of 0 when neither thread caught an error
    exit_code = (int)(exit_code || dwThreadExitCode);
close_socket:
    linux_close(sock_fd);
close_pipe:
    CloseHandle(hPipe);
exit:
    return exit_code;
}

static const char* get_sock_parent_path() {
    const char *env_tmp_paths[] = {"XDG_RUNTIME_DIR", "TMPDIR", "TMP", "TEMP"};
    char *path;

    for (size_t i = 0; i < ARRLEN(env_tmp_paths); i++)
        if ((path = getenv(env_tmp_paths[i])))
            return path;
    
    return "/tmp";
}


DWORD WINAPI winwrite_thread() {
    DWORD dwExitCode = EXIT_SUCCESS;

    while (TRUE) {
        char buf[BUFSIZE];
        ssize_t bytes_read = linux_read(sock_fd, buf, BUFSIZE);

        if (bytes_read < 0) {
            bridge_log(LL_ERROR, "Failed to read from socket: %s.\n", strerror(-bytes_read));
            dwExitCode = EXIT_FAILURE;
            break;
        } else if (bytes_read == 0) {
            bridge_log(LL_WARNING, "Connection closed by Discord client.\n");
            dwExitCode = EXIT_FAILURE;
            break;
        }

        bridge_log(LL_INFO, "%zu bytes written from Discord client to RPC client.\n", bytes_read);

        DWORD total_written = 0, cbWritten = 0;

        // Positive value so safe conversion
        while (total_written < (DWORD)bytes_read) {
            // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
            BOOL fSuccess = WriteFile(
                hPipe,                      // Pipe handle
                buf + total_written,        // Buffer to write from
                bytes_read - total_written, // Remaining unwritten bytes
                &cbWritten,                 // Pointer to receive number of bytes written
                NULL                        // not overlapped I/O
            );
                                  
            if (!fSuccess) {
                LPTSTR lpBuffer = GetLastErrorAsString();
                bridge_log(LL_ERROR, "Failed to write to named pipe: %s", lpBuffer);
                LocalFree(lpBuffer);
                break;
            }

            total_written += cbWritten;
            cbWritten = 0;
        }
    }

    // https://learn.microsoft.com/en-us/windows/win32/fileio/cancelioex-func
    // Signals current process to stop all WinAPI I/O operations
    CancelIoEx(hPipe, NULL);
    return dwExitCode;
}
