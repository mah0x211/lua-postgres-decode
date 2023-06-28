local testcase = require('testcase')
local errno = require('errno')
local decode_bit = require('postgres.decode.bit')

function testcase.bit()
    -- test that decode bit value: 01010111001 -> 0101 0111 001 append [0 0000]
    local v, err = assert(decode_bit('01010111001'))
    assert.equal(v, {
        87,
        32,
    })
    assert.is_nil(err)

    -- test that decode bit value: 0101011100100001 -> 0101 0111 0010 0001
    v, err = assert(decode_bit('0101011100100001'))
    assert.equal(v, {
        87,
        33,
    })
    assert.is_nil(err)

    -- test that EILSEQ error
    v, err = decode_bit('010101110!0')
    assert.is_nil(v)
    assert.equal(err.type, errno.EILSEQ)
    assert.match(err, '\'!\' at position')

    -- test that empty string error
    v, err = decode_bit('')
    assert.is_nil(v)
    assert.equal(err.type, errno.EINVAL)
    assert.match(err, 'empty string')

    -- test that throws an error if argument is not string
    err = assert.throws(decode_bit)
    assert.match(err, 'string expected,')
end
