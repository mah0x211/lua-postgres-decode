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

#include "lua_postgres_decode.h"

static int decode_float_lua(lua_State *L)
{
    static const char *op = "postgres.decode.float";
    size_t len            = 0;
    char *str             = (char *)lauxh_checklstring(L, 1, &len);
    char *endptr          = str;
    double fv             = 0;

    lua_settop(L, 1);
    DECODE_START(L, op, str, len);
    switch (*str) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '+':
    case '-':
        fv = decode_str2dbl(str, &endptr);
        if (errno) {
            return decode_error(L, op, errno, NULL);
        }
    }
    DECODE_END(endptr);

    lua_pushnumber(L, fv);
    return 1;
}

LUALIB_API int luaopen_postgres_decode_float(lua_State *L)
{
    lua_pushcfunction(L, decode_float_lua);
    return 1;
}
