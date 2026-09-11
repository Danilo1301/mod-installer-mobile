#pragma once

#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <iterator>
#include <random>
#include <sstream>
#include <iomanip>

bool ReadFile(const std::string& path, std::string& content);

size_t WriteToString(void* contents, size_t size, size_t count, void* userData);

std::string GenerateRandomId();