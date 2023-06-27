local testcase = require('testcase')
local errno = require('errno')
local decode_int = require('postgres.decode.int')

function testcase.int()
    -- test that decode int value
    for _, s in ipairs({
        '+9223372036854775807',
        '9223372036854775807',
        '-9223372036854775808',
    }) do
        local v, err = decode_int(s)
        assert.is_int(v)
        assert.is_nil(err)
    end

    -- test that ERANGE error
    for _, s in ipairs({
        '+9223372036854775808',
        '9223372036854775808',
        '-9223372036854775809',
    }) do
        local v, err = decode_int(s)
        assert.is_nil(v)
        assert.equal(err.type, errno.ERANGE)
    end

    -- test that EILSEQ error
    for _, s in ipairs({
        '#1234',
        '1234#',
    }) do
        local v, err = decode_int(s)
        assert.is_nil(v)
        assert.equal(err.type, errno.EILSEQ)
        assert.match(err, '\'#\' at position')
    end

    -- test that empty string error
    local v, err = decode_int('')
    assert.is_nil(v)
    assert.equal(err.type, errno.EINVAL)
    assert.match(err, 'empty string')

    -- test that throws an error if argument is not string
    err = assert.throws(decode_int)
    assert.match(err, 'string expected,')
end
