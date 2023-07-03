/**
 *  Copyright (C) 2023 Masatoshi Fukunaga
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

#define SKIP_DELIM(s, delim, ...)                                              \
    do {                                                                       \
        (s) = decode_skip_delim((s), (delim), 0, 0);                           \
        if (!(s)) {                                                            \
            return decode_error((L), (op), EILSEQ, __VA_ARGS__);               \
        }                                                                      \
    } while (0)

static int decode_tsvector_lua(lua_State *L)
{
    static const char *op = "postgres.decode.tsvector";
    size_t len            = 0;
    const char *tsv       = lauxh_checklstring(L, 1, &len);
    char *str             = (char *)tsv;
    char *chunk           = NULL;
    int nvec              = 0;

    lua_settop(L, 1);
    // create result table
    // {
    //     [1] = {
    //         lexem = 'foo',
    //         positions = { 1, 2, 3 },
    //         weights = { 'A', 'B', 'C' }
    //     },
    // }
    lua_newtable(L);

    // parse the following formats
    //  'foo' 'bar' 'baz'
    //  'fo''o' 'bar' 'baz'
    DECODE_START(L, op, str, len);

NEXT_LEXEME:
    SKIP_DELIM(str, '\'', "opening quote not found");
    chunk = str;
ESCAPE_QUOTE:
    SKIP_DELIM(str, '\'', "closing quote not found");
    if (*str == '\'') {
        str++;
        goto ESCAPE_QUOTE;
    }
    // create lexeme table
    lua_newtable(L);
    lua_pushliteral(L, "lexeme");
    lua_pushlstring(L, chunk, str - chunk - 1);
    lua_rawset(L, -3);

    // if the next character is ':', then parse the following formats
    //  'foo':1,2,3 'bar':4 'baz':1
    if (*str == ':') {
        char *endptr  = NULL;
        int nposition = 0;
        int nweight   = 0;
        intmax_t iv   = 0;

        // skip ':'
        str++;

        // positions table
        lua_pushliteral(L, "positions");
        lua_newtable(L);
        // weights table
        lua_pushliteral(L, "weights");
        lua_newtable(L);

NEXT_POSITION:
        // parse position
        iv = decode_str2imax(str, &endptr);
        if (str == endptr) {
            errno = EILSEQ;
        }
        if (errno) {
            return decode_error_at(L, op, errno, tsv, endptr);
        }
        // set position
        lua_pushinteger(L, iv);
        lua_rawseti(L, -4, ++nposition);
        str = endptr;

        // parse weight
        switch (*str) {
        case 'A':
        case 'B':
        case 'C':
            // set weight
            lua_pushlstring(L, str, 1);
            lua_rawseti(L, -2, nposition);
            nweight++;
            str++;
        }

        switch (*str) {
        case ',':
            str++;
            goto NEXT_POSITION;

        case ' ':
        case 0:
            break;

        default:
            errno = EILSEQ;
            return decode_error_at(L, op, errno, tsv, str);
        }

        // set weights table if not empty
        if (nweight) {
            lua_rawset(L, -5);
        } else {
            lua_pop(L, 2);
        }
        // set positions table
        lua_rawset(L, -3);
    }

    // add to result table
    lua_rawseti(L, -2, ++nvec);
    if (*str) {
        goto NEXT_LEXEME;
    }

    DECODE_END(str);

    return 1;
}

LUALIB_API int luaopen_postgres_decode_tsvector(lua_State *L)
{
    lua_errno_loadlib(L);
    lua_pushcfunction(L, decode_tsvector_lua);
    return 1;
}
