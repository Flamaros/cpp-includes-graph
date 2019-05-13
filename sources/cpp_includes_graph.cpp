#include "cpp_includes_graph.hpp"

#include "macro_tokenizer.hpp"
#include "macro_parser.hpp"

#include "utilities.hpp"

#include <algorithm>	// std::transform std::to_lower
#include <fstream>
#include <iostream>		// std::cout
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace fs = std::filesystem;

enum class File_Type {
	not_supported,
	source,
	header
};

struct File_Node {
	std::string				unique_name;
	std::string				label;			// Relative header_path
	fs::path				path;
	File_Type				file_type;
	std::vector<File_Node*>	parents;
	std::vector<File_Node*>	children;
	bool					printed = false;
	bool					file_found;
	size_t					nb_inclusions = 0;
	size_t					nb_lines = 0;
	std::string				string_views_buffer;
};

struct Project_Result {
	const incg::Project*						project;
	std::vector<File_Node*>						root_nodes;			// Every source file is a root node
	std::unordered_map<std::string, File_Node*>	nodes;				// All nodes by name
};

std::unordered_set<std::string>	header_extensions = {
	".h",
	".hpp",
	".hxx",
	".inl",
	".impl",
};

std::unordered_set<std::string>	source_extensions = {
	".c",
	".cpp",
	".cxx",
};

static std::string get_unique_name(const Project_Result& result)
{
	size_t		seed = result.nodes.size();
	std::string	unique;

	do
	{
		char digit = 'a' + (seed % 26);
		unique += digit;

		seed = seed / 26;
	} while (seed > 0);

	return unique;
}

static File_Type get_file_type(const fs::path& file_path)
{
	std::string extension = file_path.extension().string();

	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
	if (header_extensions.find(extension) != header_extensions.end()) {
		return File_Type::header;
	}
	if (source_extensions.find(extension) != source_extensions.end()) {
		return File_Type::source;
	}

	return File_Type::not_supported;
}

static void get_includes(File_Node* node, std::vector<std::string_view>& includes)
{
	std::vector<macro::Token>	tokens;
	macro::Macro_Parsing_Result	parsing_result;

	if (read_all_file(node->path, node->string_views_buffer) == false) {
		return;
	}

	tokenize(node->string_views_buffer, tokens);
	parse_macros(tokens, parsing_result);

	node->nb_lines = tokens.back().line;

	includes.reserve(parsing_result.includes.size());

	// @TODO resolve macro conditions here
	for (const macro::Include& include : parsing_result.includes) {
		includes.push_back(include.path);
	}
}

/// Return the full header_path if it is able to find it
/// else return the include_path
static bool get_include_path(const incg::Project& project, const fs::path& source_folder, const File_Node* parent, const fs::path& include_path, fs::path& header_path, std::string& relative_path_with_parent)
{
	fs::path	parent_directory = parent->path.parent_path();

	// Relative to the parent header_path
	header_path = parent_directory / include_path;
	if (fs::exists(header_path)) {
		relative_path_with_parent = (source_folder.filename() / header_path.lexically_relative(source_folder)).generic_string();	// @Warning we put the base of source directory to avoid conflicts if there is many similar source trees with a different root
		return true;
	}

	// Relative to a source directory
	for (const fs::path& directory : project.sources_folders)
	{
		header_path = directory / include_path;
		if (fs::exists(header_path)) {
			relative_path_with_parent = (directory.filename() / header_path.lexically_relative(directory)).generic_string();	// @Warning we put the base of source directory to avoid conflicts if there is many similar source trees with a different root
			return true;
		}
	}

	// Relative to an include directory
	for (const fs::path& directory : project.include_directories)
	{
		header_path = directory / include_path;
		if (fs::exists(header_path)) {
			relative_path_with_parent = (directory.filename() / header_path.lexically_relative(directory)).generic_string();	// @Warning we put the base of source directory to avoid conflicts if there is many similar source trees with a different root
			return true;
		}
	}

	header_path = include_path;
	relative_path_with_parent = include_path.generic_string();
	return false;
}

