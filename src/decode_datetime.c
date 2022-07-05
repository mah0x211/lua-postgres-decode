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

#define SKIP_DELIM(s, delim, ...)                                              \
 do {                                                                          \
  if (*(s) != (delim)) {                                                       \
   DECODE_TAIL_UNSET();                                                        \
   return decode_error((L), (op), EILSEQ, __VA_ARGS__);                        \
  }                                                                            \
  s++;                                                                         \
 } while (0)

#define STR2DIGIT(s, v, mind, maxd, minv, maxv)                                \
 do {                                                                          \
  char *endptr_ = NULL;                                                        \
  (v)           = decode_digit((s), (mind), (maxd), (minv), (maxv), &endptr_); \
  if (errno) {                                                                 \
   DECODE_TAIL_UNSET();                                                        \
   return decode_error_at((L), (op), errno, head_, endptr_);                   \
  }                                                                            \
  (s) = endptr_;                                                               \
 } while (0)

#define DO_START(str, len)                                                     \
 char *head_ = (char *)(str);                                                  \
 if (!(len)) {                                                                 \
  return decode_error((L), (op), EINVAL, "empty string");                      \
 }                                                                             \
 DECODE_TAIL_SET((str), (len))

#define DO_END(ptr)                                                            \
 if (*(ptr)) {                                                                 \
  DECODE_TAIL_UNSET();                                                         \
  return decode_error_at((L), (op), EILSEQ, head_, (ptr));                     \
 }                                                                             \
 DECODE_TAIL_UNSET()

// 8.5.2. Date/Time Output
// https://www.postgresql.org/docs/current/datatype-datetime.html#DATATYPE-DATETIME-OUTPUT
//
//
// Table 8.14. Date/Time Output Styles
//
// Style Spec   Description             Example
// ISO          ISO 8601, SQL standard  1997-12-17 07:37:16-08
// SQL          traditional style       12/17/1997 07:37:16.00 PST
// Postgres     original style          Wed Dec 17 07:37:16 1997 PST
// German       regional style          17.12.1997 07:37:16.00 PST
//
//
// Table 8.15. Date Order Conventions
//
// datestyle        Input Ordering  Example Output
// SQL, DMY         day/month/year  17/12/1997 15:37:16.00 CET
// SQL, MDY         month/day/year  12/17/1997 07:37:16.00 PST
// Postgres, DMY    day/month/year  Wed 17 Dec 07:37:16 1997 PST
//

int decode_time(datum_t *v, lua_State *L, const char *op, const char *str,
                size_t len, int with_tz)
{
    char *s           = (char *)str;
    intmax_t min_max  = 59;
    intmax_t sec_max  = 59;
    intmax_t usec_max = 999999;

    DO_START(str, len);

    // decode: hh:mm:ss
    STR2DIGIT(s, v->tv.hour, 2, 2, 0, 24);
    SKIP_DELIM(s, ':', "delimiter not found");
    if (v->tv.hour == 24) {
        min_max  = 0;
        sec_max  = 0;
        usec_max = 0;
    }
    STR2DIGIT(s, v->tv.min, 2, 2, 0, min_max);
    SKIP_DELIM(s, ':', "delimiter not found");
    STR2DIGIT(s, v->tv.sec, 2, 2, 0, sec_max);
    // decode: .uuuuuu (microseconds)
    if (*s == '.') {
        s++;
        STR2DIGIT(s, v->tv.usec, 1, 6, 0, usec_max);
    }

    // parse timezone: [+-]hh:mm:ss
    if (with_tz) {
        // parse sign [+-]
        switch (*s) {
        case '+':
        case '-':
            v->tv.tzsign[0] = *s;
            s++;
            break;

        default:
            SKIP_DELIM(s, '+', "separator not found");
        }

        // parse: hh | hh:mm | hh:mm:ss
        STR2DIGIT(s, v->tv.tzhour, 2, 2, 0, 24);
        if (*s == ':') {
            // parse: hh:mm
            s++;
            STR2DIGIT(s, v->tv.tzmin, 2, 2, 0, 59);
            if (*s == ':') {
                // parse: hh:mm:ss
                s++;
                STR2DIGIT(s, v->tv.tzsec, 2, 2, 0, 59);
            }
        }
    }

    DO_END(s);
    return 0;
}

