# lua-postgres-decode

[![test](https://github.com/mah0x211/lua-postgres-decode/actions/workflows/test.yml/badge.svg)](https://github.com/mah0x211/lua-postgres-decode/actions/workflows/test.yml)
[![codecov](https://codecov.io/gh/mah0x211/lua-postgres-decode/branch/master/graph/badge.svg)](https://codecov.io/gh/mah0x211/lua-postgres-decode)

A set of decoding functions for PostgreSQL output data types.


## Installation

```
luarocks install postgres-decode
```

***

## v, err = decode.int( intstr )

decode integer string to intmax_t or uintmax_t value and returns it as lua_Integer value.

see also: https://www.postgresql.org/docs/current/datatype-numeric.html#DATATYPE-INT

**Parameters**

- `intstr:string`: integer string representation.

**Returns**

- `v:integer`: lua_Integer value.
- `err:any`: error object.

**Example**

```lua
local decode_int = require('postgres.decode.int')
local v = assert(decode_int('9223372036854775807'))
print(v)
```


## v, err = decode.float( floatstr )

decode float string to double value and returns it as lua_Number value.

see also: https://www.postgresql.org/docs/current/datatype-numeric.html#DATATYPE-FLOAT

**Parameters**

- `floatstr:string`: float string representation.

**Returns**

- `v:number`: lua_Number value.
- `err:any`: error object.


## v, err = decode.date( datestr [, is_dmy] )

decode date string to a table containing year, month, day.

see also: https://www.postgresql.org/docs/current/datatype-datetime.html#DATATYPE-DATETIME-OUTPUT

**Parameters**

- `datestr:string`: date string representation.
- `is_dmy:boolean`: if `true`, the date string is formatted as DMY order.

**Returns**

- `v:table`: table containing `year`, `month`, `day`.
- `err:any`: error object.


## v, err = decode.time( timestr )

decode time string with time zone to a table containing hour, minute, second, microsecond, timezone.

see also: https://www.postgresql.org/docs/current/datatype-datetime.html#DATATYPE-DATETIME-OUTPUT

**Parameters**

- `timestr:string`: time string representation.

**Returns**

- `v:table`: table containing `hour`, `min`, `sec`, `usec`, `tz`, `tzhour`, `tzmin`, `tzsec`.
- `err:any`: error object.


## v, err = decode.timestamp( timestampstr )

decode timestamp string with time zone to a table containing year, month, day, hour, minute, second, microsecond, timezone.

see also: https://www.postgresql.org/docs/current/datatype-datetime.html#DATATYPE-DATETIME-OUTPUT

**Parameters**

- `timestampstr:string`: timestamp string representation.

**Returns**

- `v:table`: table containing `year`, `month`, `day`, `hour`, `min`, `sec`, `usec`, `tz`, `tzhour`, `tzmin`, `tzsec`.
- `err:any`: error object.


## v, err = decode.bool( boolstr )

decode boolean string to boolean value.

see also: https://www.postgresql.org/docs/current/datatype-boolean.html

**Parameters**

- `boolstr:string`: boolean string representation.

**Returns**

- `v:boolean`: boolean value.
- `err:any`: error object.


## v, err = decode.point( pointstr )

decode point string to array of 2 double values that represents x, y coordinates.

https://www.postgresql.org/docs/current/datatype-geometric.html#id-1.5.7.16.5

**Parameters**

- `pointstr:string`: point string representation.

**Returns**

- `v:number[]`: array of 2 double values.
    - `1`: `x`
    - `2`: `y`
- `err:any`: error object.


## v, err = decode.line( linestr )

decode line string to array of 3 double values that represents A, B, C coefficients.

https://www.postgresql.org/docs/current/datatype-geometric.html#DATATYPE-LINE

**Parameters**

- `linestr:string`: line string representation.

**Returns**

- `v:number[]`: array of 3 double values.
    - `1`: `A`
    - `2`: `B`
    - `3`: `C`
- `err:any`: error object.


## v, err = decode.lseg( lsegstr )

decode line segment string to array of 2 double values that represents x1, y1, x2, y2 coordinates.

https://www.postgresql.org/docs/current/datatype-geometric.html#DATATYPE-LSEG

**Parameters**

- `lsegstr:string`: line segment string representation.

**Returns**

- `v:number[][]`: array of 2 double values contained array.
    - `1`
        - `1`: `x1`
        - `2`: `y1`
    - `2`
        - `1`: `x2`
        - `2`: `y2`
- `err:any`: error object.


## v, err = decode.box( boxstr )

decode box string to array of 2 double values that represents x1, y1, x2, y2 coordinates.

https://www.postgresql.org/docs/current/datatype-geometric.html#id-1.5.7.16.8

**Parameters**

- `boxstr:string`: box string representation.

**Returns**

- `v:number[][]`: array of 2 double values contained array.
    - `1`
        - `1`: `x1`
        - `2`: `y1`
    - `2`
        - `1`: `x2`
        - `2`: `y2`
- `err:any`: error object.


## v, err = decode.path( pathstr )

decode path string to array of double values that represents x, y coordinates.

https://www.postgresql.org/docs/current/datatype-geometric.html#id-1.5.7.16.9

**Parameters**

- `pathstr:string`: path string representation.

**Returns**

- `v:number[][]`: array of 2 double values contained array.
    - `1`
        - `1`: `x1`
        - `2`: `y1`
    - `...`
    - `N`
        - `1`: `xN`
        - `2`: `yN`
- `err:any`: error object.


## v, err = decode.polygon( polygonstr )

decode polygon string to array of double values that represents x, y coordinates.

https://www.postgresql.org/docs/current/datatype-geometric.html#DATATYPE-POLYGON

**Parameters**

- `polygonstr:string`: polygon string representation.

**Returns**

- `v:number[][]`: array of 2 double values contained array.
    - `1`
        - `1`: `x1`
        - `2`: `y1`
    - `...`
    - `N`
        - `1`: `xN`
        - `2`: `yN`
- `err:any`: error object.


## v, err = decode.circle( circlestr )

decode circle string to array of 3 double values that represents x, y coordinates and radius.

https://www.postgresql.org/docs/current/datatype-geometric.html#DATATYPE-CIRCLE

**Parameters**

- `circlestr:string`: circle string representation.

**Returns**

- `v:number[]`: array of 3 double values.
    - `1`: `x`
    - `2`: `y`
    - `3`: `radius`
- `err:any`: error object.


## v, err = decode.bit( bitstr )

decode bit string to array of 8-bit unsigned integers.

see also: https://www.postgresql.org/docs/current/datatype-bit.html

**Parameters**

- `bitstr: string`: bit string representation.

**Returns**

- `v:integer[]`: array of 8-bit unsigned integers.
- `err:any`: error object.


## v, err = decode.array( str, fn )

decode array string to array of values.

see also: https://www.postgresql.org/docs/current/arrays.html

**Parameters**

- `str:string`: array string representation.
- `fn:function`: function to decode array element.
    ```lua
    --- decodefn decode array element string to value. 
    --- @param elmstr string
    --- @return v any
    --- @return err any
    function decodefn( elmstr )
        -- if decodefn returns nil, err, stop decoding and return nil and err.
        return v, 'error from decodefn'
    end
    ```

**Returns**

- `v:any[]`: array of values.
- `err:any`: error object.


