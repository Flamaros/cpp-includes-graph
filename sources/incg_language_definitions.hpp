#pragma once

#include <stdint.h>

namespace incg
{
	enum class Punctuation : uint8_t
	{
		unknown,

		hash,                   //    #
		open_brace,             //    {
		close_brace,            //    }
		open_bracket,           //    [
		close_bracket,          //    ]
		colon,                  //    :
		double_quote,           //    "
		comma,                  //    ,

		// White character at end to be able to handle correctly lines that terminate with a separator like semicolon just before a line return
		white_character,
		new_line_character
	};

	enum class Keyword
	{
		unknown,

		project,
		name,
		output_folder,
		sources_folders,
		include_directories,
	};

	inline bool is_white_punctuation(Punctuation punctuation)
	{
		return punctuation >= Punctuation::white_character;
	}
}
