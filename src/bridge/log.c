#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <tchar.h>

#include "bridge/log.h"
#include "bridge/utils/windows_utils.h"

void bridge_log(enum log_level log_level, const char *fmt, ...) {

    if (g_log_level < log_level) return;

    switch (log_level) {
        case LL_ERROR:
            winprintf("[ERROR]   ");
            break;
        case LL_WARNING:
            winprintf("[WARNING] ");
            break;            
        case LL_INFO:
            winprintf("[INFO]    ");
            break;
        case LL_DEBUG:
            winprintf("[DEBUG]   ");
            break;
        case LL_TRACE:
            winprintf("[TRACE]   ");
            break;
        default:
            assert(0 && "Invalid log level");
    }

    va_list args;

    va_start(args, fmt);
    _vtprintf(_T(fmt), args);
    va_end(args);
}