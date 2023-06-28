local testcase = require('testcase')
local decode_time = require('postgres.decode.time')

function testcase.time()
    -- test that decode time with time zone
    for _, v in ipairs({
        {
            str = '23:59:59.123456',
            cmp = {
                hour = 23,
                min = 59,
                sec = 59,
                usec = 123456,
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
    }) do
        local t = assert(decode_time(v.str))
        assert.equal(t, v.cmp)
    end

    -- test that empty string error
    local v, err = decode_time('')
    assert.is_nil(v)
    assert.match(err, 'empty string')
end
