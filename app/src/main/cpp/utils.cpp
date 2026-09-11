#include "utils.h"

bool ReadFile(const std::string& path, std::string& content)
{
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open())
        return false;

    content.assign(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>());

    return true;
}

size_t WriteToString(void* contents, size_t size, size_t count, void* userData)
{
    size_t totalSize = size * count;

    std::string* response =
            static_cast<std::string*>(userData);

    response->append(
            static_cast<char*>(contents),
            totalSize);

    return totalSize;
}

std::string GenerateRandomId()
{
    static std::random_device rd;
    static std::mt19937 generator(rd());

    std::uniform_int_distribution<uint64_t> distribution;

    std::stringstream stream;
    stream << std::hex << distribution(generator);

    return stream.str();
}