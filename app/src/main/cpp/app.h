#pragma once

#include "pch.h"
#include "paths.h"
#include "modInstallData.h"

class App {
private:
    static std::vector<ModInstallData*> Installs;
    static std::function<void(int)> UserChoiceCallback;

public:
    static void Reset();
    static void InstallApp(const std::string& url);

    static void WaitForUserChoice(std::function<void(int)> callback);
    static void OnUserChoice(int value);
private:
    static void ProcessCurrentInstall();
    static void FindAndAddDependencies(ModInstallData* data);
    static void MoveAndDeleteStuff();

public:
    static void MoveGameFiles(const std::string& newGameFilesPath, const std::vector<std::string>& deleteFiles);

    static std::vector<std::string> GetFilesRecursive(const std::string& relativePath);

    static void ShowUserChoice(int type, const std::vector<std::string>& lines);

    static void BackupFiles(const std::vector<std::string>& files, std::function<void()> callback);

    static void AddCallback(int callbackId, std::function<void()> callback);
    static void OnCallbackReceived(int callbackId);

    static bool CreateFolder(const std::string& parentUri, const std::string& folderName);

    static void ClearCacheFolder();
};