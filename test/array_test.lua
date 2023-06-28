local testcase = require('testcase')
local decode_array = require('postgres.decode.array')

function testcase.array()
    -- test that array value
    local v, err = decode_array(
                       '{ foo, { (bar), { [baz] }, baa }, <qux>, "quux", hello\\ world! }',
                       function(elmstr)
            return elmstr
        end)
    assert.equal(v, {
        'foo',
        {
            '(bar)',
            {
                '[baz]',
            },
            'baa',
        },
        '<qux>',
        '"quux"',
        'hello\\ world!',
    })
    assert.is_nil(err)

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

    -- test that empty string error
    v, err = decode_array('', function(elmstr)
        return elmstr
    end)
    assert.is_nil(v)
    assert.match(err, 'empty string')

    -- test that opening bracket error
    v, err = decode_array('foo, bar, baz }', function(elmstr)
        return elmstr
    end)
    assert.is_nil(v)
    assert.match(err, 'opening curly bracket')

    -- test that malformed array error
    v, err = decode_array('{ ', function(elmstr)
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

    -- test that nesting level too deep error
    v, err = decode_array(string.rep('{', 65), function(elmstr)
        return elmstr
    end)
    assert.is_nil(v)
    assert.match(err, 'nesting level .+ too deep', false)

    -- test that illegal character error
    v, err = decode_array('{} }', function(elmstr)
        return elmstr
    end)
    assert.is_nil(v)
    assert.match(err, "'}' found at")
end
