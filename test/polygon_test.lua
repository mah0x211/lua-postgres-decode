local testcase = require('testcase')
local decode_polygon = require('postgres.decode.polygon')

function testcase.polygon()
    -- test that decode polygon
    local v = assert(decode_polygon('((10.5,5.5), ( 20.5, 15.5  ),(1, 4) )  '))
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

