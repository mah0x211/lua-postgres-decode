local testcase = require('testcase')
local errno = require('errno')
local decode_date = require('postgres.decode.date')

function testcase.date_iso()
    -- test that decode ISO date value (yyyy-mm-dd)
    local v, err = decode_date('1999-12-31')
    assert.equal(v, {
        year = 1999,
        month = 12,
        day = 31,
    })
    assert.is_nil(err)

    -- test that ERANGE error
    for _, s in ipairs({
        '1999-13-31',
        '1999-12-32',
    }) do
        v, err = decode_date(s)
        assert.is_nil(v)
        assert.equal(err.type, errno.ERANGE)
    end

    -- test that empty string error
    v, err = decode_date('')
    assert.is_nil(v)
    assert.equal(err.type, errno.EINVAL)
    assert.match(err, 'empty string')

    -- test that EILSEQ error
    for _, s in ipairs({
        '+1999-12-31',
        '1999-12-31+',
    }) do
        v, err = decode_date(s)
        assert.is_nil(v)
        assert.equal(err.type, errno.EILSEQ)
        assert.match(err, '\'+\' at position')
    end

    -- test that throws an error if argument is not string
    err = assert.throws(decode_date)
    assert.match(err, 'string expected,')
end

function testcase.date_german()
    -- test that decode German date value (dd.mm.yyyy)
    local v, err = decode_date('31.12.1999')
    assert.equal(v, {
        year = 1999,
        month = 12,
        day = 31,
    })
    assert.is_nil(err)

    -- test that ERANGE error
    for _, s in ipairs({
        '32.12.1999',
        '31.13.1999',
    }) do
        v, err = decode_date(s)
        assert.is_nil(v)
        assert.equal(err.type, errno.ERANGE)
    end

    -- test that EILSEQ error
    v, err = decode_date('31.12.+1999')
    assert.is_nil(v)
    assert.equal(err.type, errno.EILSEQ)
    assert.match(err, '\'+\' at position')

    -- test that invalid format error
    v, err = decode_date('12:31:1999')
    assert.is_nil(v)
    assert.equal(err.type, errno.EINVAL)
    assert.match(err, 'invalid date format')
end

function testcase.date_sql()
    -- test that decode SQL date value (mm/dd/yyyy)
    local v, err = decode_date('12/31/1999')
    assert.equal(v, {
        year = 1999,
        month = 12,
        day = 31,
    })
    assert.is_nil(err)

    -- test that decode SQL date value order by DMY (dd/mm/yyyy)
    v, err = decode_date('31/12/1999', true)
    assert.equal(v, {
        year = 1999,
        month = 12,
        day = 31,
    })
    assert.is_nil(err)

    -- test that ERANGE error
    for _, s in ipairs({
        '13/31/1999',
        '12/32/1999',
    }) do
        v, err = decode_date(s)
        assert.is_nil(v)
        assert.equal(err.type, errno.ERANGE)
    end
end

function testcase.date_postgres()
    -- test that decode SQL date value (mm-dd-yyyy)
    local v, err = decode_date('12-31-1999')
    assert.equal(v, {
        year = 1999,
        month = 12,
        day = 31,
    })
    assert.is_nil(err)

    -- test that decode SQL date value order by DMY (dd-mm-yyyy)
    v, err = decode_date('31-12-1999', true)
    assert.equal(v, {
        year = 1999,
        month = 12,
        day = 31,
    })
    assert.is_nil(err)
end

