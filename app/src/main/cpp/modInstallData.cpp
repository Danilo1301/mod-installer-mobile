#include "modInstallData.h"

#include "console.h"
#include "package.h"
#include "utils.h"

int ModInstallData::CreateCustomMessage()
{
    lastConsoleMessageId = Console::AddConsoleLine("Custom line from mod install data");

    return lastConsoleMessageId;
}

void ModInstallData::SetCustomMessage(const std::string& text)
{
    Console::SetConsoleLine(lastConsoleMessageId, text);
}

void ModInstallData::ProcessPackage()
{
    std::string packagePath;

    LOGI("Searching package in: %s", extractPath.c_str());

    for (const auto& entry : std::filesystem::directory_iterator(extractPath))
    {
        LOGI(
                "Entry: %s | filename: %s | extension: %s | regular: %d",
                entry.path().string().c_str(),
                entry.path().filename().string().c_str(),
                entry.path().extension().string().c_str(),
                entry.is_regular_file()
        );

        if (entry.is_regular_file() &&
            entry.path().filename() == ".package")
        {
            packagePath = entry.path().string();

            LOGI("Found package: %s", packagePath.c_str());

            break;
        }
    }

    if (packagePath.empty())
    {
        LOGI("No .package file found in: %s", extractPath.c_str());
        return;
    }

    std::string content;

    if (!ReadFile(packagePath, content))
    {
        LOGI("Failed to read package: %s", packagePath.c_str());
        return;
    }

    if (!ParsePackage(content, *packageData))
    {
        LOGI("Failed to read package: %s", packagePath.c_str());

        return;
    }
}