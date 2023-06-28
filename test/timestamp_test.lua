local testcase = require('testcase')
local decode_timestamp = require('postgres.decode.timestamp')

function testcase.timestamp()
    -- test that decode timestamp
    for _, v in ipairs({
        {
            str = '1999-12-31 23:59:59.123456',
            cmp = {
                year = 1999,
                month = 12,
                day = 31,
                hour = 23,
                min = 59,
                sec = 59,
                usec = 123456,
            },
        },
        {
            str = '1999-12-31 23:59:59.123456+15',
            cmp = {
                year = 1999,
                month = 12,
                day = 31,
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
            str = '1999-12-31 23:59:59.123456+15:59',
            cmp = {
                year = 1999,
                month = 12,
                day = 31,
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
            str = '1999-12-31 23:59:59.123456+15:59:59',
            cmp = {
                year = 1999,
                month = 12,
                day = 31,
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
        local t = assert(decode_timestamp(v.str))
        assert.equal(t, v.cmp)
    end

    -- test that empty string error
    local v, err = decode_timestamp('')
    assert.is_nil(v)
    assert.match(err, 'empty string')

    -- test that timezone symbol error
    v, err = decode_timestamp('1999-12-31 23:59:59.123456 15:59:59')
    assert.is_nil(v)
    assert.match(err, 'timezone symbol')
end

