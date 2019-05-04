#include "cpp_includes_graph.hpp"

int main(int ac, char** av)
{
	std::vector<Project>	projects = {
		{
			/* name */ "DriveCubes",
			/* sources_folders */ {"C:/Users/Xavier/Documents/development/DriveCubes/sources"},
			/* include_directories */ {}
		},
	};

	generate_includes_graph(projects, "C:/Users/Xavier/Documents/development/cpp_includes_graph/results");

	return 0;
}
