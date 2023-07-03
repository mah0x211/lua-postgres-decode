local testcase = require('testcase')
local decode_tsvector = require('postgres.decode.tsvector')

function testcase.tsvector()
    -- test that decode tsvector value
    local tsv = decode_tsvector(
                    "'a' 'and' 'ate' 'cat' 'fat' 'mat' 'on' 'rat' 'sat'")
    assert.equal(tsv, {
        {
            lexeme = 'a',
        },
        {
            lexeme = 'and',
        },
        {
            lexeme = 'ate',
        },
        {
            lexeme = 'cat',
        },
        {
            lexeme = 'fat',
        },
        {
            lexeme = 'mat',
        },
        {
            lexeme = 'on',
        },
        {
            lexeme = 'rat',
        },
        {
            lexeme = 'sat',
        },
    })
end

function testcase.space_lexeme()
    -- test that decode tsvector value with space lexeme
    local tsv = decode_tsvector("'    ' 'contains' 'lexeme' 'spaces' 'the'")
    assert.equal(tsv, {
        {
            lexeme = '    ',
        },
        {
            lexeme = 'contains',
        },
        {
            lexeme = 'lexeme',
        },
        {
            lexeme = 'spaces',
        },
        {
            lexeme = 'the',
        },
    })
end

function testcase.escaped_with_quote()
    -- test that decode tsvector value with escaped lexeme
    local tsv =
        decode_tsvector("'Joe''s' 'a' 'contains' 'lexeme' 'quote' 'the'")
    assert.equal(tsv, {
        {
            lexeme = "Joe''s",
        },
        {
            lexeme = 'a',
        },
        {
            lexeme = 'contains',
        },
        {
            lexeme = 'lexeme',
        },
        {
            lexeme = 'quote',
        },
        {
            lexeme = 'the',
        },
    })
end

function testcase.with_positions()
    -- test that decode tsvector value with positions
    local tsv, err = decode_tsvector(
                         "'a':1,6,10 'and':8 'ate':9 'cat':3 'fat':2,11 'mat':7 'on':5 'rat':12 'sat':4")
    assert.is_nil(err)
    assert.equal(tsv, {
        {
            lexeme = 'a',
            positions = {
                1,
                6,
                10,
            },
        },
        {
            lexeme = 'and',
            positions = {
                8,
            },
        },
        {
            lexeme = 'ate',
            positions = {
                9,
            },
        },
        {
            lexeme = 'cat',
            positions = {
                3,
            },
        },
        {
            lexeme = 'fat',
            positions = {
                2,
                11,
            },
        },
        {
            lexeme = 'mat',
            positions = {
                7,
            },
        },
        {
            lexeme = 'on',
            positions = {
                5,
            },
        },
        {
            lexeme = 'rat',
            positions = {
                12,
            },
        },
        {
            lexeme = 'sat',
            positions = {
                4,
            },
        },
    })

    -- test that invalid position format error
    tsv, err = decode_tsvector("'foo': 'bar'")
    assert.is_nil(tsv)
    assert.match(err, 'position 7')

    -- test that invalid weight format error
    tsv, err = decode_tsvector("'foo':12+ 'bar'")
    assert.is_nil(tsv)
    assert.match(err, 'position 9')
end

function testcase.with_weights()
    -- test that positions has the weights
    local tsv = decode_tsvector("'a':1A 'cat':5 'fat':2,4C")
    assert.equal(tsv, {
        {
            lexeme = 'a',
            positions = {
                1,
            },
            weights = {
                'A',
            },
        },
        {
            lexeme = 'cat',
            positions = {
                5,
            },
        },
        {
            lexeme = 'fat',
            positions = {
                2,
                4,
            },
            weights = {
                nil,
                'C',
            },
        },
    })
end

