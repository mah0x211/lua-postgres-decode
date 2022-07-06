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

int decode_float(datum_t *v, lua_State *L, const char *op, const char *str,
                 size_t len)
{
    char *s      = (char *)str;
    char *endptr = s;

    if (!len) {
        return decode_error(L, op, EINVAL, "empty string");
    }
    DECODE_TAIL_SET(str, len);

    switch (*str) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '+':
    case '-':
        endptr = NULL;
        v->fv  = decode_str2dbl(s, &endptr);
        if (errno) {
            DECODE_TAIL_UNSET();
            return decode_error(L, op, errno, NULL);
        } else if ((size_t)(endptr - s) == len) {
            DECODE_TAIL_UNSET();
            return 0;
        }

    default:
        DECODE_TAIL_UNSET();
        return decode_error_at(L, op, EILSEQ, s, endptr);
    }
}

int decode_float_lua(lua_State *L)
{
    size_t len      = 0;
    const char *str = lauxh_checklstring(L, 1, &len);
    datum_t v       = {0};

    lua_settop(L, 1);
    if (decode_float(&v, L, "postgres.decode.float", str, len)) {
        return 2;
    }
    lua_pushnumber(L, v.fv);
    return 1;
}
