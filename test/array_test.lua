local testcase = require('testcase')
local decode_array = require('postgres.decode.array')

function testcase.array()
    -- test that array value
    local v, err = decode_array(
                       [[{ foo, NULL, { (bar), { [baz] }, baa }, <qux>, "quux", "hello\ world!" }]],
                       function(elmstr, is_quoted, ctx)
            assert.equal(ctx, 'context')
            if string.sub(elmstr, 1, 1) == '"' then
                assert.is_true(is_quoted)
            end
            return elmstr
        end, 'context')
    assert.is_nil(err)
    assert.equal(v, {
        'foo',
        nil,
        {
            '(bar)',
            {
                '[baz]',
            },
            'baa',
        },
        '<qux>',
        '"quux"',
        '"hello\\ world!"',
    })

    -- test that NULL elements
    v, err = decode_array('{ foo, bar, NULL }', function(elmstr)
        return elmstr
    end)
    assert.is_nil(err)
    assert.equal(v, {
        'foo',
        'bar',
    })

    -- test that can ignore elements
    v, err = decode_array('{ foo, (bar), baz }', function(elmstr)
        if elmstr == 'foo' then
            return
        end
        return elmstr
    end)
    assert.equal(v, {
        nil,
        '(bar)',
        'baz',
    })
    assert.is_nil(err)

    -- test that returns result of callback
    v, err = decode_array('{ foo, (bar), [baz] }', function()
        return 'hello', 'world!'
    end)
    assert.is_nil(v)
    assert.match(err, 'world!')
end

function testcase.custom_delimiter()
    -- test that use custom delimiter
    local v, err = decode_array('{foo; bar; baz}', function(elmstr)
        return elmstr
    end, nil, ';')
    assert.is_nil(err)
    assert.equal(v, {
        'foo',
        'bar',
        'baz',
    })

    -- test that use default delimiter if delimiter argument is nil
    v, err = decode_array('{foo, bar, baz}', function(elmstr)
        return elmstr
    end, nil, nil)
    assert.is_nil(err)
    assert.equal(v, {
        'foo',
        'bar',
        'baz',
    })
end

function testcase.empty_string_error()
    -- test that empty string error
    local v, err = decode_array('', function(elmstr)
        return elmstr
    end)
    assert.is_nil(v)
    assert.match(err, 'empty string')
end

function testcase.nesting_level_error()
    -- test that nesting level too deep error
    local v, err = decode_array(string.rep('{', 65), function(elmstr)
        return elmstr
    end)
    assert.is_nil(v)
    assert.match(err, 'nesting level .+ too deep', false)
end

function testcase.no_opening_bracket_error()
    -- test that opening bracket error
    local v, err = decode_array('foo, bar, baz }', function(elmstr)
        return elmstr
    end)
    assert.is_nil(v)
    assert.match(err, 'opening curly bracket')
end

function testcase.empty_element_error()
    -- test that empty elements are not allowed error
    local v, err = decode_array('{ ,}', function(elmstr)
        return elmstr
    end)
    assert.is_nil(v)
    assert.match(err, 'empty element')
end

function testcase.closing_quotation_error()
    -- test that quoted string error
    local v, err = decode_array('{ "foo', function(elmstr)
        return elmstr
    end)
    assert.is_nil(v)
    assert.match(err, 'closing quotation not found')
end

function testcase.malformed_array_error()
    -- test that malformed array error
    local v, err = decode_array('{ ', function(elmstr)
        return elmstr
    end)
    assert.is_nil(v)
    assert.match(err, 'malformed array string')

    -- test that unquoted string error
    v, err = decode_array('{ foo', function(elmstr)
        return elmstr
    end)
    assert.is_nil(v)
    assert.match(err, 'malformed array string')

    -- test that malformed array error
    v, err = decode_array('{ foo, ', function(elmstr)
        return elmstr
    end)
    assert.is_nil(v)
    assert.match(err, 'malformed array string')
end

function testcase.illegal_character_error()
    -- test that illegal character after closing bracket error
    local v, err = decode_array('{ } }', function(elmstr)
        return elmstr
    end)
    assert.is_nil(v)
    assert.match(err, "'}' at position 5")

    -- test that illegal character after element error
    v, err = decode_array('{ foo | }', function(elmstr)
        return elmstr
    end)
    assert.is_nil(v)
    assert.match(err, "'|' at position 7")

    -- test that illegal character error
    v, err = decode_array('{} }', function(elmstr)
        return elmstr
    end)
    assert.is_nil(v)
    assert.match(err, "'}' at position 4")
end
