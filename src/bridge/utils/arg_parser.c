#include <getopt.h>
#include <assert.h>
#include <stdlib.h>

#include "bridge/utils/arg_parser.h"
#include "bridge/log.h"

#define CMP_ARG_ASSIGN(cmp, var, val)           \
    if (_tcscoll(optarg, _T(cmp)) == 0) {       \
        var = val;                              \
        break;                                  \
    }

static struct option long_options[] = {
    {"log-level", required_argument, 0,  'l'}
};

void parse_args(int argc, _TCHAR *argv[]) {
    int c;
    while (1) {
        int option_index = 0;

        c = getopt_long_only(argc, argv, "l", long_options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0: break;
            case 'l': {
                CMP_ARG_ASSIGN("none",  g_log_level, LL_NONE);
                CMP_ARG_ASSIGN("info",  g_log_level, LL_INFO);
                CMP_ARG_ASSIGN("trace", g_log_level, LL_TRACE);
                CMP_ARG_ASSIGN("debug", g_log_level, LL_DEBUG);
                assert(0 && "Invalid log level");
                break;
            }
            case '?':
                break;
            default:
                abort();
        }
    }

}