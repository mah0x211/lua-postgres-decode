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

typedef struct {
    int year; // years
    int mon;  // months since January [1-12]
    int day;  // day of the month [1-31]
    int hour; // hours since midnight [0-24]
    int min;  // minutes [0-59]
    int sec;  // seconds [0-59]
    int usec; // microseconds [0-999999]
    // timezone
    int tzhour;     // timezone hours [0-24]
    int tzmin;      // timezone minutes [0-59]
    int tzsec;      // timezone seconds [0-59]
    char tzsign[2]; // timezone sign [+-]
} datum_timestamp_t;

typedef union {
    int bv;
    double fv;
    intmax_t iv;
    uintmax_t uv;
    datum_timestamp_t tv;
    double point[2];
    double line[3];
    double lseg[2][2];
    double box[4];
    double circle[3];
} datum_t;

int decode_bit_lua(lua_State *L);
int decode_bit(lua_State *L, const char *op, const char *str, size_t len);

int decode_bytea_lua(lua_State *L);
int decode_bytea(lua_State *L, const char *op, const char *str, size_t len,
                 int is_escape);

int decode_bool_lua(lua_State *L);
int decode_bool(datum_t *v, lua_State *L, const char *op, const char *str,
                size_t len);

int decode_float_lua(lua_State *L);
int decode_float(datum_t *v, lua_State *L, const char *op, const char *str,
                 size_t len);

int decode_int_lua(lua_State *L);
int decode_int(datum_t *v, lua_State *L, const char *op, const char *str,
               size_t len);

int decode_date_lua(lua_State *L);
int decode_date(datum_t *v, lua_State *L, const char *op, const char *str,
                size_t len, int is_dmy);

int decode_time_lua(lua_State *L);
int decode_timetz_lua(lua_State *L);
int decode_time(datum_t *v, lua_State *L, const char *op, const char *str,
                size_t len, int with_tz);

int decode_point_lua(lua_State *L);
int decode_point(datum_t *v, lua_State *L, const char *op, const char *str,
                 size_t len);

int decode_line_lua(lua_State *L);
int decode_line(datum_t *v, lua_State *L, const char *op, const char *str,
                size_t len);

int decode_lseg_lua(lua_State *L);
int decode_lseg(datum_t *v, lua_State *L, const char *op, const char *str,
                size_t len);

int decode_box_lua(lua_State *L);
int decode_box(datum_t *v, lua_State *L, const char *op, const char *str,
               size_t len);

int decode_path_lua(lua_State *L);
int decode_path(datum_t *v, lua_State *L, const char *op, const char *str,
                size_t len);

int decode_polygon_lua(lua_State *L);
int decode_polygon(datum_t *v, lua_State *L, const char *op, const char *str,
                   size_t len);

int decode_circle_lua(lua_State *L);
int decode_circle(datum_t *v, lua_State *L, const char *op, const char *str,
                  size_t len);

int decode_array_lua(lua_State *L);
typedef int (*decode_array_cb)(void *ctx, lua_State *L, const char *op,
                               const char *str, size_t len);
int decode_array(lua_State *L, const char *op, const char *str,
                 decode_array_cb cbfn, void *ctx);

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
 char *str_  = (char *)(str);                                                  \
 size_t len_ = (len);                                                          \
 char tailc_ = str_[len_];                                                     \
 str_[len_]  = 0

#define DECODE_TAIL_UNSET() str_[len_] = tailc_

#endif
