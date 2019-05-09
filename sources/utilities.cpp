#include "utilities.hpp"

#include <fstream>

namespace fs = std::filesystem;

bool read_all_file(const fs::path& file_path, std::string& data)
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
