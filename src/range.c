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

#include "lua_postgres_decode_range.h"

static int decode_range_lua(lua_State *L)
{
    static const char *op = "postgres.decode.range";
    size_t len            = 0;
    const char *src       = lauxh_checklstring(L, 1, &len);

    luaL_checktype(L, 2, LUA_TFUNCTION);
    if (lua_gettop(L) < 3) {
        lua_pushnil(L);
    }
    lua_settop(L, 3);

    DECODE_START(L, op, src, len);
    src = decode_range(L, op, (char *)src, NULL);
    if (!src) {
        return 2;
    }
    DECODE_END(src);

    return 1;
}

LUALIB_API int luaopen_postgres_decode_range(lua_State *L)
{
    lua_errno_loadlib(L);
    lua_pushcfunction(L, decode_range_lua);
    return 1;
}
