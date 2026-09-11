#include "package.h"

bool ParsePackage(const std::string& content, PackageData& package)
{
    package.Path.clear();
    package.Dependencies.clear();
    package.Rules.clear();

    std::istringstream stream(content);
    std::string line;

    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line.empty())
            continue;

        size_t separator = line.find('=');

        if (separator == std::string::npos)
            continue;

        std::string ruleName = line.substr(0, separator);
        std::string value = line.substr(separator + 1);

        if (ruleName == "path")
        {
            package.Path = value;
        }
        else if (ruleName == "dependency")
        {
            package.Dependencies.push_back(value);
        }
        else
        {
            package.Rules.emplace_back(ruleName, value);
        }
    }

    return !package.Path.empty();
}