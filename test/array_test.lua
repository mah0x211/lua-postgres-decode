local testcase = require('testcase')
local decode = require('postgres.decode')

function testcase.array()
    -- test that array value
    local v, err = decode.array(
                       '{ foo, { (bar), { [baz] }, baa }, <qux>, "quux", hello\\ world! }',
                       function(v)
            return v
        end)
    print(v, err)
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
