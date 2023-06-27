local testcase = require('testcase')
local decode_lseg = require('postgres.decode.lseg')

function testcase.lseg()
    -- test that decode line-segment
    local v = assert(decode_lseg('[ (10.5,  5.5), ( 20.5, 15.5  )]'))
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
