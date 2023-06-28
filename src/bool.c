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

// 8.6. Boolean Type
// https://www.postgresql.org/docs/current/datatype-boolean.html

static int decode_bool_lua(lua_State *L)
{
    static const char *op = "postgres.decode.bool";
    size_t len            = 0;
    const char *str       = lauxh_checklstring(L, 1, &len);
    int boolv             = 0;

    lua_settop(L, 1);
    if (len > 1) {
        return decode_error_at(L, op, EILSEQ, str, str + 1);
    }

    // boolean: t or f
    DECODE_START(L, op, str, len);
    switch (*str) {
    case 't':
        boolv = 1;
    case 'f':
        break;
    default:
        return decode_error_at(L, op, EILSEQ, str, str);
    }
    DECODE_END(str + 1);

    lua_pushboolean(L, boolv);
    return 1;
}

LUALIB_API int luaopen_postgres_decode_bool(lua_State *L)
{
    lua_errno_loadlib(L);
    lua_pushcfunction(L, decode_bool_lua);
    return 1;
}
