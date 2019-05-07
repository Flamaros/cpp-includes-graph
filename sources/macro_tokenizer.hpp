#pragma once

#include "macro_language_definitions.hpp"

#include <vector>
#include <string_view>

struct Token
{
private:
    friend bool operator ==(const Token& lhs, const Token& rhs);

public:
    Punctuation			punctuation = Punctuation::unknown;
    Keyword				keyword = Keyword::_unknown;
    std::string_view	text;
    size_t				line;       // Starting from 1
    size_t				column;     // Starting from 1
};

inline bool operator==(const Token& lhs, const Token& rhs)
{
    return lhs.text == rhs.text;
}

void    tokenize(const std::string& text, std::vector<Token>& tokens);
