local testcase = require('testcase')
local errno = require('errno')
local decode = require('postgres.decode')

function testcase.float()
    -- test that decode float value
    local v, err = decode.float('01234.56789')
    assert.equal(v, 1234.56789)
    assert.is_nil(err)

    -- test that ERANGE error
    v, err = decode.float('1e+309')
    assert.is_nil(v)
    assert.equal(err.type, errno.ERANGE)

    -- test that EILSEQ error
    v, err = decode.float('01234#.56789')
    assert.is_nil(v)
    assert.equal(err.type, errno.EILSEQ)

    -- test that empty string error
    v, err = decode.float('')
    assert.is_nil(v)
    assert.equal(err.type, errno.EINVAL)
    assert.match(err, 'empty string')

    -- test that throws an error if argument is not string
    err = assert.throws(decode.float)
    assert.match(err, 'string expected,')
end