int decode_time_lua(lua_State *L)
{
    size_t len      = 0;
    const char *str = lauxh_checklstring(L, 1, &len);
    datum_t v       = {0};

    lua_settop(L, 1);
    if (decode_time(&v, L, "postgres.decode.time", str, len, 0)) {
        return 2;
    }

    lua_createtable(L, 0, 4);
    lauxh_pushint2tbl(L, "hour", v.tv.hour);
    lauxh_pushint2tbl(L, "min", v.tv.min);
    lauxh_pushint2tbl(L, "sec", v.tv.sec);
    lauxh_pushint2tbl(L, "usec", v.tv.usec);
    return 1;
}

int decode_timetz_lua(lua_State *L)
{
    size_t len      = 0;
    const char *str = lauxh_checklstring(L, 1, &len);
    datum_t v       = {0};

    lua_settop(L, 1);
    if (decode_time(&v, L, "postgres.decode.timetz", str, len, 1)) {
        return 2;
    }

    lua_createtable(L, 0, 4);
    lauxh_pushint2tbl(L, "hour", v.tv.hour);
    lauxh_pushint2tbl(L, "min", v.tv.min);
    lauxh_pushint2tbl(L, "sec", v.tv.sec);
    lauxh_pushint2tbl(L, "usec", v.tv.usec);
    lauxh_pushstr2tbl(L, "tz", v.tv.tzsign);
    lauxh_pushint2tbl(L, "tzhour", v.tv.tzhour);
    lauxh_pushint2tbl(L, "tzmin", v.tv.tzmin);
    lauxh_pushint2tbl(L, "tzsec", v.tv.tzsec);
    return 1;
}

int decode_date(datum_t *v, lua_State *L, const char *op, const char *str,
                size_t len, int is_dmy)
{
    char *s    = (char *)str;
    char delim = s[2];

    DO_START(str, len);

    // date styles
    switch (delim) {
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
        // ISO: yyyy-mm-dd
        // XSD: yyyy-mm-dd
        // decode year: yyyy
        STR2DIGIT(s, v->tv.year, 4, 4, 0, -1);
        SKIP_DELIM(s, '-', "separator not found");
        STR2DIGIT(s, v->tv.mon, 2, 2, 1, 12);
        SKIP_DELIM(s, '-', "separator not found");
        STR2DIGIT(s, v->tv.day, 2, 2, 1, 31);
        break;

    case '.':
        is_dmy = 1;
    case '/':
    case '-':
        if (is_dmy) {
            // German   : dd.mm.yyyy
            // SQL      : dd/mm/yyyy
            // Postgres : dd-mm-yyyy
            STR2DIGIT(s, v->tv.day, 2, 2, 1, 31);
            SKIP_DELIM(s, delim, "separator not found");
            STR2DIGIT(s, v->tv.mon, 2, 2, 1, 12);
        } else {
            // SQL      : mm/dd/yyyy
            // Postgres : mm-dd-yyyy
            STR2DIGIT(s, v->tv.mon, 2, 2, 1, 12);
            SKIP_DELIM(s, delim, "separator not found");
            STR2DIGIT(s, v->tv.day, 2, 2, 1, 31);
        }
        // decode yyyy
        SKIP_DELIM(s, delim, "separator not found");
        STR2DIGIT(s, v->tv.year, 4, 4, 0, -1);
        break;

    default:
        DECODE_TAIL_UNSET();
        return decode_error(L, op, EINVAL, "invalid date format");
    }

    DO_END(s);
    return 0;
}

int decode_date_lua(lua_State *L)
{
    size_t len      = 0;
    const char *str = lauxh_checklstring(L, 1, &len);
    int is_dmy      = lauxh_optboolean(L, 2, 0);
    datum_t v       = {0};

    lua_settop(L, 1);
    if (decode_date(&v, L, "postgres.decode.date", str, len, is_dmy)) {
        return 2;
    }
    lua_createtable(L, 0, 3);
    lauxh_pushint2tbl(L, "year", v.tv.year);
    lauxh_pushint2tbl(L, "month", v.tv.mon);
    lauxh_pushint2tbl(L, "day", v.tv.day);
    return 1;
}
