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

#include "lua_postgres_decode_geom.h"

// 8.8.4. Boxes
// https://www.postgresql.org/docs/current/datatype-geometric.html#id-1.5.7.16.8

static int decode_box_lua(lua_State *L)
{
    static const char *op = "postgres.decode.box";
    size_t len            = 0;
    char *str             = (char *)lauxh_checklstring(L, 1, &len);
    double x[2]           = {0};
    double y[2]           = {0};

    lua_settop(L, 1);
    // box: (x1,y1), (x2,y2)
    DECODE_START(L, op, str, len);
    GEOM_STR2POINT(str, x[0], y[0]);
    GEOM_SKIP_DELIM(str, ',', "separator not found");
    GEOM_STR2POINT(str, x[1], y[1]);
    DECODE_END(str);

    lua_createtable(L, 2, 0);
    for (int i = 0; i < 2; i++) {
        lua_createtable(L, 2, 0);
        lauxh_pushnum2arr(L, 1, x[i]);
        lauxh_pushnum2arr(L, 2, y[i]);
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

LUALIB_API int luaopen_postgres_decode_box(lua_State *L)
{
    lua_pushcfunction(L, decode_box_lua);
    return 1;
}
