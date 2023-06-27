local testcase = require('testcase')
local decode_point = require('postgres.decode.point')

function testcase.point()
    -- test that decode point
    local v = assert(decode_point('(10.5,  5.5)  '))
    assert.equal(v, {
        10.5,
        5.5,
    })
end

