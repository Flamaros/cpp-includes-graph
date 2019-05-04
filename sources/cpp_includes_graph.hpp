#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct Project {
	std::string							name;
	std::vector<std::filesystem::path>	sources_folders;
	std::vector<std::filesystem::path>	include_directories;
};

/*
	This function print on the standard output and generate an image that represent the graph of the includes.
	It use dot binary from the Graphiz framework to generate the image.
*/
void	generate_includes_graph(const std::vector<Project>& projects, const std::filesystem::path& output_folder);
