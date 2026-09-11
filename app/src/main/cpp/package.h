#pragma once

#include "pch.h"

struct PackageData
{
    std::string Path;
    std::vector<std::pair<std::string, std::string>> Rules;
    std::vector<std::string> Dependencies;
};

bool ParsePackage(const std::string& content, PackageData& package);