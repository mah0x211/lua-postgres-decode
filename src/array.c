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

#define MAX_ARRAY_DEPTH 64

#define SKIP_DELIM_EX(s, delim, open_delim, skip_space, ...)                   \
    do {                                                                       \
        (s) = decode_skip_delim((s), (delim), (open_delim), skip_space);       \
        if (!(s)) {                                                            \
            return decode_error((L), (op), EILSEQ, __VA_ARGS__);               \
        }                                                                      \
    } while (0)

#define SKIP_DELIM(s, delim, ...) SKIP_DELIM_EX(s, delim, 0, 1, __VA_ARGS__)

static void decode_array_item(lua_State *L, const char *token, size_t len)
{
    // call function
    lua_pushvalue(L, 2); // passed function
    lua_pushlstring(L, token, len);
    lua_pushboolean(L, *token == '"');
    lua_pushvalue(L, 3); // passed arg
    lua_call(L, 3, 2);
}

static int decode_array_lua(lua_State *L)
{
    static const char *op           = "postgres.decode.array";
    size_t len                      = 0;
    char *src                       = (char *)lauxh_checklstring(L, 1, &len);
    char *str                       = src;
    char delim                      = ',';
    int depth                       = 0;
    int arrlen[MAX_ARRAY_DEPTH + 1] = {0};
    const char *token               = NULL;
    size_t token_len                = 0;

    luaL_checktype(L, 2, LUA_TFUNCTION);
    if (lua_gettop(L) < 3) {
        lua_pushnil(L);
    } else if (lua_gettop(L) > 3) {
        size_t delim_len = 1;
        char *delim_str  = (char *)lauxh_optlstring(L, 4, ",", &delim_len);
        if (delim_len != 1) {
            return decode_error(L, op, EINVAL, "delimiter must be a character");
        }
        delim = *delim_str;
    }

    lua_settop(L, 3);
    lua_newtable(L);

    // skip spaces
    str = decode_skip_space(str);
    if (!*str) {
        return decode_error(L, op, EINVAL, "empty string");
    } else if (*str != '{') {
        return decode_error(L, op, EILSEQ, "opening curly bracket not found");
    }
    str = decode_skip_space(str + 1);
    depth++;
    arrlen[depth] = 0;

NEXT_ELEMENT:
    switch (*str) {
    case 0:
        return decode_error(L, op, EILSEQ, "malformed array string");

    case '{':
        // found nested array
        depth++;
        if (depth > MAX_ARRAY_DEPTH || !lua_checkstack(L, 1)) {
            return decode_error(L, op, EILSEQ, "nesting level %d/%d too deep",
                                depth, MAX_ARRAY_DEPTH);
        }
        arrlen[depth] = 0;
        lua_newtable(L);
        str = decode_skip_space(str + 1);
        goto NEXT_ELEMENT;

    case '}':
        // found end of array
        depth--;
        str = decode_skip_space(str + 1);
        if (depth) {
            // end of nested array
            arrlen[depth]++;
            lua_rawseti(L, -2, arrlen[depth]);
            if (*str == delim) {
                // skip comma
                str = decode_skip_space(str + 1);
            }
            goto NEXT_ELEMENT;
        }
        // end of array
        if (*str) {
            return decode_error_at(L, op, EILSEQ, src, str);
        }
        lua_settop(L, 4);
        return 1;

    case '"':
        // found quoted value
        token = str;
        str++;
        // search closing quotation
        while (*str != '"') {
            if (!*str) {
                return decode_error(L, op, EILSEQ,
                                    "closing quotation not found");
            } else if (*str == '\\') {
                // skip escaped character
                str++;
            }
            str++;
        }
        str++;
        token_len = str - token;
        break;

    default:
        if (*str == delim) {
            // empty elements are not allowed
            return decode_error(L, op, EILSEQ,
                                "empty elements are not allowed");
        }

        // found unquoted value
        token = str;
        while (*str != ' ' && *str != delim && *str != '}') {
            if (!*str) {
                return decode_error(L, op, EILSEQ, "malformed array string");
            }
            str++;
        }
        token_len = str - token;
        // check for NULL
        if (token_len == 4 && strncasecmp(token, "NULL", token_len) == 0) {
            arrlen[depth]++;
            lua_pushnil(L);
            lua_rawseti(L, -2, arrlen[depth]);
            goto CHECK_DELIMITER;
        }
        break;
    }

    // call function
    decode_array_item(L, token, token_len);
    // check for error
    if (!lua_isnil(L, -1)) {
        return decode_error(L, op, EILSEQ, lua_tostring(L, -1));
    }
    lua_pop(L, 1);
    arrlen[depth]++;
    lua_rawseti(L, -2, arrlen[depth]);

CHECK_DELIMITER:
    // next delimiter must be delim or '}'
    str = decode_skip_space(str);
    if (*str == delim) {
        str = decode_skip_space(str + 1);
    } else if (*str != '}') {
        return decode_error_at(L, op, EILSEQ, src, str);
    }
    goto NEXT_ELEMENT;
}

LUALIB_API int luaopen_postgres_decode_array(lua_State *L)
{
    lua_errno_loadlib(L);
    lua_pushcfunction(L, decode_array_lua);
    return 1;
}
