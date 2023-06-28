/**
 *  Copyright (C) 2022 Masatoshi Fukunaga
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */

#include "lua_postgres_decode_datetime.h"

static int decode_timestamp_lua(lua_State *L)
{
    size_t len           = 0;
    const char *str      = lauxh_checklstring(L, 1, &len);
    datum_timestamp_t ts = {0};

    lua_settop(L, 1);
    if (decode_timestamp(&ts, L, "postgres.decode.timestamp", str, len)) {
        return 2;
    }

    lua_createtable(L, 0, 4);
    lauxh_pushint2tbl(L, "year", ts.year);
    lauxh_pushint2tbl(L, "month", ts.mon);
    lauxh_pushint2tbl(L, "day", ts.day);
    lauxh_pushint2tbl(L, "hour", ts.hour);
    lauxh_pushint2tbl(L, "min", ts.min);
    lauxh_pushint2tbl(L, "sec", ts.sec);
    lauxh_pushint2tbl(L, "usec", ts.usec);
    if (ts.tzsign[0]) {
        lauxh_pushstr2tbl(L, "tz", ts.tzsign);
        lauxh_pushint2tbl(L, "tzhour", ts.tzhour);
        lauxh_pushint2tbl(L, "tzmin", ts.tzmin);
        lauxh_pushint2tbl(L, "tzsec", ts.tzsec);
    }

    return 1;
}

LUALIB_API int luaopen_postgres_decode_timestamp(lua_State *L)
{
    lua_errno_loadlib(L);
    lua_pushcfunction(L, decode_timestamp_lua);
    return 1;
}