/// Generate the node tree from the given node (basically fill the children member of the node)
/// This is a recursive function
static void generate_includes_graph(const incg::Project& project, const fs::path& source_folder, File_Node* parent, Project_Result& result)
{
	std::vector<std::string_view>	includes;

	includes.reserve(64);
	parent->children.reserve(64);
	get_includes(parent, includes);

	for (const std::string_view& include : includes) 
	{
		std::string	label;
		fs::path	header_path;
		bool		file_found;
		
		file_found = get_include_path(project, source_folder, parent, include, header_path, label);

		auto it = result.nodes.find(label);

		if (it != result.nodes.end())	// No need to create the node as it already exist
		{
			File_Node* node = it->second;

			node->nb_inclusions++;
			node->parents.push_back(parent);

			parent->children.push_back(node);	// Simply link it to his new parent (inlcuder)
		}
		else
		{
			File_Node*	node = new File_Node;

			node->unique_name = get_unique_name(result);
			node->label = label;
			node->path = header_path;
			node->file_type = File_Type::header;
			node->file_found = file_found;
			node->nb_inclusions++;
			node->parents.push_back(parent);

			parent->children.push_back(node);

			result.nodes.insert(std::pair<std::string, File_Node*>(node->label, node));

			generate_includes_graph(project, source_folder, node, result);
		}
	}
}

/// This is a recursive function
static void print_node(std::ofstream& stream, File_Node* node)
{
	if (node->printed) {
		return;
	}

	std::string border_color;
	std::string background_color;

	// https://www.graphviz.org/doc/info/colors.html
	if (node->file_found) {
		border_color = "black";
	}
	else {
		border_color = "red";
	}

	if (node->file_type == File_Type::source) {
		background_color = "lightseagreen";
	}
	else {
		background_color = "orange";
	}

	std::string label;

	if (node->file_type == File_Type::header) {
		label += std::to_string(node->nb_inclusions) + "x\n";
	}
	label += node->label;
	if (node->nb_lines) {
		label += " (" + std::to_string(node->nb_lines) + " loc)";
	}

	stream << "\t" << node->unique_name << " [label=\"" << label << "\" shape=box, style=filled, color=" << border_color << ", fillcolor=" << background_color << "]" << std::endl;
	for (File_Node* child_node : node->children) {
		stream << "\t" << node->unique_name << " -> " << child_node->unique_name << std::endl;
		print_node(stream, child_node);
	}

	node->printed = true;
};

