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

// F.18. hstore
// https://www.postgresql.org/docs/current/hstore.html

static int decode_hstore_lua(lua_State *L)
{
    static const char *op = "postgres.decode.polygon";
    size_t len            = 0;
    char *str             = (char *)lauxh_checklstring(L, 1, &len);
    char *chunk           = NULL;

    lua_settop(L, 1);
    lua_newtable(L);
    // hstore: "key"=>"value", ... "keyn"=>"valuen"
    DECODE_START(L, op, str, len);

#define SKIP_DELIM(s, delim, errmsg)                                           \
    do {                                                                       \
        (s) = decode_skip_delim((s), delim, 0, 0);                             \
        if (!(s)) {                                                            \
            return decode_error(L, op, EILSEQ, errmsg);                        \
        }                                                                      \
    } while (0)

CHECK_NEXT:
    // key
    SKIP_DELIM(str, '"', "opening double-quote not found");
    chunk = str;
    SKIP_DELIM(str, '"', "closing double-quote not found");
    lua_pushlstring(L, chunk, str - chunk - 1);
    str = decode_skip_space(str);
    // separator: =>
    if (str[0] != '=' || str[1] != '>') {
        return decode_error(L, op, EILSEQ, "key-value separator not found");
    }
    str += 2;
    if (*str == 'N') {
        // NULL value
        if (str[1] != 'U' || str[2] != 'L' || str[3] != 'L') {
            return decode_error(L, op, EILSEQ, "invalid null value");
        }
        str += 4;
        // remove pushed key
        lua_pop(L, 1);
    } else {
        // quoted value
        SKIP_DELIM(str, '"', "opening double-quote not found");
        chunk = str;
        SKIP_DELIM(str, '"', "closing double-quote not found");
        // set key-value pair
        lua_pushlstring(L, chunk, str - chunk - 1);
        lua_rawset(L, -3);
    }
    str = decode_skip_space(str);
    if (*str == ',') {
        str++;
        goto CHECK_NEXT;
    }
    DECODE_END(str);

    return 1;
}

LUALIB_API int luaopen_postgres_decode_hstore(lua_State *L)
{
    lua_errno_loadlib(L);
    lua_pushcfunction(L, decode_hstore_lua);
    return 1;
}
