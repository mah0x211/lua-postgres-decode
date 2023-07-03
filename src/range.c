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

static int decode_range_item(lua_State *L, const char *token, size_t len)
{
    int top = lua_gettop(L);
    // call function
    lua_pushvalue(L, 2);
    lua_pushlstring(L, token, len);
    lua_call(L, 1, LUA_MULTRET);
    return lua_gettop(L) - top;
}

static int decode_range_lua(lua_State *L)
{
    static const char *op = "postgres.decode.range";
    size_t len            = 0;
    const char *src       = lauxh_checklstring(L, 1, &len);
    char *str             = (char *)src;
    char *token           = NULL;
    size_t token_len      = 0;
    int ntoken            = 0;
    int lower_inc         = 0;
    int upper_inc         = 0;
    int retval            = 0;

    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_settop(L, 2);
    lua_newtable(L);

    DECODE_START(L, op, str, len);
    // skip spaces
    str = decode_skip_space(str);

    if (!*str) {
        return decode_error(L, op, EINVAL, "empty string");
    } else if (strncasecmp(str, "empty", 5) == 0) {
        // its empty range
        str = decode_skip_space(str + 5);
        goto DONE;
    }

    switch (*str) {
    default:
        return decode_error_at(L, op, EILSEQ, src, str);

    case '[':
        lower_inc = 1;
    case '(':
        str   = decode_skip_space(str + 1);
        token = str;
    }

    // find delimiter or closing parenthesis
NEXT_CHAR:
    switch (*str) {
    case 0:
        return decode_error(L, op, EILSEQ, "malformed range string");

    case '{':
        str = decode_skip_delim(str + 1, '}', '{', 1);
        goto NEXT_CHAR;
    case '(':
        str = decode_skip_delim(str + 1, ')', '(', 1);
        goto NEXT_CHAR;
    case '[':
        str = decode_skip_delim(str + 1, ']', '[', 1);
        goto NEXT_CHAR;
    case '<':
        str = decode_skip_delim(str + 1, '>', '<', 1);
        goto NEXT_CHAR;

    default:
        str++;
        goto NEXT_CHAR;

    case ',':
        if (ntoken) {
            // first token already processed
            return decode_error_at(L, op, EILSEQ, src, str);
        }
        break;

    case ']':
        upper_inc = 1;
    case ')':
        if (!ntoken) {
            // first token not yet processed
            return decode_error_at(L, op, EILSEQ, src, str);
        }
        break;
    }

    // call function
    token_len = str - token;
    ntoken++;
    if (token_len) {
        retval = decode_range_item(L, token, token_len);
        switch (retval) {
        default:
            // function returns multiple values
            return decode_error(L, op, EILSEQ, lua_tostring(L, -1));

        case 0:
            lua_pushnil(L);
            if (ntoken == 1) {
                lower_inc = 0;
            } else {
                upper_inc = 0;
            }
        case 1:
            lua_rawseti(L, -2, ntoken);
        }
    } else if (ntoken == 1) {
        lower_inc = 0;
    } else {
        upper_inc = 0;
    }

    str = decode_skip_space(str + 1);
    if (ntoken < 2) {
        token = str;
        goto NEXT_CHAR;
    }

DONE:
    DECODE_END(str);

    if (lower_inc) {
        lua_pushliteral(L, "lower_inc");
        lua_pushboolean(L, 1);
        lua_rawset(L, -3);
    }
    if (upper_inc) {
        lua_pushliteral(L, "upper_inc");
        lua_pushboolean(L, 1);
        lua_rawset(L, -3);
    }

    return 1;
}

LUALIB_API int luaopen_postgres_decode_range(lua_State *L)
{
    lua_errno_loadlib(L);
    lua_pushcfunction(L, decode_range_lua);
    return 1;
}
