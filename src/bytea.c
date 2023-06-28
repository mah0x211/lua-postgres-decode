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

// 8.4. Binary Data Types
// https://www.postgresql.org/docs/current/datatype-binary.html

static int decode_bytea_lua(lua_State *L)
{
    static const char *op = "postgres.decode.bytea";
    size_t len            = 0;
    const char *str       = lauxh_checklstring(L, 1, &len);
    int is_escape         = lauxh_optboolean(L, 2, 0);
    size_t i              = 0;

    lua_settop(L, 1);
    if (is_escape) {
        return 1;
    }

    // hex format
    if (!len) {
        return decode_error(L, op, EINVAL, "empty string");
    } else if (str[0] != '\\') {
        return decode_error_at(L, op, EILSEQ, str, str);
    } else if (str[1] != 'x') {
        return decode_error_at(L, op, EILSEQ, str, str + 1);
    }
    for (i = 2; i < len; i++) {
        if (!isxdigit(str[i])) {
            return decode_error_at(L, op, EILSEQ, str, str + i);
        }
    }
    return 1;
}

LUALIB_API int luaopen_postgres_decode_bytea(lua_State *L)
{
    lua_errno_loadlib(L);
    lua_pushcfunction(L, decode_bytea_lua);
    return 1;
}
