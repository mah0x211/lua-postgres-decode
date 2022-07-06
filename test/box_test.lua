local testcase = require('testcase')
local decode = require('postgres.decode')

function testcase.box()
    -- test that decode box
    local v = assert(decode.box('(10.5,  5.5), ( 20.5, 15.5  )'))
    assert.equal(v, {
        {
            10.5,
            5.5,
        },
        {
            20.5,
            15.5,
        },
    })
end

