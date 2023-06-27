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

#ifndef lua_postgres_decode_geom_h
#define lua_postgres_decode_geom_h

#include "lua_postgres_decode.h"

// 8.8. Geometric Types
// https://www.postgresql.org/docs/current/datatype-geometric.html

#define GEOM_SKIP_DELIM(s, delim, ...)                                         \
    do {                                                                       \
        (s) = decode_skip_delim((s), (delim), 0, 1);                           \
        if (!(s)) {                                                            \
            return decode_error((L), (op), EILSEQ, __VA_ARGS__);               \
        }                                                                      \
    } while (0)

#define GEOM_STR2DBL(s, v)                                                     \
    do {                                                                       \
        char *endptr_ = NULL;                                                  \
        (v)           = decode_str2dbl((s), &endptr_);                         \
        if (errno) {                                                           \
            return decode_error((L), (op), errno, NULL);                       \
        }                                                                      \
        (s) = endptr_;                                                         \
    } while (0)

#define GEOM_STR2POINT(s, x, y)                                                \
    do {                                                                       \
        /* parse (x,y) */                                                      \
        GEOM_SKIP_DELIM((s), '(', "opening round bracket not found");          \
        GEOM_STR2DBL((s), (x));                                                \
        GEOM_SKIP_DELIM(s, ',', "separator not found");                        \
        GEOM_STR2DBL((s), (y));                                                \
        GEOM_SKIP_DELIM((s), ')', "closing round bracket not found");          \
    } while (0)

#endif
