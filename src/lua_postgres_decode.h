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

#ifndef lua_postgres_decode_h
#define lua_postgres_decode_h

#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>
// lua
#include <lauxhlib.h>
#include <lua_errno.h>

static inline int decode_error(lua_State *L, const char *op, int err,
                               const char *fmt, ...)
{
    const char *msg = NULL;
    char buf[255]   = {0};

    if (fmt) {
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buf, 255, fmt, ap);
        va_end(ap);
        msg = (const char *)buf;
    }

    lua_settop(L, 0);
    lua_pushnil(L);
    lua_errno_new_ex(L, LUA_ERRNO_T_DEFAULT, err, op, msg, 0, 0);
    return 2;
}

static inline int decode_error_at(lua_State *L, const char *op, int err,
                                  const char *str, const char *endptr)
{
    return decode_error(L, op, err, "'%c' at position %d", *endptr,
                        endptr - str + 1);
}

static inline double decode_str2dbl(char *str, char **endptr)
{
    if (endptr) {
        *endptr = NULL;
    }
    errno = 0;
    return strtod(str, endptr);
}

static inline uintmax_t decode_str2umax(char *str, char **endptr)
{
    if (endptr) {
        *endptr = NULL;
    }
    errno = 0;
    return strtoumax(str, endptr, 10);
}

static inline intmax_t decode_str2imax(char *str, char **endptr)
{
    if (endptr) {
        *endptr = NULL;
    }
    errno = 0;
    return strtoimax(str, endptr, 10);
}

static inline intmax_t decode_digit(char *str, uint8_t mindigit,
                                    uint8_t maxdigit, uintmax_t minval,
                                    uintmax_t maxval, char **endptr)

{
    char *s         = str;
    uintmax_t limit = INTMAX_MAX;
    uintmax_t digit = 1;
    union {
        uintmax_t uv;
        intmax_t iv;
    } v = {0};

    if (mindigit == 0 || mindigit > maxdigit || maxdigit > 19) {
        errno = EDOM;
        return 0;
    }
    for (int i = 1; i < maxdigit; i++) {
        digit = (digit << 3) + (digit << 1);
    }

    errno = 0;
    while (digit >= 1) {
        uintmax_t prev = v.uv;

        if (!isdigit(*s)) {
            // not treated as an error if more than min digits are decoded
            if ((s - str) >= mindigit) {
                break;
            }
            errno = EILSEQ;
            goto FAILED;
        }

        // decode
        v.uv += (*s - '0') * digit;
        if (v.uv > limit || v.uv < prev) {
            errno = EOVERFLOW;
            v.iv  = INTMAX_MAX;
            goto FAILED;
        } else if (v.uv > maxval) {
            errno = ERANGE;
            goto FAILED;
        }
        digit /= 10;
        s++;
    }

    if (v.uv < minval) {
        errno = ERANGE;
        goto FAILED;
    }

FAILED:
    if (endptr) {
        *endptr = s;
    }

    return v.iv;
}

static inline char *decode_skip_space(char *s)
{
    while (*s == ' ') {
        s++;
    }
    return s;
}

static inline char *decode_skip_delim(char *s, char delim, char open_delim,
                                      int skip_space)
{
    int skip = 0;

    // skip leading whitespaces
    s = decode_skip_space(s);
    while (*s) {
        if (*s == delim) {
            if (!skip) {
                s++;
                // skip trailing whitespaces
                if (skip_space) {
                    return decode_skip_space(s);
                }
                return s;
            }
            skip--;
        } else if (*s == open_delim) {
            skip++;
        } else if (*s == '\\') {
            if (s[1]) {
                s++;
            }
        }
        s++;
    }

    return NULL;
}

#define DECODE_TAIL_SET(str, len)                                              \
    char *str_  = (char *)(str);                                               \
    size_t len_ = (len);                                                       \
    char tailc_ = str_[len_];                                                  \
    str_[len_]  = 0

#define DECODE_TAIL_UNSET() str_[len_] = tailc_

#define DECODE_START(L, op, str, len)                                          \
    do {                                                                       \
        if (!(len)) {                                                          \
            return decode_error((L), (op), EINVAL, "empty string");            \
        }                                                                      \
        lua_State *L_ = (L);                                                   \
        char *op_     = (char *)(op);                                          \
        char *head_   = (char *)(str);                                         \
    DECODE_TAIL_SET((str), (len))

#define DECODE_END(ptr)                                                        \
    if (*(ptr)) {                                                              \
        DECODE_TAIL_UNSET();                                                   \
        return decode_error_at(L_, op_, EILSEQ, head_, (ptr));                 \
    }                                                                          \
    DECODE_TAIL_UNSET();                                                       \
    }                                                                          \
    while (0)

#endif
