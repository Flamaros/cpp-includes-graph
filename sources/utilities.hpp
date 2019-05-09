#pragma once

#include <filesystem>
#include <string>

bool read_all_file(const std::filesystem::path& file_path, std::string& data);
