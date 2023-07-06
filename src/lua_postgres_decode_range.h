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

#ifndef lua_postgres_decode_range_h
#define lua_postgres_decode_range_h

#include "lua_postgres_decode.h"

// 8.17. Range Types
// https://www.postgresql.org/docs/current/rangetypes.html

/**
 * @brief decode_range_item
 *  decode each range item by callback function, and return the number of return
 *  values. the callback function must be placed at index 2 of the stack.
 * @param L
 * @param token
 * @param len
 */
static inline void decode_range_item(lua_State *L, const char *token,
                                     size_t len)
{
    // call function
    lua_pushvalue(L, 2); // passed function
    lua_pushlstring(L, token, len);
    lua_pushboolean(L, *token == '"');
    lua_pushvalue(L, 3); // passed arg
    lua_call(L, 3, 2);
}

/**
 * @brief decode_range
 *  decode range string.
 *  if pos is not NULL, then pos is used as the start position.
 * @param L
 * @param op operation name for error message
 * @param src source string
 * @param len source string length
 * @param pos start position of source string or NULL
 * @return char* next position of source string or NULL on error, when error
 * then nil and error message are pushed to the stack.
 */
static char *decode_range(lua_State *L, const char *op, char *src, char *pos)
{
    char *str        = (pos) ? pos : src;
    char *token      = NULL;
    size_t token_len = 0;
    int ntoken       = 0;
    int lower_inc    = 0;
    int upper_inc    = 0;

    lua_newtable(L);
    // skip spaces
    str = decode_skip_space(str);
    if (!*str) {
        decode_error(L, op, EINVAL, "empty string");
        return NULL;
    } else if (strncasecmp(str, "empty", 5) == 0) {
        // its empty range
        str = decode_skip_space(str + 5);
        goto DONE;
    }

    switch (*str) {
    default:
        decode_error_at(L, op, EILSEQ, src, str);
        return NULL;

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
        decode_error(L, op, EILSEQ, "malformed range string");
        return NULL;

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
            decode_error_at(L, op, EILSEQ, src, str);
            return NULL;
        }
        break;

    case ']':
        upper_inc = 1;
    case ')':
        if (!ntoken) {
            // first token not yet processed
            decode_error_at(L, op, EILSEQ, src, str);
            return NULL;
        }
        break;
    }

    // call function
    token_len = str - token;
    ntoken++;
    if (token_len) {
        decode_range_item(L, token, token_len);
        if (!lua_isnil(L, -1)) {
            // function returns multiple values
            decode_error(L, op, EILSEQ, lua_tostring(L, -1));
            return NULL;
        }
        lua_pop(L, 1);
        if (lua_isnil(L, -1)) {
            if (ntoken == 1) {
                lower_inc = 0;
            } else {
                upper_inc = 0;
            }
        }
        lua_rawseti(L, -2, ntoken);
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

    return str;
}

#endif
