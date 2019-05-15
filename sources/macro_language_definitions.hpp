#pragma once

#include <stdint.h>

namespace macro
{
	enum class Punctuation : uint8_t
	{
		unknown,

		// Multiple characters punctuation
		// !!!!! Have to be sort by the number of characters
		// !!!!! to avoid bad forward detection (/** issue when last * can override the /* detection during the forward test)

	// TODO the escape of line return can be handle by the parser by checking if there is no more token after the \ on the line
	//    InhibitedLineReturn,    //    \\n or \\r\n          A backslash that preceed a line return (should be completely skipped)
		line_comment,           //    //
		open_block_comment,     //    /*
		close_block_comment,    //    */
		arrow,                  //    ->
		logical_and,            //    &&
		logical_or,             //    ||
		double_colon,           //    ::                 Used for namespaces
		equality_test,          //    ==
		difference_test,        //    !=
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
		tilde,                  //    ~                  Should stay the first of single character symbols
		backquote,              //    `
		bang,                   //    !
		at,                     //    @
		hash,                   //    #
		dollar,                 //    $
		percent,                //    %
		caret,                  //    ^
		ampersand,              //    &					 bitwise and
		star,                   //    *
		open_parenthesis,       //    (
		close_parenthesis,      //    )
		underscore,             //    _
		dash,                   //    -
		plus,                   //    +
		equals,                 //    =
		open_brace,             //    {
		close_brace,            //    }
		open_bracket,           //    [
		close_bracket,          //    ]
		colon,                  //    :                  Used in ternaire expression
		semicolon,              //    ;
		single_quote,           //    '
		double_quote,           //    "
		pipe,                   //    |					 bitwise or
		slash,                  //    /
		backslash,              //    '\'
		less,                   //    <
		greater,                //    >
		comma,                  //    ,
		dot,                    //    .
		question_mark,          //    ?

		// White character at end to be able to handle correctly lines that terminate with a separator like semicolon just before a line return
		white_character,
		new_line_character
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
		_elif,
		_endif,
		_ifdef,
		_ifndef,
		_defined,
		_error
	};

	inline bool is_white_punctuation(Punctuation punctuation)
	{
		return punctuation >= Punctuation::white_character;
	}
}
