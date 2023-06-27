local testcase = require('testcase')
local decode_box = require('postgres.decode.box')

function testcase.box()
    -- test that decode box
    local v = assert(decode_box('(10.5,  5.5), ( 20.5, 15.5  )'))
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