// @TODO use dot as library instead as binary ?
static void	generate_includes_graph(const incg::Project& project, const fs::path& output_folder, Project_Result& result)
{
	std::ofstream	dot_file;
	std::string		dot_filepath;
	std::string		png_filepath;

	auto generating_dot_start = std::chrono::high_resolution_clock::now();
	{
		dot_filepath = output_folder.generic_string() + "/" + std::string(project.name) + ".dot";
		png_filepath = output_folder.generic_string() + "/" + std::string(project.name) + ".png";
		dot_file.open(dot_filepath, std::fstream::out | std::fstream::binary);
		if (dot_file.is_open() == false) {
			std::cout << "Error: unable to open file " << dot_filepath << std::endl;
			return;
		}

		std::cout << "Project: " << project.name << std::endl;

		result.project = &project;

		for (const fs::path& source_folder : project.sources_folders)
		{
			for (const auto& entry : fs::recursive_directory_iterator(source_folder))
			{
				if (entry.is_regular_file() == false)
					continue;

				if (get_file_type(entry.path()) != File_Type::source)	// Headers are children of sources files
					continue;

				// @TODO create nodes of header files directly here and add them to the result.nodes map but not to the result.root_nodes
				// by doing it, it will reveal orphan header files in the graph (no source parent)

				File_Node * node = new File_Node;

				node->unique_name = get_unique_name(result);
				node->label = (source_folder.filename() / entry.path().lexically_relative(source_folder)).generic_string();	// @Warning we put the base of source directory to avoid conflicts if there is many similar source trees with a different root
				node->path = entry.path();
				node->file_type = File_Type::source;
				node->file_found = true;

				result.nodes.insert(std::pair<std::string, File_Node*>(node->label, node));

				generate_includes_graph(project, source_folder, node, result);

				result.root_nodes.push_back(node);
			}
		}



		// @TODO we also need to retrieve headers that are root nodes, stored in result.nodes
		// I think that we can simply iterate over nodes in a non recursive way

		// Generate the dot file
		{
			dot_file << "digraph {" << std::endl;
			dot_file << "\t" "rankdir = LR" << std::endl;

			for (size_t root_index = 0; root_index < result.root_nodes.size(); root_index++) {
				print_node(dot_file, result.root_nodes[root_index]);
			}

			dot_file << "}" << std::endl;
		}
	}
	auto generating_dot_end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> generating_dot_duration = generating_dot_end - generating_dot_start;

	// Generate the graph image
	auto generating_image_start = std::chrono::high_resolution_clock::now();
	{
		std::string	command_line;
		int			command_line_result;

		command_line = "dot.exe " + dot_filepath + " -Tpng -o " + png_filepath;
		command_line_result = system(command_line.c_str());
		if (command_line_result != 0) {
			std::cerr << "Command line : \"" << command_line << "\" failed." << std::endl
				<< "Do you have installed Graphiz tools and put the bin folder into the PATH environment variable? [You can download it at: https://www.graphviz.org/]." << std::endl;
		}
	}
	auto generating_image_end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> generating_image_duration = generating_image_end - generating_image_start;

	// Print some stats
	{
		size_t	nb_source_files = 0;
		size_t	nb_source_lines = 0;
		size_t	nb_header_files = 0;
		size_t	nb_header_lines = 0;
		size_t	nb_header_not_found = 0;

		for (const auto& pair : result.nodes) {
			const File_Node* node = pair.second;

			if (node->file_type == File_Type::source) {
				nb_source_files++;
			}
			else {
				nb_header_files++;
			}

			if (node->file_type == File_Type::source) {
				nb_source_lines += node->nb_lines;
			}
			else {
				nb_header_lines += node->nb_lines;
			}

			if (node->file_found == false) {
				nb_header_not_found++;
			}
		}

		std::cout << std::fixed << std::setprecision(3);

		std::cout << "\t" "Dot file generated in: " << generating_dot_duration.count() << "s" << std::endl;
		std::cout << "\t" "Image generated in: " << generating_image_duration.count() << "s" << std::endl;
		std::cout << std::endl;

		std::cout << "\t" "Source files: " << nb_source_files << " - Lines of code: " << nb_source_lines << " - Average lines of code per file: " << (double)nb_source_lines / (double)nb_source_files << std::endl;
		std::cout << "\t" "Header files: " << nb_header_files << " - Not found: " << nb_header_not_found << " - Lines of code: " << nb_header_lines << " - Average lines of code per file: " << (double)nb_header_lines / (double)nb_header_files << std::endl;
		std::cout << "\t" "Total lines of code: " << nb_source_lines + nb_header_lines << " - Number of lines ratio (header / source): " << (double)nb_header_lines / (double)nb_source_lines << std::endl;
		std::cout << std::endl;
	}
}

void generate_includes_graph(const incg::Configuration& configuration)
{
	std::vector<Project_Result>	results;

	results.resize(configuration.projects.size());

	// @TODO launch that in threads (check outputs to std::cout first)
	for (size_t project_index = 0; project_index < configuration.projects.size(); project_index++)
	{
		std::string_view	output_folder = configuration.projects[project_index].output_folder;

		fs::create_directories(output_folder);
		if (fs::is_directory(output_folder) == false) {
			std::cout << "Error: unable to find or create the directory " << output_folder << std::endl;
			return;
		}

		generate_includes_graph(configuration.projects[project_index], output_folder, results[project_index]);
	}
}
