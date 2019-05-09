#pragma once

#include "macro_tokenizer.hpp"

#include <vector>
#include <string_view>

namespace macro
{
	enum class Include_Type
	{
		local,
		external
	};

	struct Include
	{
		Include_Type		type;
		std::string_view	path;
	};

	struct Macro_Parsing_Result
	{
		std::vector<Include>	includes;
	};

	void parse_macros(const std::vector<Token>& tokens, Macro_Parsing_Result& result);
}
