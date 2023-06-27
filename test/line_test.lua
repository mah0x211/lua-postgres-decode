local testcase = require('testcase')
local decode_line = require('postgres.decode.line')

function testcase.line()
    -- test that decode line
    local v = assert(decode_line('{10.5,  5.5, 1.3}'))
    assert.equal(v, {
        10.5,
        5.5,
        1.3,
    })
end

