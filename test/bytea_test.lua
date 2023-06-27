local testcase = require('testcase')
local errno = require('errno')
local decode_bytea = require('postgres.decode.bytea')

function testcase.bytea_hex()
    -- test that decode hex bytea value
    local v, err = decode_bytea('\\x012345679abcdef')

    assert.equal(v, '\\x012345679abcdef')
    assert.is_nil(err)

    -- test that decode empty hex bytea value
    v, err = decode_bytea('\\x')
    assert.equal(v, '\\x')
    assert.is_nil(err)

    -- test that EILSEQ error
    v, err = decode_bytea('a')
    assert.is_nil(v)
    assert.equal(err.type, errno.EILSEQ)
    assert.match(err, '\'a\' at position')

    v, err = decode_bytea('\\a')
    assert.is_nil(v)
    assert.equal(err.type, errno.EILSEQ)
    assert.match(err, '\'a\' at position')

    -- test that EILSEQ error
    v, err = decode_bytea('\\x0123h456')
    assert.is_nil(v)
    assert.equal(err.type, errno.EILSEQ)
    assert.match(err, '\'h\' at position')

    -- test that empty string error
    v, err = decode_bytea('')
    assert.is_nil(v)
    assert.equal(err.type, errno.EINVAL)
    assert.match(err, 'empty string')

    -- test that throws an error if argument is not string
    err = assert.throws(decode_bytea)
    assert.match(err, 'string expected,')
end

function testcase.bytea_escape()
    -- test that returns a passed string argument if escape argument is true
    for _, a in ipairs({
        '\\x012345679abcdef',
        'a',
        '\\a',
        '\\x0123h456',
        '',
    }) do
        local v, err = decode_bytea(a, true)
        assert.equal(v, a)
        assert.is_nil(err)
    end

    -- test that throws an error if argument is not string
    local err = assert.throws(decode_bytea)
    assert.match(err, 'string expected,')
end

