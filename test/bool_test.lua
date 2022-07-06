local testcase = require('testcase')
local errno = require('errno')
local decode = require('postgres.decode')

function testcase.bool()
    -- test that decode bool 'true' value
    local v, err = decode.bool('t')
    assert.is_true(v)
    assert.is_nil(err)

    -- test that decode bool 'false' value
    v, err = decode.bool('f')
    assert.is_false(v)
    assert.is_nil(err)

    -- test that EILSEQ error
    v, err = decode.bool('a')
    assert.is_nil(v)
    assert.equal(err.type, errno.EILSEQ)
    assert.match(err, '\'a\' at position')

    -- test that EILSEQ error
    v, err = decode.bool('true')
    assert.is_nil(v)
    assert.equal(err.type, errno.EILSEQ)
    assert.match(err, '\'r\' at position')

    -- test that empty string error
    v, err = decode.bool('')
    assert.is_nil(v)
    assert.equal(err.type, errno.EINVAL)
    assert.match(err, 'empty string')

    -- test that throws an error if argument is not string
    err = assert.throws(decode.bool)
    assert.match(err, 'string expected,')
end

