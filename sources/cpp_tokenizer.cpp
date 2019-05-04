#include "cpp_tokenizer.hpp"

#include "hash_table.hpp"

#include <unordered_map>

static const std::size_t    tokens_length_heuristic = 6;

/// Return a key for the punctuation of 2 characters
constexpr static std::uint16_t punctuationKey_2(const char* str)
{
    return ((std::uint16_t)str[0] << 8) | (std::uint16_t)str[1];
}

static hash_table<std::uint16_t, Punctuation, Punctuation::Unknown> punctuationTable_2 = {
    {punctuationKey_2("//"), Punctuation::LineComment},
    {punctuationKey_2("/*"), Punctuation::OpenBlockComment},
    {punctuationKey_2("*/"), Punctuation::CloseBlockComment},
    {punctuationKey_2("->"), Punctuation::Arrow},
    {punctuationKey_2("&&"), Punctuation::And},
    {punctuationKey_2("||"), Punctuation::Or},
    {punctuationKey_2("::"), Punctuation::DoubleColon},
    {punctuationKey_2("=="), Punctuation::EqualityTest},
    {punctuationKey_2("!="), Punctuation::DifferenceTest},
};

static hash_table<std::uint8_t, Punctuation, Punctuation::Unknown> punctuationTable_1 = {
    // White characters (aren't handle for an implicit skip/separation between tokens)
    {' ', Punctuation::WhiteCharacter},       // space
    {'\t', Punctuation::WhiteCharacter},      // horizontal tab
    {'\v', Punctuation::WhiteCharacter},      // vertical tab
    {'\f', Punctuation::WhiteCharacter},      // feed
    {'\r', Punctuation::WhiteCharacter},      // carriage return
    {'\n', Punctuation::NewLineCharacter},    // newline
    {'~', Punctuation::Tilde},
    {'`', Punctuation::Backquote},
    {'!', Punctuation::Bang},
    {'@', Punctuation::At},
    {'#', Punctuation::Hash},
    {'$', Punctuation::Dollar},
    {'%', Punctuation::Percent},
    {'^', Punctuation::Caret},
    {'&', Punctuation::Ampersand},
    {'*', Punctuation::Star},
    {'(', Punctuation::OpenParenthesis},
    {')', Punctuation::CloseParenthesis},
//  {'_', Token::Underscore},
    {'-', Punctuation::Dash},
    {'+', Punctuation::Plus},
    {'=', Punctuation::Equals},
    {'{', Punctuation::OpenBrace},
    {'}', Punctuation::CloseBrace},
    {'[', Punctuation::OpenBracket},
    {']', Punctuation::CloseBracket},
    {':', Punctuation::Colon},
    {';', Punctuation::Semicolon},
    {'\'', Punctuation::SingleQuote},
    {'"', Punctuation::DoubleQuote},
    {'|', Punctuation::Pipe},
    {'/', Punctuation::Slash},
    {'\\', Punctuation::Backslash},
    {'<', Punctuation::Less},
    {'>', Punctuation::Greater},
    {',', Punctuation::Comma},
    {'.', Punctuation::Dot},
    {'?', Punctuation::QuestionMark}
};

/// This implemenation doesn't do any lookup in tables
/// Instead it use hash tables specialized by length of punctuation
static Punctuation endingPunctuation(const string_ref& text, int& punctuationLength)
{
    Punctuation punctuation = Punctuation::Unknown;

    punctuationLength = 2;
    if (text.length() >= 2)
        punctuation = punctuationTable_2[punctuationKey_2(text.starting_ptr() + text.length() - 2)];
    if (punctuation != Punctuation::Unknown)
        return punctuation;
    punctuationLength = 1;
    if (text.length() >= 1)
        punctuation = punctuationTable_1[*(text.starting_ptr() + text.length() - 1)];
    return punctuation;
}

