local testcase = require('testcase')
local decode_array = require('postgres.decode.array')

function testcase.array()
    -- test that array value
    local v, err = decode_array(
                       '{ foo, { (bar), { [baz] }, baa }, <qux>, "quux", hello\\ world! }',
                       function(v)
            return v
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
end
