local testcase = require('testcase')
local decode = require('postgres.decode')

function testcase.path()
    -- test that decode path enclosed in round bracket
    local v = assert(decode.path('((10.5,  5.5), ( 20.5, 15.5  ) )  '))
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

    -- test that decode path enclosed in square bracket
    v = assert(decode.path('[(10.5,  5.5), ( 20.5, 15.5  )  ]'))
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
