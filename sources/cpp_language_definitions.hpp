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
    Unknown,

    Typedef,
    Extern,
    Class,
    Struct,
    Enum,
    Union,
    Static,
    Virtual,
    Override,
    Public,
    Protected,
    Private,
    Friend,
    Template,
    Typename,
    Return,
    Operator,
    If,
    Else,
    Switch,
    Case,
    Default,
    For,
    While,
    Do,
    Goto,
    Label,
    Inline,
    New,
    Delete,
    Try,
    Catch,
    Throw,
    Noexcept,
    StaticAssert,
    Namespace,
    Explicit,

    // Values
    True,
    False,
    Nullptr,
    // --

    // Compiler specific keywords
    GNU_ATTRIBUTE,
    GNU_STDCALL,

    MS_DECLSPEC,
    MS_STDCALL,
    MS_PRAGMA,
    MS_INLINE,
    MS_UUID,
    // SAL (Microsoft Source Annotation Language) https://msdn.microsoft.com/fr-fr/library/hh916382.aspx?f=255&MSPPError=-2147217396
    SAL_IN,    // Should stay the first SAL_* keyword or update isSALKeyword method
    SAL_IN_OPT,
    SAL_IN_Z,
    SAL_IN_OPT_Z,
    SAL_IN_READS,
    SAL_IN_READS_OPT,
    SAL_IN_READS_BYTES,
    SAL_IN_READS_BYTES_OPT,
    SAL_IN_READS_Z,
    SAL_IN_READS_OPT_Z,
    SAL_IN_READS_OR_Z,
    SAL_IN_READS_OR_Z_OPT,
    SAL_IN_READS_TO_PTR,
    SAL_IN_READS_TO_PTR_OPT,
    SAL_IN_READS_TO_PTR_Z,
    SAL_IN_READS_TO_PTR_OPT_Z,
    SAL_OUT,
    SAL_OUT_OPT,
    SAL_OUT_WRITES,
    SAL_OUT_WRITES_OPT,
    SAL_OUT_WRITES_BYTES,
    SAL_OUT_WRITES_BYTES_OPT,
    SAL_OUT_WRITES_Z,
    SAL_OUT_WRITES_OPT_Z,
    SAL_OUT_WRITES_TO,
    SAL_OUT_WRITES_TO_OPT,
    SAL_OUT_WRITES_ALL,
    SAL_OUT_WRITES_ALL_OPT,
    SAL_OUT_WRITES_BYTES_TO,
    SAL_OUT_WRITES_BYTES_TO_OPT,
    SAL_OUT_WRITES_BYTES_ALL,
    SAL_OUT_WRITES_BYTES_ALL_OPT,
    SAL_OUT_WRITES_TO_PTR,
    SAL_OUT_WRITES_TO_PTR_OPT,
    SAL_OUT_WRITES_TO_PTR_Z,
    SAL_OUT_WRITES_TO_PTR_OPT_Z,
    SAL_INOUT,
    SAL_INOUT_OPT,
    SAL_INOUT_Z,
    SAL_INOUT_OPT_Z,
    SAL_INOUT_UPDATES,
    SAL_INOUT_UPDATES_OPT,
    SAL_INOUT_UPDATES_Z,
    SAL_INOUT_UPDATES_OPT_Z,
    SAL_INOUT_UPDATES_TO,
    SAL_INOUT_UPDATES__TO_OPT,
    SAL_INOUT_UPDATES_ALL,
    SAL_INOUT_UPDATES_ALL_OPT,
    SAL_INOUT_UPDATES_BYTES,
    SAL_INOUT_UPDATES_BYTES_OPT,
    SAL_INOUT_UPDATES_BYTES_TO,
    SAL_INOUT_UPDATES_BYTES_TO_OPT,
    SAL_INOUT_UPDATES_BYTES_ALL,
    SAL_INOUT_UPDATES_BYTES_ALL_OPT,
    SAL_OUTPTR,
    SAL_OUTPTR_RESULT_MAYBENULL,
    SAL_OUTPTR_OPT,
    SAL_OUTPTR_OPT_RESULT_MAYBENULL,
    SAL_OUTPTR_RESULT_Z,
    SAL_OUTPTR_OPT_RESULT_Z,
    SAL_OUTPTR_RESULT_MAYBENULL_Z,
    SAL_OUTPTR_OPT_RESULT_MAYBENULL_Z,
    SAL_OUTPTR_RESULT_NULLONFAILURE,
    SAL_OUTPTR_OPT_RESULT_NULLONFAILURE,    // Should stay the last SAL_* keyword or update isSALKeyword method
    // --

    // Compiler keywords to ignore // Warning (update keywordToIgnore)
    MS_CDECL,
    MS_FASTCALL,
    // --

    // Types
    ConstModifier,
    SignedModifier,
    UnsignedModifier,
    ShortModifier,      // Short is a modifier, look at Long comment
    LongModifier,       // Long isn't a type, but a modifier. We can use long modifier alone because of C legacy, when the type is unspecified it is assumed as int (so a long is equivalent to long int)

    // Let fundamental type keywords at the end
    Void,
    Bool,
    MS_Bool,
    Char,
    WChar,  // wchar_t is a compiler type in C++11 and a typedef in C or previous versions of C++
    Int,
    MS_Int64,
    Float,
    Double,
};

inline bool isWhitePunctuation(Punctuation punctuation)
{
    return punctuation >= Punctuation::WhiteCharacter;
}

inline bool isSALKeyword(Keyword keyword)
{
    return keyword >= Keyword::SAL_IN && keyword <= Keyword::SAL_OUTPTR_OPT_RESULT_NULLONFAILURE;
}
