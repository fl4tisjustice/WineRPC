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
    {"log-level", required_argument, NULL,  'l'},
    {0,           0,                 0,   0 }
};

void parse_args(int argc, char *argv[]) {
    int c;
    while (1) {
        int option_index = 0;

        opterr = 0; // Disable error messages since we provide our own
        c = getopt_long(argc, argv, "", long_options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0:
                break;
            case 'l': {
                CMP_ARG_ASSIGN("none",    g_log_level, LL_NONE);
                CMP_ARG_ASSIGN("error",   g_log_level, LL_ERROR);
                CMP_ARG_ASSIGN("warning", g_log_level, LL_WARNING);
                CMP_ARG_ASSIGN("info",    g_log_level, LL_INFO);
                CMP_ARG_ASSIGN("debug",   g_log_level, LL_DEBUG);
                CMP_ARG_ASSIGN("trace",   g_log_level, LL_TRACE);
                // No match, meaning invalid value

                g_log_level = LL_ERROR;
                bridge_log(LL_ERROR, "Invalid log level. Please supply either no log value to silence the program or a valid one.\n");
                exit(EXIT_FAILURE);
            }
            case '?': // Unknown option
                g_log_level = LL_WARNING;
                bridge_log(LL_WARNING, "Unkown option given. Assuming log level set to \"none\".\n");
                g_log_level = LL_NONE;
                break;
            default:
                abort();
        }
    }

}