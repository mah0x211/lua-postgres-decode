local testcase = require('testcase')
local decode = require('postgres.decode')

function testcase.line()
    -- test that decode line
    local v = assert(decode.line('{10.5,  5.5, 1.3}'))
    assert.equal(v, {
        10.5,
        5.5,
        1.3,
    })
end

