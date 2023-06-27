local testcase = require('testcase')
local decode_timetz = require('postgres.decode.timetz')

function testcase.timetz()
    -- test that decode timetz
    for _, v in ipairs({
        {
            str = '23:59:59.123456+15:59:59',
            cmp = {
                hour = 23,
                min = 59,
                sec = 59,
                usec = 123456,
                tz = '+',
                tzhour = 15,
                tzmin = 59,
                tzsec = 59,
            },
        },
        {
            str = '23:59:59.123456+15:59',
            cmp = {
                hour = 23,
                min = 59,
                sec = 59,
                usec = 123456,
                tz = '+',
                tzhour = 15,
                tzmin = 59,
                tzsec = 0,
            },
        },
        {
            str = '23:59:59.123456+15',
            cmp = {
                hour = 23,
                min = 59,
                sec = 59,
                usec = 123456,
                tz = '+',
                tzhour = 15,
                tzmin = 0,
                tzsec = 0,
            },
        },
    }) do
        local t = assert(decode_timetz(v.str))
        assert.equal(t, v.cmp)
    end
end

