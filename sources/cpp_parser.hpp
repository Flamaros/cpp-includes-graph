#pragma once

#include "cpp_tokenizer.hpp"

struct Cpp_Parse_Result
{

};

void parse_cpp(const std::vector<Token>& tokens, Cpp_Parse_Result& result);
