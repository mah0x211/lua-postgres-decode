local testcase = require('testcase')
local decode = require('postgres.decode')

function testcase.lseg()
    -- test that decode line-segment
    local v = assert(decode.lseg('[ (10.5,  5.5), ( 20.5, 15.5  )]'))
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
