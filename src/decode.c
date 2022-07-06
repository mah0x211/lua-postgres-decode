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

LUALIB_API int luaopen_postgres_decode(lua_State *L)
{
    lua_errno_loadlib(L);
    lua_newtable(L);
    lauxh_pushfn2tbl(L, "bit", decode_bit_lua);
    lauxh_pushfn2tbl(L, "bytea", decode_bytea_lua);
    lauxh_pushfn2tbl(L, "bool", decode_bool_lua);
    lauxh_pushfn2tbl(L, "float", decode_float_lua);
    lauxh_pushfn2tbl(L, "int", decode_int_lua);
    lauxh_pushfn2tbl(L, "date", decode_date_lua);
    lauxh_pushfn2tbl(L, "time", decode_time_lua);
    lauxh_pushfn2tbl(L, "timetz", decode_timetz_lua);
    lauxh_pushfn2tbl(L, "point", decode_point_lua);
    lauxh_pushfn2tbl(L, "line", decode_line_lua);
    lauxh_pushfn2tbl(L, "lseg", decode_lseg_lua);
    lauxh_pushfn2tbl(L, "box", decode_box_lua);
    lauxh_pushfn2tbl(L, "path", decode_path_lua);
    lauxh_pushfn2tbl(L, "polygon", decode_polygon_lua);
    lauxh_pushfn2tbl(L, "circle", decode_circle_lua);
    lauxh_pushfn2tbl(L, "array", decode_array_lua);
    return 1;
}
