#include "cpp_includes_graph.hpp"
#include "incg_parser.hpp"

#include "utilities.hpp"

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

static bool load_configuration_file(const fs::path& path, incg::Configuration& configuration)
{
	std::vector<incg::Token>	tokens;

	if (read_all_file(path, configuration.__string_views_buffer) == false) {
		std::cerr << "Error: Failed to read the configuration file " << path <<  "." << std::endl;
		return false;
	}

	incg::tokenize(configuration.__string_views_buffer, tokens);
	incg::parse_configuration(tokens, configuration);

	if (configuration.projects.empty()) {
		std::cerr << "Error: Configuration file doesn't contains any project." << std::endl;
		return false;
	}

	configuration.file_path = std::filesystem::absolute(path);
	configuration.base_path = configuration.file_path.parent_path();

	return true;
}

int main(int ac, char** av)
{
	if (ac != 2) {
		std::cerr << "Error: No configuration file path specified." << std::endl;
		return 1;
	}

	fs::path			configuration_file_path = av[1];
	incg::Configuration	configuration;

	if (load_configuration_file(configuration_file_path, configuration) == false) {
		return 2;
	}

	generate_includes_graph(configuration);

	return 0;
}
