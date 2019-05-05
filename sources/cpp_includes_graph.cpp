#include "cpp_includes_graph.hpp"

#include "macro_tokenizer.hpp"
#include "macro_parser.hpp"

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
	std::string				label;			// Relative path
	fs::path				path;
	File_Type				file_type;
	std::vector<File_Node*>	parents;
	std::vector<File_Node*>	children;
};

struct Project_Result {
	const Project*								project;
	std::vector<File_Node*>						root_nodes;			// Every source file is a root node
	std::unordered_map<std::string, File_Node*>	nodes;				// All nodes by name
};

std::unordered_set<std::string>	header_extensions = {
	".h",
	".hpp",
	".hxx",
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

static bool read_all_file(const fs::path& file_path, std::string& data)
{
	std::ifstream   file(file_path, std::fstream::binary);
	std::streampos  file_size;

	if (file.is_open() == false) {
		return false;
	}

	file.seekg(0, file.end);
	file_size = file.tellg();
	file.seekg(0, file.beg);

	data.resize((size_t)file_size);
	file.read(reinterpret_cast<char*>(data.data()), data.size());
	return file.fail() == false;
}

static void get_includes(const fs::path& file_path, std::vector<std::string>& includes)
{
	std::string			data;
	std::vector<Token>	tokens;
	Cpp_Parse_Result	parsing_result;

	if (read_all_file(file_path, data) == false) {
		return;
	}

	tokenize(data, tokens);
	parse_cpp(tokens, parsing_result);

	includes.reserve(parsing_result.includes.size());

	// @TODO resolve macro conditions here
	for (const Include& include : parsing_result.includes) {
		includes.push_back(include.path.to_string());
	}
}

/// Return the full path if it is able to find it
/// else return the header_path
static fs::path get_include_path(const Project& project, fs::path& parent_path, const fs::path& header_path)
{
	// TODO look relatively to the directory of the parent
	// TODO look to the sources directories (as VS certainly implicitly add it to includes directories)
	// TODO look to the includes directories

	return header_path;
}

/// Generate the node tree from the given node (basically fill the children member of the node)
/// This is a recursive function
static void generate_includes_graph(const Project& project, File_Node* parent, Project_Result& result)
{
	std::vector<std::string>	includes;

	includes.reserve(64);
	parent->children.reserve(64);
	get_includes(parent->path, includes);

	for (const std::string& include : includes) 
	{
		fs::path	header_path = get_include_path(project, parent->path, include);

		std::string	label = header_path.generic_string();	// @TODO we may need to clean the label here just like for sources

		auto it = result.nodes.find(label);

		if (it != result.nodes.end())	// No need to create the node as it already exist
		{
			it->second->parents.push_back(parent);
			parent->children.push_back(it->second);	// Simply link it to his new parent (inlcuder)
		}
		else
		{
			File_Node*	node = new File_Node;

			node->unique_name = get_unique_name(result);
			node->label = label;
			node->path = header_path;
			node->file_type = File_Type::header;
			node->parents.push_back(parent);

			parent->children.push_back(node);

			result.nodes.insert(std::pair<std::string, File_Node*>(node->label, node));

			generate_includes_graph(project, node, result);
		}
	}
}

/// This is a recursive function
static void print_node(std::ofstream& stream, const File_Node* node)
{
	std::string background_color;

	// https://www.graphviz.org/doc/info/colors.html
	if (node->file_type == File_Type::source) {
		background_color = "lightseagreen";
	}
	else {
		background_color = "orange";
	}

	stream << "\t" << node->unique_name << " [label=\"" << node->label << "\" shape=box, style=filled, fillcolor=" << background_color << "]" << std::endl;
	for (const File_Node* child_node : node->children) {
		stream << "\t" << node->unique_name << " -> " << child_node->unique_name << std::endl;
		print_node(stream, child_node);
	}
};

// @TODO use dot as library instead as binary ?
static void	generate_includes_graph(const Project& project, const fs::path& output_folder, Project_Result& result)
{
	std::ofstream	dot_file;
	std::string		dot_filepath;

	dot_filepath = output_folder.generic_string() + "/" + project.name + ".dot";
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

			File_Node* node = new File_Node;

			node->unique_name = get_unique_name(result);
			node->label = (source_folder.filename() / entry.path().lexically_relative(source_folder)).generic_string();	// @Warning we put the base of source directory to avoid conflicts if there is many similar source trees with a different root
			node->path = entry.path();
			node->file_type = File_Type::source;

			result.nodes.insert(std::pair<std::string, File_Node*>(node->label, node));

			generate_includes_graph(project, node, result);

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

void generate_includes_graph(const std::vector<Project>& projects, const std::filesystem::path& output_folder)
{
	std::vector<Project_Result>	results;

	results.resize(projects.size());

	fs::create_directories(output_folder);
	if (fs::is_directory(output_folder) == false) {
		std::cout << "Error: unable to find or create the directory " << output_folder << std::endl;
		return;
	}

	// @TODO launch that in threads (check outputs to std::cout first)
	for (size_t project_index = 0; project_index < projects.size(); project_index++)
	{
		generate_includes_graph(projects[project_index], output_folder, results[project_index]);
	}
}
