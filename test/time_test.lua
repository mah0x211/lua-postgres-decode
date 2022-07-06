local testcase = require('testcase')
local decode = require('postgres.decode')

function testcase.time()
    -- test that decode time_without_time_zone
    local v = assert(decode.time('23:59:59.123456'))
    assert.equal(v, {
        hour = 23,
        min = 59,
        sec = 59,
        usec = 123456,
    })
end

-- local function select_data_types()
--     local c = assert(new_connection())

--     -- test that select data types
--     assert(c:query([[
-- SELECT
--     t.oid AS "oid",
--     replace(pg_catalog.format_type(t.oid, NULL), ' ', '_') AS "name",
--     t.typtype AS "type",
--     t.typcategory AS "category",
--     CASE t.typcategory
--         WHEN 'A' THEN 'array'
--         WHEN 'B' THEN 'boolean'
--         WHEN 'C' THEN 'composite'
--         WHEN 'D' THEN 'date_time'
--         WHEN 'E' THEN 'enum'
--         WHEN 'G' THEN 'geometric'
--         WHEN 'I' THEN 'network_address'
--         WHEN 'N' THEN 'numeric'
--         WHEN 'P' THEN 'pseudo'
--         WHEN 'R' THEN 'range'
--         WHEN 'S' THEN 'string'
--         WHEN 'T' THEN 'timespan'
--         WHEN 'U' THEN 'user_defined'
--         WHEN 'V' THEN 'bit_string'
--         WHEN 'X' THEN 'unknown'
--         ELSE 'unknown'
--     END AS "category_name",
--     typinput AS "input",
--     typoutput AS "output",
--     pg_catalog.obj_description(t.oid, 'pg_type') AS "description"
-- FROM
--     pg_catalog.pg_type t
-- WHERE
--     -- without pseudo-type
--     t.typtype != 'p'
-- AND
--     -- non-composite type or kind of the composite type class
--     (t.typrelid = 0 OR (
--         SELECT
--             c.relkind = 'c'
--         FROM
--             pg_catalog.pg_class c
--         WHERE
--             c.oid = t.typrelid
--     ))
-- AND
--     -- non-reference type
--     NOT EXISTS(
--         SELECT
--             1
--         FROM
--             pg_catalog.pg_type el
--         WHERE
--             el.oid = t.typelem
--         AND
--             el.typarray = t.oid
--     )
-- AND
--     -- type visible in the search path
--     pg_catalog.pg_type_is_visible(t.oid)
-- ORDER BY t.oid, t.typcategory
--     ]]))
--     local res = assert(c:get_result())
--     local stat = assert(res:stat())
--     local cols = {}
--     for _, field in ipairs(stat.fields) do
--         cols[#cols + 1] = field.name
--     end

--     local rows = assert(res:rows())
--     for _, row in ipairs(rows) do
--         for i, name in ipairs(cols) do
--             print(string.format('%15s: %s', name, row[i]))
--         end
--         print('-------------------------')
--     end
--     -- print(dump(rows))
-- end
