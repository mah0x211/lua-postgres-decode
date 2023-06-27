local testcase = require('testcase')
local errno = require('errno')
local decode_bool = require('postgres.decode.bool')

function testcase.bool()
    -- test that decode bool 'true' value
    local v, err = decode_bool('t')
    assert.is_true(v)
    assert.is_nil(err)

    -- test that decode bool 'false' value
    v, err = decode_bool('f')
    assert.is_false(v)
    assert.is_nil(err)

    -- test that EILSEQ error
    v, err = decode_bool('a')
    assert.is_nil(v)
    assert.equal(err.type, errno.EILSEQ)
    assert.match(err, '\'a\' at position')

    -- test that EILSEQ error
    v, err = decode_bool('true')
    assert.is_nil(v)
    assert.equal(err.type, errno.EILSEQ)
    assert.match(err, '\'r\' at position')

    -- test that empty string error
    v, err = decode_bool('')
    assert.is_nil(v)
    assert.equal(err.type, errno.EINVAL)
    assert.match(err, 'empty string')

    -- test that throws an error if argument is not string
    err = assert.throws(decode_bool)
    assert.match(err, 'string expected,')
end

