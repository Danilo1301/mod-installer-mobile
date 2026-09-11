#include "mod_database.h"

#include <sstream>

std::vector<ModData> ParseModDatabase(const std::string& content)
{
    std::vector<ModData> mods;

    std::istringstream stream(content);
    std::string line;

    ModData currentMod{};
    bool hasValues = false;

    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line == "#")
        {
            if (hasValues)
            {
                mods.push_back(currentMod);

                currentMod = {};
                hasValues = false;
            }

            continue;
        }

        if (line.empty())
            continue;

        size_t separator = line.find('=');

        if (separator == std::string::npos)
            continue;

        std::string key = line.substr(0, separator);
        std::string value = line.substr(separator + 1);

        if (!key.empty() && key.back() == ' ')
            key.pop_back();

        if (!value.empty() && value.front() == ' ')
            value.erase(0, 1);

        if (key == "name")
        {
            currentMod.Name = value;
            hasValues = true;
        }
        else if (key == "author")
        {
            currentMod.Author = value;
            hasValues = true;
        }
        else if (key == "version")
        {
            currentMod.Version = value;
            hasValues = true;
        }
        else if (key == "description")
        {
            currentMod.Description = value;
            hasValues = true;
        }
        else if (key == "github")
        {
            currentMod.Url = value;
            currentMod.IsRepository = true;
            hasValues = true;
        }
        else if (key == "zip")
        {
            currentMod.Url = value;
            currentMod.IsRepository = false;
            hasValues = true;
        }
    }

    if (hasValues)
        mods.push_back(currentMod);

    return mods;
}