#pragma once

#include <string>
#include <vector>
#include <unordered_map>

struct ModData
{
    std::string Name = "Mod";
    std::string Author = "Unknown";
    std::string Version = "0.0.0";
    std::string Description;
    std::string Url;
    bool IsRepository = false;
};

std::vector<ModData> ParseModDatabase(const std::string& content);