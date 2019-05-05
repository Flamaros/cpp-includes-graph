#include "cpp_includes_graph.hpp"

int main(int ac, char** av)
{
	std::vector<Project>	projects = {
		{
			/* name */ "DriveCubes",
			/* sources_folders */ {"C:/Users/Xavier/Documents/development/DriveCubes/sources"},
			/* include_directories */ {
				"C:/Users/Xavier/Documents/development/DriveCubes/dependencies/libjpeg-turbo32/include",
				"C:/Users/Xavier/Documents/development/DriveCubes/dependencies/libjpeg-turbo64/include",
				"C:/Users/Xavier/Documents/development/DriveCubes/dependencies/glm",
				"C:/Users/Xavier/Documents/development/DriveCubes/dependencies/libpng",
				"C:/Users/Xavier/Documents/development/DriveCubes/dependencies/libwebp/src",
				"C:/Users/Xavier/Documents/development/DriveCubes/dependencies/zlib",
			}
		},
	};

	generate_includes_graph(projects, "C:/Users/Xavier/Documents/development/cpp_includes_graph/results");

	return 0;
}
