local testcase = require('testcase')
local decode = require('postgres.decode')

function testcase.point()
    -- test that decode point
    local v = assert(decode.point('(10.5,  5.5)  '))
    assert.equal(v, {
        10.5,
        5.5,
    })
end

