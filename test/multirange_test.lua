local testcase = require('testcase')
local decode_multirange = require('postgres.decode.multirange')

function testcase.multirange()
    -- test that decode multirange value
    local rval, err = decode_multirange(
                          '{[123, 456],  (123, 456] ,  [123, 456), (123, 456) }',
                          function(elmstr)
            return elmstr
        end)
    assert.is_nil(err)
    assert.equal(rval, {
        {
            '123',
            '456',
            lower_inc = true,
            upper_inc = true,
        },
        {
            '123',
            '456',
            upper_inc = true,
        },
        {
            '123',
            '456',
            lower_inc = true,
        },
        {
            '123',
            '456',
        },
    })

    -- test that decode multirange value contains brackets
    for v, cmp in pairs({
        ['{( {123}, (456)) }'] = {
            {
                '{123}',
                '(456)',
            },
        },
        ['{( [123], <456>) }'] = {
            {
                '[123]',
                '<456>',
            },
        },
    }) do
        rval, err = decode_multirange(v, function(elmstr)
            return elmstr
        end)
        assert.is_nil(err)
        assert.equal(rval, cmp)
    end
end

function testcase.empty()
    -- test that decode empty range value
    for _, v in ipairs({
        '{empty}',
        '{ empty}',
        '{empty }',
        '{ empty }',
    }) do
        local rval, err = decode_multirange(v, function(elmstr)
            return elmstr
        end)
        assert.is_nil(err)
        assert.equal(rval, {
            {},
        })
    end
end

function testcase.callback_error()
    -- test that callback error
    local rval, err = decode_multirange('{[123, 456]}', function(elmstr)
        return elmstr, 'callback error'
    end)
    assert.is_nil(rval)
    assert.match(err, 'callback error')
end

function testcase.invalid_format_error()
    -- test that empty string error
    for _, v in ipairs({
        '',
        '     ',
    }) do
        local rval, err = decode_multirange(v, function(elmstr)
            return elmstr
        end)
        assert.is_nil(rval)
        assert.match(err, 'empty string')
    end

    -- test that illegal sequence error
    for _, v in ipairs({
        '[ ]',
        '{[  }',
        '{ empty  ',
        '{[123,, 456], }',
        '{ (123, 456) ]}',
    }) do
        local rval, err = decode_multirange(v, function(elmstr)
            return elmstr
        end)
        assert.is_nil(rval)
        assert.match(err, 'EILSEQ')
    end
end
