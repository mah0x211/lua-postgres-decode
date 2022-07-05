local testcase = require('testcase')
local decode = require('postgres.decode')

function testcase.circle()
    -- test that decode polygon
    local v = assert(decode.circle('<(10.5,5.5), 15.5 >  '))
    assert.equal(v, {
        10.5,
        5.5,
        15.5,
    })
end

