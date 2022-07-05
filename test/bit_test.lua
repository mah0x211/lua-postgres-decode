local testcase = require('testcase')
local errno = require('errno')
local decode = require('postgres.decode')

function testcase.bit()
    -- test that decode bit value
    local v, err = assert(decode.bit('0101011100'))
    assert.equal(v, '0101011100')
    assert.is_nil(err)

    -- test that EILSEQ error
    v, err = decode.bit('010101110!0')
    assert.is_nil(v)
    assert.equal(err.type, errno.EILSEQ)
    assert.match(err, '\'!\' at position')

    -- test that empty string error
    v, err = decode.bit('')
    assert.is_nil(v)
    assert.equal(err.type, errno.EINVAL)
    assert.match(err, 'empty string')

    -- test that throws an error if argument is not string
    err = assert.throws(decode.bit)
    assert.match(err, 'string expected,')
end
