#pragma once

#include "pch.h"

struct ModDataEntry {
public:
    std::string version;
    std::string downloadUrl;
};

std::vector<ModDataEntry> FindModDatasInGithub(const std::string& githubUrl);