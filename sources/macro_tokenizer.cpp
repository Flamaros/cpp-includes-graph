#include "macro_tokenizer.hpp"

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
	{std::string("include"),	Keyword::_include},
	{std::string("define"),		Keyword::_define},
	{std::string("undef"),		Keyword::_undef},
	{std::string("pragma"),		Keyword::_pragma},
	{std::string("if"),			Keyword::_if},
	{std::string("else"),		Keyword::_else},
	{std::string("endif"),		Keyword::_endif},
	{std::string("ifdef"),		Keyword::_ifdef},
	{std::string("ifndef"),		Keyword::_ifndef},
	{std::string("defined"),	Keyword::_defined},
	{std::string("error"),		Keyword::_error},
};

static Keyword isKeyword(const string_ref& text)
{
//    return keywords.getValue(text, keywordKey(text));
    const auto& it = keywords.find(text.to_string());
    if (it != keywords.end())
        return it->second;
    return Keyword::_unknown;
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
        token.keyword = punctuation == Punctuation::Unknown ? isKeyword(text) : Keyword::_unknown;

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
