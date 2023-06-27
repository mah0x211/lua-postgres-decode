local testcase = require('testcase')
local decode_circle = require('postgres.decode.circle')

function testcase.circle()
    -- test that decode circle
    local v = assert(decode_circle('<(10.5,5.5), 15.5 >  '))
    assert.equal(v, {
        10.5,
        5.5,
        15.5,
    })
end

