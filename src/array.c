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

// int decode_array_lua(lua_State *L);
typedef int (*decode_array_cb)(void *ctx, lua_State *L, const char *op,
                               const char *str, size_t len);

int decode_array(lua_State *L, const char *op, const char *str, size_t len,
                 decode_array_cb cbfn, void *ctx)
{
    const int top                   = lua_gettop(L);
    char *s                         = (char *)str;
    int depth                       = 0;
    int arrlen[MAX_ARRAY_DEPTH + 1] = {0};
    const char *token               = NULL;
    int retval                      = 0;

    if (!len) {
        return decode_error(L, op, EINVAL, "empty string");
    }

    // skip spaces
    SKIP_DELIM(s, '{', "opening curly bracket not found");
    depth++;
    arrlen[depth] = 0;
    lua_newtable(L);

CHECK_TOKEN:
    switch (*s) {
    case 0:
        return decode_error(L, op, EILSEQ, "malformed array string");

    case '{':
        depth++;
        if (depth > MAX_ARRAY_DEPTH) {
            return decode_error(L, op, EILSEQ, "nesting level %d/%d too deep",
                                depth, MAX_ARRAY_DEPTH);
        }
        arrlen[depth] = 0;
        lua_newtable(L);
        s = decode_skip_space(s + 1);
        goto CHECK_TOKEN;

    case '}':
        if (!depth) {
            return decode_error(L, op, EILSEQ, "'%c' found at position %d", *s,
                                s - str);
        }
        depth--;
        s = decode_skip_space(s + 1);

        if (depth) {
            // end of nested array
            arrlen[depth]++;
            lua_rawseti(L, -2, arrlen[depth]);
            if (*s == ',') {
                s = decode_skip_space(s + 1);
                goto CHECK_ITEM;
            }
            goto CHECK_TOKEN;
        }

        // end of array
        if (*s) {
            return decode_error(L, op, EILSEQ, "'%c' found at position %d", *s,
                                s - str);
        }
        lua_settop(L, top + 1);
        return 1;
    }

CHECK_ITEM:
    switch (*s) {
    case '"':
        token = s;
        s++;
        SKIP_DELIM_EX(s, '"', 0, 0,
                      "closing quotation not found after position %d",
                      token - str);
        break;

    case '(':
        token = s;
        s++;
        SKIP_DELIM_EX(s, ')', '(', 0,
                      "closing round bracket not found after position %d",
                      token - str);
        break;

    case '[':
        token = s;
        s++;
        SKIP_DELIM_EX(s, ']', '[', 0,
                      "closing square bracket not found after position %d",
                      s - str);
        break;

    case '<':
        token = s;
        s++;
        SKIP_DELIM_EX(s, '>', '<', 0,
                      "closing angle bracket not found after position %d",
                      token - str);
        break;

    default:
        token = s;
        s++;
        while (*s) {
            if (*s == ',' || *s == '}' || *s == ' ') {
                break;
            } else if (*s == '\\') {
                if (s[1]) {
                    s++;
                }
            }
            s++;
        }
        if (!*s) {
            return decode_error(L, op, EILSEQ, "malformed array string");
        }
    }

    // call function
    retval = cbfn(ctx, L, op, token, s - token);
    switch (retval) {
    default:
        // function returns multiple values
        // remove 3rd and subsequent values
        lua_pop(L, retval - 2);
        return decode_error(L, op, EILSEQ, lua_tostring(L, -1));

    case 0:
        lua_pushnil(L);
    case 1:
        arrlen[depth]++;
        lua_rawseti(L, -2, arrlen[depth]);
    }

    s = decode_skip_space(s);
    if (*s == ',') {
        SKIP_DELIM(s, ',', NULL);
    } else if (*s != '}') {
        return decode_error_at(L, op, EILSEQ, str, s);
    }
    goto CHECK_TOKEN;
}

static int decode_array_item(void *ctx, lua_State *L, const char *op,
                             const char *token, size_t len)
{
    int top = lua_gettop(L);
    (void)ctx;
    (void)op;
    // call function
    lua_pushvalue(L, 2);
    lua_pushlstring(L, token, len);
    lua_call(L, 1, LUA_MULTRET);
    return lua_gettop(L) - top;
}

static int decode_array_lua(lua_State *L)
{
    size_t len      = 0;
    const char *str = lauxh_checklstring(L, 1, &len);

    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_settop(L, 2);
    return decode_array(L, "postgres.decode.array", str, len, decode_array_item,
                        NULL);
}

LUALIB_API int luaopen_postgres_decode_array(lua_State *L)
{
    lua_pushcfunction(L, decode_array_lua);
    return 1;
}
