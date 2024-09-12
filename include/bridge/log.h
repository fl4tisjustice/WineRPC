#pragma once

#define BIT(n) (1 << n)

enum log_level {
    _INVALID = -1,
    LL_NONE,
    LL_ERROR,
    LL_WARNING,
    LL_INFO,
    LL_DEBUG,
    LL_TRACE
};

extern enum log_level g_log_level;

void bridge_log(enum log_level, const char *fmt, ...);
