local testcase = require('testcase')
local decode_path = require('postgres.decode.path')

function testcase.path()
    -- test that decode path enclosed in round bracket
    local v, err = decode_path('((10.5,  5.5), ( 20.5, 15.5  ) )  ')
    assert.is_nil(err)
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
    v, err = decode_path('[(10.5,  5.5), ( 20.5, 15.5  )  ]')
    assert.is_nil(err)
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

    -- test that opening bracket error
    v, err = decode_path('{(10.5,  5.5), ( 20.5, 15.5  )  }')
    assert.is_nil(v)
    assert.match(err, 'opening square or round bracket')
end
