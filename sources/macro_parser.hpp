#pragma once

#include "macro_tokenizer.hpp"

enum class Include_Type
{
	local,
	external
};

struct Include
{
	Include_Type	type;
	string_ref		path;
};

struct Cpp_Parse_Result
{
	std::vector<Include>	includes;
};

void parse_cpp(const std::vector<Token>& tokens, Cpp_Parse_Result& result);
