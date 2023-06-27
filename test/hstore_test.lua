local testcase = require('testcase')
local errno = require('errno')
local decode_hstore = require('postgres.decode.hstore')

function testcase.hstore()
    -- test that decode hstore value
    local v, err = assert(decode_hstore(
                              '"a"=>"1","b"=>"2",  "c"=>NULL, "d"=>"NULL"'))
    assert.equal(v, {
        a = '1',
        b = '2',
        c = nil,
        d = 'NULL',
    })
    assert.is_nil(err)

    -- test that opening double-quote error
    v, err = decode_hstore('foo=>bar')
    assert.is_nil(v)
    assert.equal(err.type, errno.EILSEQ)
    assert.match(err, 'opening double-quote')

    -- test that closing double-quote error
    v, err = decode_hstore('"foo=>bar')
    assert.is_nil(v)
    assert.equal(err.type, errno.EILSEQ)
    assert.match(err, 'closing double-quote')

    -- test that key-value separator error
    v, err = decode_hstore('"foo"=>"string", "bar"="1"')
    assert.is_nil(v)
    assert.equal(err.type, errno.EILSEQ)
    assert.match(err, 'key-value separator')

    -- test that invalid null value error
    v, err = decode_hstore('"foo"=>NUL, "bar"=>"1"')
    assert.is_nil(v)
    assert.equal(err.type, errno.EILSEQ)
    assert.match(err, 'invalid null')

    -- test that empty string error
    v, err = decode_hstore('')
    assert.is_nil(v)
    assert.equal(err.type, errno.EINVAL)
    assert.match(err, 'empty string')

    -- test that throws an error if argument is not string
    err = assert.throws(decode_hstore)
    assert.match(err, 'string expected,')
end
