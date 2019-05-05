#include "macro_tokenizer.hpp"

#include "hash_table.hpp"

#include <unordered_map>

static const std::size_t    tokens_length_heuristic = 6;

/// Return a key for the punctuation of 2 characters
constexpr static std::uint16_t punctuation_key_2(const char* str)
{
    return ((std::uint16_t)str[0] << 8) | (std::uint16_t)str[1];
}

static Hash_Table<std::uint16_t, Punctuation, Punctuation::unknown> punctuation_table_2 = {
    {punctuation_key_2("//"), Punctuation::line_comment},
    {punctuation_key_2("/*"), Punctuation::open_block_comment},
    {punctuation_key_2("*/"), Punctuation::close_block_comment},
    {punctuation_key_2("->"), Punctuation::arrow},
    {punctuation_key_2("&&"), Punctuation::logical_and},
    {punctuation_key_2("||"), Punctuation::logical_or},
    {punctuation_key_2("::"), Punctuation::double_colon},
    {punctuation_key_2("=="), Punctuation::equality_test},
    {punctuation_key_2("!="), Punctuation::difference_test},
};

static Hash_Table<std::uint8_t, Punctuation, Punctuation::unknown> punctuation_table_1 = {
    // White characters (aren't handle for an implicit skip/separation between tokens)
    {' ', Punctuation::white_character},       // space
    {'\t', Punctuation::white_character},      // horizontal tab
    {'\v', Punctuation::white_character},      // vertical tab
    {'\f', Punctuation::white_character},      // feed
    {'\r', Punctuation::white_character},      // carriage return
    {'\n', Punctuation::new_line_character},   // newline
    {'~', Punctuation::tilde},
    {'`', Punctuation::backquote},
    {'!', Punctuation::bang},
    {'@', Punctuation::at},
    {'#', Punctuation::hash},
    {'$', Punctuation::dollar},
    {'%', Punctuation::percent},
    {'^', Punctuation::caret},
    {'&', Punctuation::ampersand},
    {'*', Punctuation::star},
    {'(', Punctuation::open_parenthesis},
    {')', Punctuation::close_parenthesis},
//  {'_', Token::underscore},
    {'-', Punctuation::dash},
    {'+', Punctuation::plus},
    {'=', Punctuation::equals},
    {'{', Punctuation::open_brace},
    {'}', Punctuation::close_brace},
    {'[', Punctuation::open_bracket},
    {']', Punctuation::close_bracket},
    {':', Punctuation::colon},
    {';', Punctuation::semicolon},
    {'\'', Punctuation::single_quote},
    {'"', Punctuation::double_quote},
    {'|', Punctuation::pipe},
    {'/', Punctuation::slash},
    {'\\', Punctuation::backslash},
    {'<', Punctuation::less},
    {'>', Punctuation::greater},
    {',', Punctuation::comma},
    {'.', Punctuation::dot},
    {'?', Punctuation::question_mark}
};

/// This implemenation doesn't do any lookup in tables
/// Instead it use hash tables specialized by length of punctuation
static Punctuation ending_punctuation(const string_ref& text, int& punctuation_length)
{
    Punctuation punctuation = Punctuation::unknown;

    punctuation_length = 2;
    if (text.length() >= 2)
        punctuation = punctuation_table_2[punctuation_key_2(text.starting_ptr() + text.length() - 2)];
    if (punctuation != Punctuation::unknown)
        return punctuation;
    punctuation_length = 1;
    if (text.length() >= 1)
        punctuation = punctuation_table_1[*(text.starting_ptr() + text.length() - 1)];
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

static Keyword is_keyword(const string_ref& text)
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

    string_ref  previous_token_text;
    string_ref  punctuation_text;
    int         start_position = 0;
    int         current_position = 0;
    int         current_line = 1;
    int         current_column = 1;
    int         text_column = 1;

    Token       token;
    string_ref  text;
    Punctuation punctuation = Punctuation::unknown;
    int         punctuation_length = 0;

    auto    generateToken = [&](string_ref text, Punctuation punctuation, size_t column) {
        token.line = current_line;
        token.column = column;
        token.text = text;
        token.punctuation = punctuation;
        token.keyword = punctuation == Punctuation::unknown ? is_keyword(text) : Keyword::_unknown;

        tokens.push_back(token);
    };

    // Extracting token one by one (based on the punctuation)
    bool    eof = false;
    while (eof == false)
    {
        string_ref  forward_text;
        Punctuation forward_punctuation = Punctuation::unknown;
        int         forward_punctuation_length = 0;

        if (current_position + 2 < (int)buffer.length())
        {
            forward_text = string_ref(buffer, start_position, (current_position - start_position) + 2);
            forward_punctuation = ending_punctuation(forward_text, forward_punctuation_length);
        }

        text = string_ref(buffer, start_position, (current_position - start_position) + 1);
        punctuation = ending_punctuation(text, punctuation_length);

        if (punctuation == Punctuation::new_line_character)
            current_column = 0; // 0 because the current_position was not incremented yet, and the cursor is virtually still on previous line

        if (punctuation != Punctuation::unknown
            && (forward_punctuation == Punctuation::unknown
                || forward_punctuation >= Punctuation::tilde
                || punctuation < forward_punctuation))                 // Mutiple characters ponctuation have a lower enum value
        {
            previous_token_text = string_ref(buffer, text.position(), text.length() - punctuation_length);
            punctuation_text = string_ref(buffer, text.position() + text.length() - punctuation_length, punctuation_length);

            if (previous_token_text.length())
                generateToken(previous_token_text, Punctuation::unknown, text_column);

            if (is_white_punctuation(punctuation) == false)
                generateToken(punctuation_text, punctuation, current_column - punctuation_text.length() + 1);

            start_position = current_position + 1;
            text_column = current_column + 1; // text_column comes 1 here after a line return
        }

        if (current_position + 1 >= (int)buffer.length())
        {
            // Handling the case of the last token of stream
            if (punctuation == Punctuation::unknown)
            {
                previous_token_text = string_ref(buffer, text.position(), text.length());

                if (previous_token_text.length())
                    generateToken(previous_token_text, Punctuation::unknown, text_column);
            }

            eof = true;
        }

        if (punctuation == Punctuation::new_line_character)
            current_line++;

        current_column++;
        current_position++;
    }
}