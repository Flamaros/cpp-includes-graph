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
		std::filesystem::path	file_path;	/// File path of the configuration file
		std::filesystem::path	base_path;	/// Parent directory of the configuration file (to make all path relative to this directory)
		std::string				__string_views_buffer;	// @Warning private field
	};

	bool parse_configuration(const std::vector<Token>& tokens, Configuration& result);
}
