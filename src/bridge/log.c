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

#include "bridge/log.h"
#include "bridge/utils/windows.h"

void bridge_log(enum log_level log_level, const char *fmt, ...) {
    if (g_log_level < log_level) return;

    switch (log_level) {
        case LL_ERROR:
            printf("[ERROR]   ");
            break;
        case LL_WARNING:
            printf("[WARNING] ");
            break;            
        case LL_INFO:
            printf("[INFO]    ");
            break;
        case LL_DEBUG:
            printf("[DEBUG]   ");
            break;
        case LL_TRACE:
            printf("[TRACE]   ");
            break;
        default:
            assert(0 && "Invalid log level");
    }

    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}