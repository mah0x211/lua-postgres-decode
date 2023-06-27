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

#ifndef lua_postgres_decode_datetime_h
#define lua_postgres_decode_datetime_h

#include "lua_postgres_decode.h"

#define DATETIME_SKIP_DELIM(s, delim, ...)                                     \
    do {                                                                       \
        if (*(s) != (delim)) {                                                 \
            return decode_error((L), (op), EILSEQ, __VA_ARGS__);               \
        }                                                                      \
        (s)++;                                                                 \
    } while (0)

#define DATETIME_STR2DIGIT(s, v, mind, maxd, minv, maxv)                       \
    do {                                                                       \
        char *endptr_ = NULL;                                                  \
        (v) = decode_digit((s), (mind), (maxd), (minv), (maxv), &endptr_);     \
        if (errno) {                                                           \
            return decode_error_at((L), (op), errno, head_, endptr_);          \
        }                                                                      \
        (s) = endptr_;                                                         \
    } while (0)

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

static inline int decode_time(datum_timestamp_t *ts, lua_State *L,
                              const char *op, const char *str, size_t len,
                              int with_tz)
{
    char *s           = (char *)str;
    intmax_t min_max  = 59;
    intmax_t sec_max  = 59;
    intmax_t usec_max = 999999;

    DECODE_START(L, op, s, len);

    // decode: hh:mm:ss
    DATETIME_STR2DIGIT(s, ts->hour, 2, 2, 0, 24);
    DATETIME_SKIP_DELIM(s, ':', "delimiter not found");
    if (ts->hour == 24) {
        min_max  = 0;
        sec_max  = 0;
        usec_max = 0;
    }
    DATETIME_STR2DIGIT(s, ts->min, 2, 2, 0, min_max);
    DATETIME_SKIP_DELIM(s, ':', "delimiter not found");
    DATETIME_STR2DIGIT(s, ts->sec, 2, 2, 0, sec_max);
    // decode: .uuuuuu (microseconds)
    if (*s == '.') {
        s++;
        DATETIME_STR2DIGIT(s, ts->usec, 1, 6, 0, usec_max);
    }

    // parse timezone: [+-]hh:mm:ss
    if (with_tz) {
        // parse sign [+-]
        switch (*s) {
        case '+':
        case '-':
            ts->tzsign[0] = *s;
            s++;
            break;

        default:
            DATETIME_SKIP_DELIM(s, '+', "separator not found");
        }

        // parse: hh | hh:mm | hh:mm:ss
        DATETIME_STR2DIGIT(s, ts->tzhour, 2, 2, 0, 24);
        if (*s == ':') {
            // parse: hh:mm
            s++;
            DATETIME_STR2DIGIT(s, ts->tzmin, 2, 2, 0, 59);
            if (*s == ':') {
                // parse: hh:mm:ss
                s++;
                DATETIME_STR2DIGIT(s, ts->tzsec, 2, 2, 0, 59);
            }
        }
    }

    DECODE_END(s);
    return 0;
}

static inline int decode_date(datum_timestamp_t *ts, lua_State *L,
                              const char *op, const char *str, size_t len,
                              int is_dmy)
{
    char *s    = (char *)str;
    char delim = s[2];

    DECODE_START(L, op, s, len);

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
        DATETIME_STR2DIGIT(s, ts->year, 4, 4, 0, -1);
        DATETIME_SKIP_DELIM(s, '-', "separator not found");
        DATETIME_STR2DIGIT(s, ts->mon, 2, 2, 1, 12);
        DATETIME_SKIP_DELIM(s, '-', "separator not found");
        DATETIME_STR2DIGIT(s, ts->day, 2, 2, 1, 31);
        break;

    case '.':
        is_dmy = 1;
    case '/':
    case '-':
        if (is_dmy) {
            // German   : dd.mm.yyyy
            // SQL      : dd/mm/yyyy
            // Postgres : dd-mm-yyyy
            DATETIME_STR2DIGIT(s, ts->day, 2, 2, 1, 31);
            DATETIME_SKIP_DELIM(s, delim, "separator not found");
            DATETIME_STR2DIGIT(s, ts->mon, 2, 2, 1, 12);
        } else {
            // SQL      : mm/dd/yyyy
            // Postgres : mm-dd-yyyy
            DATETIME_STR2DIGIT(s, ts->mon, 2, 2, 1, 12);
            DATETIME_SKIP_DELIM(s, delim, "separator not found");
            DATETIME_STR2DIGIT(s, ts->day, 2, 2, 1, 31);
        }
        // decode yyyy
        DATETIME_SKIP_DELIM(s, delim, "separator not found");
        DATETIME_STR2DIGIT(s, ts->year, 4, 4, 0, -1);
        break;

    default:
        return decode_error(L, op, EINVAL, "invalid date format");
    }

    DECODE_END(s);
    return 0;
}

#undef DATETIME_SKIP_DELIM
#undef DATETIME_STR2DIGIT

#endif
