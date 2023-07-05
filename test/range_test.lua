local testcase = require('testcase')
local decode_range = require('postgres.decode.range')

function testcase.range()
    -- test that decode range value that both lower and upper are inclusive
    local rval, err = decode_range('[123, "456"] ',
                                   function(elmstr, is_quoted, ctx)
        assert.equal(ctx, 'context')
        if string.sub(elmstr, 1, 1) == '"' then
            assert.is_true(is_quoted)
        end
        return elmstr
    end, 'context')
    assert.is_nil(err)
    assert.equal(rval, {
        '123',
        '"456"',
        lower_inc = true,
        upper_inc = true,
    })

    -- test that decode range value that lower exclusive and upper inclusive
    rval, err = decode_range(' (123, 456] ', function(elmstr)
        return elmstr
    end)
    assert.is_nil(err)
    assert.equal(rval, {
        '123',
        '456',
        upper_inc = true,
    })

    -- test that decode range value that lower inclusive and upper exclusive
    rval, err = decode_range(' [123, 456)', function(elmstr)
        return elmstr
    end)
    assert.is_nil(err)
    assert.equal(rval, {
        '123',
        '456',
        lower_inc = true,
    })

    -- test that decode range value contains brackets
    for v, cmp in pairs({
        ['( {123}, (456)) '] = {
            '{123}',
            '(456)',
        },
        ['( [123], <456>) '] = {
            '[123]',
            '<456>',
        },
    }) do
        rval, err = decode_range(v, function(elmstr)
            return elmstr
        end)
        assert.is_nil(err)
        assert.equal(rval, cmp)
    end
end

function testcase.callback_can_change_values()
    -- test that callback function can change element value
    local rval, err = decode_range('[123, 456] ', function(elmstr)
        if elmstr == '456' then
            return 'abc'
        end
    end)
    assert.is_nil(err)
    assert.equal(rval, {
        nil,
        'abc',
        upper_inc = true,
    })

    rval, err = decode_range('[123, 456] ', function()
    end)
    assert.is_nil(err)
    assert.equal(rval, {
        nil,
        nil,
    })
end

function testcase.unbound()
    -- test that decode range value that lower is infinity
    local rval, err = decode_range("[, 456] ", function(elmstr)
        return elmstr
    end)
    assert.is_nil(err)
    assert.equal(rval, {
        nil,
        '456',
        upper_inc = true,
    })

    -- test that decode range value that upper is infinity
    rval, err = decode_range("[123, ] ", function(elmstr)
        return elmstr
    end)
    assert.is_nil(err)
    assert.equal(rval, {
        '123',
        nil,
        lower_inc = true,
    })
end

function testcase.empty()
    -- test that decode empty range value
    for _, v in ipairs({
        'empty',
        ' empty',
        'empty ',
        ' empty ',
    }) do
        local rval, err = decode_range(v, function(elmstr)
            return elmstr
        end)
        assert.is_nil(err)
        assert.equal(rval, {})
    end
end

function testcase.callback_error()
    -- test that callback error
    local rval, err = decode_range('[123, 456]', function(elmstr)
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
        local rval, err = decode_range(v, function(elmstr)
            return elmstr
        end)
        assert.is_nil(rval)
        assert.match(err, 'empty string')
    end

    -- test that illegal sequence error
    for _, v in ipairs({
        '[  ',
        '[  ]',
        '[123, 456, 789]',
        '{ 123, 456 }',
    }) do
        local rval, err = decode_range(v, function(elmstr)
            return elmstr
        end)
        assert.is_nil(rval)
        assert.match(err, 'EILSEQ')
    end
end
