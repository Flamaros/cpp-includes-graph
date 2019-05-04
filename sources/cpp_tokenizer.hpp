#pragma once

#include "cpp_language_definitions.hpp"

#include "string_ref.hpp"

#include <vector>

struct Token
{
private:
    friend bool operator ==(const Token& lhs, const Token& rhs);

public:
    Punctuation punctuation = Punctuation::Unknown;
    Keyword     keyword = Keyword::Unknown;
    string_ref  text;
    size_t      line;       // Starting from 1
    size_t      column;     // Starting from 1
};

inline bool operator==(const Token& lhs, const Token& rhs)
{
    return lhs.text == rhs.text;
}

void    tokenize(const std::string& text, std::vector<Token>& tokens);
