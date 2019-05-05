#pragma once

#include <stdint.h>

enum class Punctuation : uint8_t
{
    Unknown,

    // Multiple characters punctuation
    // !!!!! Have to be sort by the number of characters
    // !!!!! to avoid bad forward detection (/** issue when last * can override the /* detection during the forward test)

// TODO the escape of line return can be handle by the parser by checking if there is no more token after the \ on the line
//    InhibitedLineReturn,    //    \\n or \\r\n          A backslash that preceed a line return (should be completely skipped)
    LineComment,            //    //
    OpenBlockComment,       //    /*
    CloseBlockComment,      //    */
    Arrow,                  //    ->
    And,                    //    &&
    Or,                     //    ||
    DoubleColon,            //    ::                 Used for namespaces
    EqualityTest,           //    ==
    DifferenceTest,         //    !=
    // Not sure that must be detected as one token instead of multiples (especially <<, >>, <<= and >>=) because of templates
//    LeftShift,              //    <<
//    RightShift,             //    >>
//    AdditionAssignment,     //    +=
//    SubstractionAssignment, //    -=
//    MultiplicationAssignment, //    *=
//    DivisionAssignment,     //    /=
//    DivisionAssignment,     //    %=
//    DivisionAssignment,     //    |=
//    DivisionAssignment,     //    &=
//    DivisionAssignment,     //    ^=
//    DivisionAssignment,     //    <<=
//    DivisionAssignment,     //    >>=

    // Mostly in the order on a QWERTY keyboard (symbols making a pair are grouped)
    Tilde,                  //    ~                  Should stay the first of single character symbols
    Backquote,              //    `
    Bang,                   //    !
    At,                     //    @
    Hash,                   //    #
    Dollar,                 //    $
    Percent,                //    %
    Caret,                  //    ^
    Ampersand,              //    &
    Star,                   //    *
    OpenParenthesis,        //    (
    CloseParenthesis,       //    )
    Underscore,             //    _
    Dash,                   //    -
    Plus,                   //    +
    Equals,                 //    =
    OpenBrace,              //    {
    CloseBrace,             //    }
    OpenBracket,            //    [
    CloseBracket,           //    ]
    Colon,                  //    :                  Used in ternaire expression
    Semicolon,              //    ;
    SingleQuote,            //    '
    DoubleQuote,            //    "
    Pipe,                   //    |
    Slash,                  //    /
    Backslash,              //    '\'
    Less,                   //    <
    Greater,                //    >
    Comma,                  //    ,
    Dot,                    //    .
    QuestionMark,           //    ?

    // White character at end to be able to handle correctly lines that terminate with a separator like semicolon just before a line return
    WhiteCharacter,
    NewLineCharacter
};

enum class Keyword
{
    _unknown,

	_include,
	_define,
	_undef,
	_pragma,
	_if,
	_else,
	_endif,
	_ifdef,
	_ifndef,
	_defined,
	_error
};

inline bool isWhitePunctuation(Punctuation punctuation)
{
    return punctuation >= Punctuation::WhiteCharacter;
}
