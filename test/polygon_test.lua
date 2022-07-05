local testcase = require('testcase')
local decode = require('postgres.decode')

function testcase.polygon()
    -- test that decode polygon
    local v = assert(decode.polygon('((10.5,5.5), ( 20.5, 15.5  ),(1, 4) )  '))
    assert.equal(v, {
        {
            10.5,
            5.5,
        },
        {
            20.5,
            15.5,
        },
        {
            1.0,
            4.0,
        },
    })
end