static std::unordered_map<std::string, Keyword> keywords = {
    {std::string("typedef"),         Keyword::Typedef},
    {std::string("extern"),          Keyword::Extern},
    {std::string("class"),           Keyword::Class},
    {std::string("struct"),          Keyword::Struct},
    {std::string("enum"),            Keyword::Enum},
    {std::string("union"),           Keyword::Union},
    {std::string("static"),          Keyword::Static},
    {std::string("virtual"),         Keyword::Virtual},
    {std::string("override"),        Keyword::Override},
    {std::string("public"),          Keyword::Public},
    {std::string("protected"),       Keyword::Protected},
    {std::string("private"),         Keyword::Private},
    {std::string("friend"),          Keyword::Friend},
    {std::string("template"),        Keyword::Template},
    {std::string("typename"),        Keyword::Typename},
    {std::string("return"),          Keyword::Return},
    {std::string("operator"),        Keyword::Operator},
    {std::string("if"),              Keyword::If},
    {std::string("else"),            Keyword::Else},
    {std::string("switch"),          Keyword::Switch},
    {std::string("case"),            Keyword::Case},
    {std::string("default"),         Keyword::Default},
    {std::string("for"),             Keyword::For},
    {std::string("while"),           Keyword::While},
    {std::string("do"),              Keyword::Do},
    {std::string("goto"),            Keyword::Goto},
    {std::string("label"),           Keyword::Label},
    {std::string("inline"),          Keyword::Inline},
    {std::string("__forceinline"),   Keyword::Inline},
    {std::string("new"),             Keyword::New},
    {std::string("delete"),          Keyword::Delete},
    {std::string("try"),             Keyword::Try},
    {std::string("catch"),           Keyword::Catch},
    {std::string("throw"),           Keyword::Throw},
    {std::string("noexcept"),        Keyword::Noexcept},
    {std::string("static_assert"),   Keyword::StaticAssert},
    {std::string("namespace"),       Keyword::Namespace},
    {std::string("explicit"),       Keyword::Explicit},

    // Values
    {std::string("true"),            Keyword::True},
    {std::string("false"),           Keyword::False},
    {std::string("nullptr"),         Keyword::Nullptr},
    // --

    // Compiler specific keywords
    {std::string("__attribute__"),   Keyword::GNU_ATTRIBUTE},
    {std::string("__stdcall__"),     Keyword::GNU_STDCALL},

    {std::string("__declspec"),      Keyword::MS_DECLSPEC},
    {std::string("__stdcall"),       Keyword::MS_STDCALL},
    {std::string("__pragma"),        Keyword::MS_PRAGMA},
    {std::string("__inline"),        Keyword::MS_INLINE},
    {std::string("uuid"),            Keyword::MS_UUID},

    // SAL (from sal.h C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\include\sal.h)
    {std::string("_In_"),                               Keyword::SAL_IN},
    {std::string("_In_opt_"),                           Keyword::SAL_IN_OPT},
    {std::string("_In_z_"),                             Keyword::SAL_IN_Z},
    {std::string("_In_opt_z_"),                         Keyword::SAL_IN_OPT_Z},
    {std::string("_In_reads_"),                         Keyword::SAL_IN_READS},
    {std::string("_In_reads_opt_"),                     Keyword::SAL_IN_READS_OPT},
    {std::string("_In_reads_bytes_"),                   Keyword::SAL_IN_READS_BYTES},
    {std::string("_In_reads_bytes_opt_"),               Keyword::SAL_IN_READS_BYTES_OPT},
    {std::string("_In_reads_z_"),                       Keyword::SAL_IN_READS_Z},
    {std::string("_In_reads_opt_z_"),                   Keyword::SAL_IN_READS_OPT_Z},
    {std::string("_In_reads_or_z_"),                    Keyword::SAL_IN_READS_OR_Z},
    {std::string("_In_reads_or_z_opt_"),                Keyword::SAL_IN_READS_OR_Z_OPT},
    {std::string("_In_reads_to_ptr_"),                  Keyword::SAL_IN_READS_TO_PTR},
    {std::string("_In_reads_to_ptr_opt_"),              Keyword::SAL_IN_READS_TO_PTR_OPT},
    {std::string("_In_reads_to_ptr_z_"),                Keyword::SAL_IN_READS_TO_PTR_Z},
    {std::string("_In_reads_to_ptr_opt_z_"),            Keyword::SAL_IN_READS_TO_PTR_OPT_Z},
    {std::string("_Out_"),                              Keyword::SAL_OUT},
    {std::string("_Out_opt_"),                          Keyword::SAL_OUT_OPT},
    {std::string("_Out_writes_"),                       Keyword::SAL_OUT_WRITES},
    {std::string("_Out_writes_opt_"),                   Keyword::SAL_OUT_WRITES_OPT},
    {std::string("_Out_writes_bytes_"),                 Keyword::SAL_OUT_WRITES_BYTES},
    {std::string("_Out_writes_bytes_opt_"),             Keyword::SAL_OUT_WRITES_BYTES_OPT},
    {std::string("_Out_writes_z_"),                     Keyword::SAL_OUT_WRITES_Z},
    {std::string("_Out_writes_opt_z_"),                 Keyword::SAL_OUT_WRITES_OPT_Z},
    {std::string("_Out_writes_to_"),                    Keyword::SAL_OUT_WRITES_TO},
    {std::string("_Out_writes_to_opt_"),                Keyword::SAL_OUT_WRITES_TO_OPT},
    {std::string("_Out_writes_all_"),                   Keyword::SAL_OUT_WRITES_ALL},
    {std::string("_Out_writes_all_opt_"),               Keyword::SAL_OUT_WRITES_ALL_OPT},
    {std::string("_Out_writes_bytes_to_"),              Keyword::SAL_OUT_WRITES_BYTES_TO},
    {std::string("_Out_writes_bytes_to_opt_"),          Keyword::SAL_OUT_WRITES_BYTES_TO_OPT},
    {std::string("_Out_writes_bytes_all_"),             Keyword::SAL_OUT_WRITES_BYTES_ALL},
    {std::string("_Out_writes_bytes_all_opt_"),         Keyword::SAL_OUT_WRITES_BYTES_ALL_OPT},
    {std::string("_Out_writes_to_ptr_"),                Keyword::SAL_OUT_WRITES_TO_PTR},
    {std::string("_Out_writes_to_ptr_opt_"),            Keyword::SAL_OUT_WRITES_TO_PTR_OPT},
    {std::string("_Out_writes_to_ptr_z_"),              Keyword::SAL_OUT_WRITES_TO_PTR_Z},
    {std::string("_Out_writes_to_ptr_opt_z_"),          Keyword::SAL_OUT_WRITES_TO_PTR_OPT_Z},
    {std::string("_Inout_"),                            Keyword::SAL_INOUT},
    {std::string("_Inout_opt_"),                        Keyword::SAL_INOUT_OPT},
    {std::string("_Inout_z_"),                          Keyword::SAL_INOUT_Z},
    {std::string("_Inout_opt_z_"),                      Keyword::SAL_INOUT_OPT_Z},
    {std::string("_Inout_updates_"),                    Keyword::SAL_INOUT_UPDATES},
    {std::string("_Inout_updates_opt_"),                Keyword::SAL_INOUT_UPDATES_OPT},
    {std::string("_Inout_updates_z_"),                  Keyword::SAL_INOUT_UPDATES_Z},
    {std::string("_Inout_updates_opt_z_"),              Keyword::SAL_INOUT_UPDATES_OPT_Z},
    {std::string("_Inout_updates_to_"),                 Keyword::SAL_INOUT_UPDATES_TO},
    {std::string("_Inout_updates_to_opt_"),             Keyword::SAL_INOUT_UPDATES__TO_OPT},
    {std::string("_Inout_updates_all_"),                Keyword::SAL_INOUT_UPDATES_ALL},
    {std::string("_Inout_updates_all_opt_"),            Keyword::SAL_INOUT_UPDATES_ALL_OPT},
    {std::string("_Inout_updates_bytes_"),              Keyword::SAL_INOUT_UPDATES_BYTES},
    {std::string("_Inout_updates_bytes_opt_"),          Keyword::SAL_INOUT_UPDATES_BYTES_OPT},
    {std::string("_Inout_updates_bytes_to_"),           Keyword::SAL_INOUT_UPDATES_BYTES_TO},
    {std::string("_Inout_updates_bytes_to_opt_"),       Keyword::SAL_INOUT_UPDATES_BYTES_TO_OPT},
    {std::string("_Inout_updates_bytes_all_"),          Keyword::SAL_INOUT_UPDATES_BYTES_ALL},
    {std::string("_Inout_updates_bytes_all_opt_"),      Keyword::SAL_INOUT_UPDATES_BYTES_ALL_OPT},
    {std::string("_Outptr_"),                           Keyword::SAL_OUTPTR},
    {std::string("_Outptr_result_maybenull_"),          Keyword::SAL_OUTPTR_RESULT_MAYBENULL},
    {std::string("_Outptr_opt_"),                       Keyword::SAL_OUTPTR_OPT},
    {std::string("_Outptr_opt_result_maybenull_"),      Keyword::SAL_OUTPTR_OPT_RESULT_MAYBENULL},
    {std::string("_Outptr_result_z_"),                  Keyword::SAL_OUTPTR_RESULT_Z},
    {std::string("_Outptr_opt_result_z_"),              Keyword::SAL_OUTPTR_OPT_RESULT_Z},
    {std::string("_Outptr_result_maybenull_z_"),        Keyword::SAL_OUTPTR_RESULT_MAYBENULL_Z},
    {std::string("_Outptr_opt_result_maybenull_z_"),    Keyword::SAL_OUTPTR_OPT_RESULT_MAYBENULL_Z},
    {std::string("_Outptr_result_nullonfailure_"),      Keyword::SAL_OUTPTR_RESULT_NULLONFAILURE},
    {std::string("_Outptr_opt_result_nullonfailure_"),  Keyword::SAL_OUTPTR_OPT_RESULT_NULLONFAILURE},

    // Compiler keywords to ignore
    {std::string("__cdecl"),         Keyword::MS_CDECL},
    {std::string("__fastcall"),      Keyword::MS_FASTCALL},

    // Types
    {std::string("const"),           Keyword::ConstModifier},
    {std::string("signed"),          Keyword::SignedModifier},
    {std::string("unsigned"),        Keyword::UnsignedModifier},
    {std::string("short"),           Keyword::ShortModifier},
    {std::string("long"),            Keyword::LongModifier},
    {std::string("void"),            Keyword::Void},
    {std::string("bool"),            Keyword::Bool},
    {std::string("_Bool"),           Keyword::MS_Bool},
    {std::string("char"),            Keyword::Char},
    {std::string("wchar_t"),         Keyword::WChar},
    {std::string("int"),             Keyword::Int},
    {std::string("__int64"),         Keyword::MS_Int64},
    {std::string("float"),           Keyword::Float},
    {std::string("double"),          Keyword::Double}
};

