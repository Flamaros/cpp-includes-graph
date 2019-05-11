#pragma once

#include "incg_tokenizer.hpp"

#include <vector>
#include <string_view>
#include <filesystem>

namespace incg
{
	struct Project {
		std::string_view				name;
		std::string_view				output_folder;
		std::vector<std::string_view>	sources_folders;
		std::vector<std::string_view>	include_directories;
	};

	struct Configuration
	{
		std::vector<Project>	projects;
		std::string				string_views_buffer;	// @Warning private field
	};

	bool parse_configuration(const std::vector<Token>& tokens, Configuration& result);
}
