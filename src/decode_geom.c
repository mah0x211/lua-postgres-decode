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
  (s) = decode_skip_delim((s), (delim), 0, 1);                                 \
  if (!(s)) {                                                                  \
   DECODE_TAIL_UNSET();                                                        \
   return decode_error((L), (op), EILSEQ, __VA_ARGS__);                        \
  }                                                                            \
 } while (0)

#define STR2DBL(s, v)                                                          \
 do {                                                                          \
  char *endptr_ = NULL;                                                        \
  (v)           = decode_str2dbl((s), &endptr_);                               \
  if (errno) {                                                                 \
   DECODE_TAIL_UNSET();                                                        \
   return decode_error((L), (op), errno, NULL);                                \
  }                                                                            \
  (s) = endptr_;                                                               \
 } while (0)

#define STR2POINT(s, x, y)                                                     \
 do {                                                                          \
  /* parse (x,y) */                                                            \
  SKIP_DELIM((s), '(', "opening round bracket not found");                     \
  STR2DBL((s), (x));                                                           \
  SKIP_DELIM(s, ',', "separator not found");                                   \
  STR2DBL((s), (y));                                                           \
  SKIP_DELIM((s), ')', "closing round bracket not found");                     \
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

// 8.8. Geometric Types
// https://www.postgresql.org/docs/current/datatype-geometric.html

typedef int (*geomfn)(datum_t *v, lua_State *L, const char *op, const char *str,
                      size_t len);

static int decode_geom(datum_t *v, lua_State *L, const char *op, geomfn fn)
{
    size_t len      = 0;
    const char *str = lauxh_checklstring(L, 1, &len);
    lua_settop(L, 1);
    return fn(v, L, op, str, len);
}

// 8.8.1. Points
// https://www.postgresql.org/docs/current/datatype-geometric.html#id-1.5.7.16.5

int decode_point(datum_t *v, lua_State *L, const char *op, const char *str,
                 size_t len)
{
    char *s = (char *)str;
    DO_START(str, len);

    // parse (x,y)
    STR2POINT(s, v->point[0], v->point[1]);

    DO_END(s);
    return 0;
}

int decode_point_lua(lua_State *L)
{
    datum_t v = {0};

    // point: (x, y)
    if (decode_geom(&v, L, "postgres.decode.point", decode_point)) {
        return 2;
    }
    lua_createtable(L, 2, 0);
    lauxh_pushnum2arr(L, 1, v.point[0]);
    lauxh_pushnum2arr(L, 2, v.point[1]);
    return 1;
}

// 8.8.2. Lines
// https://www.postgresql.org/docs/current/datatype-geometric.html#DATATYPE-LINE

int decode_line(datum_t *v, lua_State *L, const char *op, const char *str,
                size_t len)
{
    char *s = (char *)str;
    DO_START(str, len);

    // parse {a,b,c}
    SKIP_DELIM(s, '{', "opening curly bracket not found");
    STR2DBL(s, v->line[0]);
    SKIP_DELIM(s, ',', "separator not found");
    STR2DBL(s, v->line[1]);
    SKIP_DELIM(s, ',', "separator not found");
    STR2DBL(s, v->line[2]);
    SKIP_DELIM(s, '}', "closing curly bracket not found");

    DO_END(s);
    return 0;
}

int decode_line_lua(lua_State *L)
{
    datum_t v = {0};

    // line: {a, b, c}
    if (decode_geom(&v, L, "postgres.decode.line", decode_line)) {
        return 2;
    }
    lua_createtable(L, 3, 0);
    lauxh_pushnum2arr(L, 1, v.line[0]);
    lauxh_pushnum2arr(L, 2, v.line[1]);
    lauxh_pushnum2arr(L, 3, v.line[2]);
    return 1;
}

// 8.8.3. Line Segments
// https://www.postgresql.org/docs/current/datatype-geometric.html#DATATYPE-LSEG

int decode_lseg(datum_t *v, lua_State *L, const char *op, const char *str,
                size_t len)
{
    char *s = (char *)str;
    DO_START(str, len);

    // parse [(x1,y1),(x2,y2)]
    SKIP_DELIM(s, '[', "opening square bracket not found");
    STR2POINT(s, v->lseg[0][0], v->lseg[0][1]);
    SKIP_DELIM(s, ',', "separator not found");
    STR2POINT(s, v->lseg[1][0], v->lseg[1][1]);
    SKIP_DELIM(s, ']', "closing square bracket not found");

    DO_END(s);
    return 0;
}

int decode_lseg_lua(lua_State *L)
{
    datum_t v = {0};

    // line-segment: [(x1, y1), (x2, y2)]
    if (decode_geom(&v, L, "postgres.decode.lseg", decode_lseg)) {
        return 2;
    }
    lua_createtable(L, 2, 0);
    for (int i = 0; i < 2; i++) {
        lua_createtable(L, 2, 0);
        lauxh_pushnum2arr(L, 1, v.lseg[i][0]);
        lauxh_pushnum2arr(L, 2, v.lseg[i][1]);
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

// 8.8.4. Boxes
// https://www.postgresql.org/docs/current/datatype-geometric.html#id-1.5.7.16.8

int decode_box(datum_t *v, lua_State *L, const char *op, const char *str,
               size_t len)
{
    char *s = (char *)str;
    DO_START(str, len);

    // parse (x1,y1),(x2,y2)
    STR2POINT(s, v->box[0], v->box[1]);
    SKIP_DELIM(s, ',', "separator not found");
    STR2POINT(s, v->box[2], v->box[3]);

    DO_END(s);
    return 0;
}

int decode_box_lua(lua_State *L)
{
    datum_t v = {0};

    // box: (x1, y1), (x2, y2)
    if (decode_geom(&v, L, "postgres.decode.box", decode_box)) {
        return 2;
    }
    lua_createtable(L, 2, 0);
    for (int i = 0; i < 2; i++) {
        lua_createtable(L, 2, 0);
        lauxh_pushnum2arr(L, 1, v.box[i * 2]);
        lauxh_pushnum2arr(L, 2, v.box[i * 2 + 1]);
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

// 8.8.7. Circles
// https://www.postgresql.org/docs/current/datatype-geometric.html#DATATYPE-CIRCLE

int decode_circle(datum_t *v, lua_State *L, const char *op, const char *str,
                  size_t len)
{
    char *s = (char *)str;
    DO_START(str, len);

    // parse <(x,y),r>
    SKIP_DELIM(s, '<', "opening angle bracket not found");
    STR2POINT(s, v->circle[0], v->circle[1]);
    SKIP_DELIM(s, ',', "separator not found");
    STR2DBL(s, v->circle[2]);
    SKIP_DELIM(s, '>', "closing angle bracket not found");

    DO_END(s);
    return 0;
}

int decode_circle_lua(lua_State *L)
{
    datum_t v = {0};

    // circle: <(x,y),r>
    if (decode_geom(&v, L, "postgres.decode.circle", decode_circle)) {
        return 2;
    }
    lua_createtable(L, 2, 0);
    lauxh_pushnum2arr(L, 1, v.circle[0]);
    lauxh_pushnum2arr(L, 2, v.circle[1]);
    lauxh_pushnum2arr(L, 3, v.circle[2]);
    return 1;
}

// 8.8.5. Paths
// https://www.postgresql.org/docs/current/datatype-geometric.html#id-1.5.7.16.9

int decode_path(datum_t *v, lua_State *L, const char *op, const char *str,
                size_t len)
{
    char *s          = (char *)str;
    char delim       = *s;
    char delim_close = ')';
    double x         = 0;
    double y         = 0;
    int idx          = 0;

    (void)v;
    DO_START(str, len);

    // path: [(x1, y1), ... (xn, yn)] or ((x1, y1), ... (xn, yn))
    switch (delim) {
    case '[':
        delim_close = ']';
    case '(':
        s++;
        break;
    default:
        SKIP_DELIM(s, '[', "opening square or round bracket not found");
    }

    // decode: (x1, y1), ... (xn, yn)
    lua_newtable(L);
CHECK_NEXT:
    STR2POINT(s, x, y);
    lua_createtable(L, 2, 0);
    lauxh_pushnum2arr(L, 1, x);
    lauxh_pushnum2arr(L, 2, y);
    lua_rawseti(L, -2, ++idx);
    if (*s == ',') {
        SKIP_DELIM(s, ',', "separator not found");
        goto CHECK_NEXT;
    }
    SKIP_DELIM(s, delim_close, "closing square or round bracket not found");

    DO_END(s);
    return 1;
}

int decode_path_lua(lua_State *L)
{
    // path: [(x1, y1), ... (xn, yn)] or ((x1, y1), ... (xn, yn))
    return decode_geom(NULL, L, "postgres.decode.path", decode_path);
}

// 8.8.6. Polygons
// https://www.postgresql.org/docs/current/datatype-geometric.html#DATATYPE-POLYGON

int decode_polygon(datum_t *v, lua_State *L, const char *op, const char *str,
                   size_t len)
{
    char *s  = (char *)str;
    double x = 0;
    double y = 0;
    int idx  = 0;

    (void)v;
    DO_START(str, len);

    // polygon: ((x1, y1), ... (xn, yn))
    SKIP_DELIM(s, '(', "opening round bracket not found");

    lua_newtable(L);
CHECK_NEXT:
    STR2POINT(s, x, y);
    lua_createtable(L, 2, 0);
    lauxh_pushnum2arr(L, 1, x);
    lauxh_pushnum2arr(L, 2, y);
    lua_rawseti(L, -2, ++idx);
    if (*s == ',') {
        SKIP_DELIM(s, ',', "separator not found");
        goto CHECK_NEXT;
    }
    SKIP_DELIM(s, ')', "closing round bracket not found");

    DO_END(s);
    return 1;
}

int decode_polygon_lua(lua_State *L)
{
    // polygon: ((x1, y1), ... (xn, yn))
    return decode_geom(NULL, L, "postgres.decode.polygon", decode_polygon);
}

#undef SKIP_DELIM
#undef STR2DBL
#undef STR2POINT
#undef DO_START
#undef DO_END
