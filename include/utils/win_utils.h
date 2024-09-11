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

#include <windef.h>
#include <tchar.h>

#define PrintLastError(lpErrorMessageTemplate)                              \
    do {                                                                    \
        LPTSTR _lpBuffer = GetLastErrorAsString();                          \
        _tprintf("%s%s", TEXT(lpErrorMessageTemplate), _lpBuffer);          \
        (VOID)LocalFree(_lpBuffer);                                         \
    } while (0);

LPTSTR GetLastErrorAsString(VOID);
