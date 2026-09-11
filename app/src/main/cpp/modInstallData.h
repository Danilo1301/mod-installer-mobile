#pragma once

#include "pch.h"
#include "package.h"

enum ModInstallStep
{
    None = 0,
    Downloading,
    FinishedDownload,
    Extracting,
    ProcessingPackage,
};

class ModInstallData {
private:
    int lastConsoleMessageId;

public:
    ModInstallStep step = ModInstallStep::None;
    std::string url;
    std::string zipPath;
    std::string extractPath;
    PackageData* packageData = new PackageData();

    int CreateCustomMessage();
    void SetCustomMessage(const std::string& text);

    void ProcessPackage();
};