#pragma once

#include "incg_parser.hpp"

#include <filesystem>
#include <string>
#include <vector>

/*
	This function print on the standard output and generate an image that represent the graph of the includes.
	It use dot binary from the Graphiz framework to generate the image.
*/
void	generate_includes_graph(const incg::Configuration& configuration);
