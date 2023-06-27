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

// 8.8.6. Polygons
// https://www.postgresql.org/docs/current/datatype-geometric.html#DATATYPE-POLYGON

static int decode_polygon_lua(lua_State *L)
{
    static const char *op = "postgres.decode.polygon";
    size_t len            = 0;
    char *str             = (char *)lauxh_checklstring(L, 1, &len);
    int idx               = 0;
    double x, y;

    lua_settop(L, 1);
    lua_newtable(L);
    // polygon: ((x1, y1), ... (xn, yn))
    DECODE_START(L, op, str, len);
    GEOM_SKIP_DELIM(str, '(', "opening round bracket not found");

CHECK_NEXT:
    GEOM_STR2POINT(str, x, y);
    lua_createtable(L, 2, 0);
    lauxh_pushnum2arr(L, 1, x);
    lauxh_pushnum2arr(L, 2, y);
    lua_rawseti(L, -2, ++idx);
    if (*str == ',') {
        str = decode_skip_space(str + 1);
        goto CHECK_NEXT;
    }
    GEOM_SKIP_DELIM(str, ')', "closing round bracket not found");
    DECODE_END(str);

    return 1;
}

LUALIB_API int luaopen_postgres_decode_polygon(lua_State *L)
{
    lua_pushcfunction(L, decode_polygon_lua);
    return 1;
}