static Keyword isKeyword(const string_ref& text)
{
//    return keywords.getValue(text, keywordKey(text));
    const auto& it = keywords.find(text.to_string());
    if (it != keywords.end())
        return it->second;
    return Keyword::Unknown;
}

void    tokenize(const std::string& buffer, std::vector<Token>& tokens)
{
    tokens.reserve(buffer.length() / tokens_length_heuristic);

    string_ref  previousTokenText;
    string_ref  punctuationText;
    int         startPos = 0;
    int         currentPos = 0;
    int         currentLine = 1;
    int         currentColumn = 1;
    int         textColumn = 1;

    Token       token;
    string_ref  text;
    Punctuation punctuation = Punctuation::Unknown;
    int         punctuationLength = 0;

    auto    generateToken = [&](string_ref text, Punctuation punctuation, size_t column) {
        token.line = currentLine;
        token.column = column;
        token.text = text;
        token.punctuation = punctuation;
        token.keyword = punctuation == Punctuation::Unknown ? isKeyword(text) : Keyword::Unknown;

        tokens.push_back(token);
    };

    // Extracting token one by one (based on the punctuation)
    bool    eof = false;
    while (eof == false)
    {
        string_ref  forwardText;
        Punctuation forwardPunctuation = Punctuation::Unknown;
        int         forwardPunctuationLength = 0;

        if (currentPos + 2 < (int)buffer.length())
        {
            forwardText = string_ref(buffer, startPos, (currentPos - startPos) + 2);
            forwardPunctuation = endingPunctuation(forwardText, forwardPunctuationLength);
        }

        text = string_ref(buffer, startPos, (currentPos - startPos) + 1);
        punctuation = endingPunctuation(text, punctuationLength);

        if (punctuation == Punctuation::NewLineCharacter)
            currentColumn = 0; // 0 because the currentPos was not incremented yet, and the cursor is virtually still on previous line

        if (punctuation != Punctuation::Unknown
            && (forwardPunctuation == Punctuation::Unknown
                || forwardPunctuation >= Punctuation::Tilde
                || punctuation < forwardPunctuation))                 // Mutiple characters ponctuation have a lower enum value
        {
            previousTokenText = string_ref(buffer, text.position(), text.length() - punctuationLength);
            punctuationText = string_ref(buffer, text.position() + text.length() - punctuationLength, punctuationLength);

            if (previousTokenText.length())
                generateToken(previousTokenText, Punctuation::Unknown, textColumn);

            if (isWhitePunctuation(punctuation) == false)
                generateToken(punctuationText, punctuation, currentColumn - punctuationText.length() + 1);

            startPos = currentPos + 1;
            textColumn = currentColumn + 1; // textColumn comes 1 here after a line return
        }

        if (currentPos + 1 >= (int)buffer.length())
        {
            // Handling the case of the last token of stream
            if (punctuation == Punctuation::Unknown)
            {
                previousTokenText = string_ref(buffer, text.position(), text.length());

                if (previousTokenText.length())
                    generateToken(previousTokenText, Punctuation::Unknown, textColumn);
            }

            eof = true;
        }

        if (punctuation == Punctuation::NewLineCharacter)
            currentLine++;

        currentColumn++;
        currentPos++;
    }
}
