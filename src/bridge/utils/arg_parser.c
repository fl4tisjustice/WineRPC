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

#include <getopt.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "bridge/utils/arg_parser.h"
#include "bridge/log.h"

#define CMP_ARG_ASSIGN(cmp, var, val)               \
    if (strncmp(optarg, cmp, strlen(cmp)) == 0) {   \
        var = val;                                  \
        break;                                      \
    }

static const struct option long_options[] = {
    { "log-level", required_argument, NULL,  'l' },
    { "help",      no_argument,       NULL,  'h' },
    { 0,           0,                 0,      0  }
};

void parse_args(int argc, char *argv[]) {
    int c;
    while (1) {
        int option_index = 0;

        c = getopt_long(argc, argv, "hcw", long_options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0:
                break;
            case 'h':
                printf(
                    "WineRPC %s\n"
                    "Usage: winerpcbridge.exe [OPTION]\n"
                    "Launch through wine under a given prefix to allow Discord RPC to work\n"
                    "with an application/game running under the same prefix.\n\n"
                    
                    "  -h, --help                 Display this help message and exit.\n"
                    "      --log-level=LEVEL      Opt into select logging levels.\n"
                    "                             Available options are:\n"
                    "                             none, error, warning, info, debug, trace\n"
                    "                             An unspecified log level will assume that\n"
                    "                             \"none\" was selected.\n"
                    "  -w, --warranty             Display warranty info and exit.\n"
                    "  -c  --copyright            Display copyright info and exit.\n\n"

                    "Report bugs to <https://github.com/fl4tisjustice/WineRPC/issues>\n\n",
                    
                    VERSION
                );
                exit(EXIT_SUCCESS);
            case 'c':
                printf(
                    "Copyright (C) 2024  Leah Santos  <leahsantos@proton.me>\n\n"

                    "This program is free software: you can redistribute it and/or modify\n"
                    "it under the terms of the GNU General Public License as published by\n"
                    "the Free Software Foundation, either version 3 of the License, or\n"
                    "(at your option) any later version.\n\n"

                );
                exit(EXIT_SUCCESS);
            case 'w':
                printf(
                    "This program is distributed in the hope that it will be useful,\n"
                    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
                    "GNU General Public License for more details.\n\n"

                    "You should have received a copy of the GNU General Public License\n"
                    "along with this program.  If not, see <https://www.gnu.org/licenses/>.\n\n"

                    "The full license is available in the LICENSE file distributed with\n"
                    "the source code in the root of the project.\n\n"

                );
                exit(EXIT_SUCCESS);
            case 'l': {
                CMP_ARG_ASSIGN("none",    g_log_level, LL_NONE);
                CMP_ARG_ASSIGN("error",   g_log_level, LL_ERROR);
                CMP_ARG_ASSIGN("warning", g_log_level, LL_WARNING);
                CMP_ARG_ASSIGN("info",    g_log_level, LL_INFO);
                CMP_ARG_ASSIGN("debug",   g_log_level, LL_DEBUG);
                CMP_ARG_ASSIGN("trace",   g_log_level, LL_TRACE);
                // No match, meaning invalid value

                printf("Invalid log level. Please supply either no log value to silence the program or a valid one.\n");
                exit(EXIT_FAILURE);
            }
            case '?': // Unknown option
                printf("Try 'winerpcbridge.exe --help' for more information.");
                exit(EXIT_FAILURE);
            default:
                abort();
        }
    }
}