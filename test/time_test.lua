local testcase = require('testcase')
local decode_time = require('postgres.decode.time')

function testcase.time()
    -- test that decode time_without_time_zone
    local v = assert(decode_time('23:59:59.123456'))
    assert.equal(v, {
        hour = 23,
        min = 59,
        sec = 59,
        usec = 123456,
    })
end

