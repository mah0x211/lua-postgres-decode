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

// 8.10. Bit String Types
// https : // www.postgresql.org/docs/current/datatype-bit.html

static int decode_bit_lua(lua_State *L)
{
    static const char *op = "postgres.decode.bit";
    size_t len            = 0;
    const char *str       = lauxh_checklstring(L, 1, &len);

    lua_settop(L, 1);
    if (!len) {
        return decode_error(L, op, EINVAL, "empty string");
    }

    for (size_t i = 0; i < len; i++) {
        if (str[i] != '0' && str[i] != '1') {
            return decode_error_at(L, op, EILSEQ, str, str + i);
        }
    }

    return 1;
}

LUALIB_API int luaopen_postgres_decode_bit(lua_State *L)
{
    lua_pushcfunction(L, decode_bit_lua);
    return 1;
}
