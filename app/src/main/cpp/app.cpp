#include "app.h"

#include "utils.h"
#include "download.h"
#include "console.h"
#include "zip.h"
#include "modData.h"

static constexpr int USER_CHOICE_BACKUP_MENU = 0;

static int COLOR_GRAY = 0xFF9B9B9B;
static int COLOR_RED = 0xFFFF5151;

std::vector<ModInstallData*> App::Installs;

std::function<void(int)> App::UserChoiceCallback;

bool CanMoveToGameFolder = false;
std::vector<std::pair<std::string, std::string>> MoveRules;

static int _nextCallbackId = 0;
static std::unordered_map<int, std::function<void()>> _callbacks;

void App::Reset()
{
    CanMoveToGameFolder = false;
    MoveRules.clear();
}

void App::InstallApp(const std::string& url)
{
    std::string id = GenerateRandomId();

    auto data = new ModInstallData();
    data->url = url;
    data->zipPath = Paths::CachePath + "/mod_" + id + ".zip";
    data->extractPath = Paths::CachePath + "/mod_" + id + "_files/";

    Installs.insert(Installs.begin(), data);

    ProcessCurrentInstall();
}

void App::ProcessCurrentInstall()
{
    if(CanMoveToGameFolder)
    {
        MoveAndDeleteStuff();
    }

    if(Installs.empty()) return;

    auto data = Installs[0];

    if(data->step == ModInstallStep::None)
    {

    }

    if(data->step == ModInstallStep::None)
    {
        data->step = ModInstallStep::Downloading;

        //Console::AddConsoleLine("I have started downloading...");
        Console::AddConsoleLine("Downloading " + data->url + "...", COLOR_GRAY);

        data->CreateCustomMessage();
        data->SetCustomMessage("Wait...");

        bool result = DownloadFile(
                data->url,
                data->zipPath,
                [data, totalSize = int64_t(0)](
                        int64_t downloaded,
                        int64_t total) mutable
                {
                    if (total > totalSize)
                        totalSize = total;

                    if (totalSize <= 0)
                        return;

                    int percent =
                            static_cast<int>(
                                    downloaded * 100 / totalSize
                            );

                    if (percent > 100)
                        percent = 100;

                    data->SetCustomMessage(
                            "Downloading " +
                            std::to_string(percent) +
                            "%"
                    );
                }
        );

        if (!result)
        {
            Console::AddConsoleLine("Ocorreu um erro ao baixar");
            return;
        }

        Console::AddConsoleLine("Download completed!");
        Console::AddConsoleLine("Extracting...");

        data->step = ModInstallStep::Extracting;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        if (!ExtractZip(data->zipPath, data->extractPath))
        {
            Console::AddConsoleLine("Failed to extract mod");
            return;
        }

        Console::AddConsoleLine("Extracted successfully!");

        data->step = ModInstallStep::ProcessingPackage;

        data->ProcessPackage();

        if(data->packageData->Dependencies.empty())
        {
            Console::AddConsoleLine("Found no dependecies");
        } else {
            for (const std::string& dependency : data->packageData->Dependencies)
            {
                Console::AddConsoleLine("Dependency found: " + dependency);
            }

            //Console::AddConsoleLine("Deseja instalar?");

            WaitForUserChoice([data](int value)
                {
                    if (value == 1)
                    {
                        FindAndAddDependencies(data);
                    }

                    ProcessCurrentInstall();
                }
            );

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            OnUserChoice(1);
        }
    }

    if(data->step == ModInstallStep::ProcessingPackage)
    {
        Console::AddConsoleLine("Copying files to temporary folders...");

        std::filesystem::path sourcePath =
                std::filesystem::path(data->extractPath) /
                data->packageData->Path;

        std::filesystem::path newGameFilesPath =
                std::filesystem::path(Paths::CachePath) /
                "newGameFiles";

        std::filesystem::path manualFilesPath =
                newGameFilesPath /
                "manual_installation_files";

        std::error_code error;

        std::filesystem::create_directories(
                newGameFilesPath,
                error
        );

        if (error)
        {
            Console::AddConsoleLine(
                    "Failed to create newGameFiles: " +
                    error.message()
            );

            return;
        }

        if (std::filesystem::exists(newGameFilesPath))
        {
            std::filesystem::remove_all(
                    newGameFilesPath,
                    error
            );

            if (error)
            {
                Console::AddConsoleLine(
                        "Failed to clear newGameFiles: " +
                        error.message()
                );

                return;
            }
        }

        std::filesystem::rename(
                sourcePath,
                newGameFilesPath,
                error
        );

        if (error) {
            Console::AddConsoleLine(
                    "Failed to move game files: " +
                    error.message()
            );

            return;
        }

        for (const auto& rule : data->packageData->Rules)
        {
            if (rule.first != "manual_install_path")
                continue;

            std::filesystem::path manualSourcePath =
                    std::filesystem::path(data->extractPath) /
                    rule.second;

            std::filesystem::create_directories(
                    manualFilesPath,
                    error
            );

            if (error)
            {
                Console::AddConsoleLine(
                        "Failed to create manualFiles: " +
                        error.message()
                );

                return;
            }

            std::filesystem::copy(
                    manualSourcePath,
                    manualFilesPath,
                    std::filesystem::copy_options::recursive |
                    std::filesystem::copy_options::overwrite_existing,
                    error
            );

            if (error)
            {
                Console::AddConsoleLine(
                        "Failed to copy manual files: " +
                        error.message()
                );

                return;
            }

            break;
        }

        MoveRules.insert(
                MoveRules.end(),
                data->packageData->Rules.begin(),
                data->packageData->Rules.end()
        );

        delete data;
        Installs.erase(Installs.begin());

        if(Installs.empty())
        {
            CanMoveToGameFolder = true;
        }

        ProcessCurrentInstall();

        return;
    }
}

void App::WaitForUserChoice(std::function<void(int)> callback)
{
    UserChoiceCallback = std::move(callback);
}

void App::OnUserChoice(int value)
{
    if (!UserChoiceCallback)
        return;

    auto callback = std::move(UserChoiceCallback);

    callback(value);
}

void App::FindAndAddDependencies(ModInstallData* data)
{
    for (const std::string& dependency : data->packageData->Dependencies)
    {
        //Console::AddConsoleLine("Dependency: " + dependency);

        std::vector<ModDataEntry> entries = FindModDatasInGithub(dependency);

        LOGI(
                "Dependency URL: %s | Entries: %zu",
                dependency.c_str(),
                entries.size()
        );

        if (entries.empty())
        {
            Console::AddConsoleLine("No releases found: " + dependency, COLOR_RED);

            continue;
        }

        for (const ModDataEntry& entry : entries)
        {
            LOGI(
                    "Version: %s | Download URL: %s",
                    entry.version.c_str(),
                    entry.downloadUrl.c_str()
            );
        }

        ModDataEntry entry = entries[0];

        if (entry.downloadUrl.empty())
        {
            Console::AddConsoleLine("URL de download vazia", COLOR_RED);
            continue;
        }

        InstallApp(entry.downloadUrl);
    }
}

void App::MoveAndDeleteStuff()
{
    std::filesystem::path newGameFilesPath =
            std::filesystem::path(Paths::CachePath) /
            "newGameFiles";

    std::filesystem::path manualFilesPath =
            newGameFilesPath /
            "manual_installation_files";

    if (!std::filesystem::exists(newGameFilesPath))
    {
        Console::AddConsoleLine(
                "newGameFiles folder does not exist"
        );

        return;
    }

    std::vector<std::string> deleteFiles;
    std::vector<std::string> deleteFilesDisplay;

    for (const auto& rule : MoveRules)
    {
        if (rule.first != "delete_old_files_from_folder")
            continue;

        std::vector<std::string> files =
                GetFilesRecursive(rule.second);

        for (const std::string& file : files)
        {
            deleteFiles.push_back(file);
        }
    }

    std::string gameFolderPath = Paths::GameFolderPath;

    size_t treeIndex =
            gameFolderPath.find("/tree/");

    if (treeIndex == std::string::npos)
        return;

    gameFolderPath =
            gameFolderPath.substr(
                    treeIndex + 6
            );

    for (const std::string& filePath : deleteFiles)
    {
        size_t documentIndex =
                filePath.find("/document/");

        if (documentIndex == std::string::npos)
            continue;

        std::string gameFilePath =
                filePath.substr(
                        documentIndex + 10
                );

        if (gameFilePath.rfind(gameFolderPath, 0) != 0)
            continue;

        gameFilePath =
                gameFilePath.substr(
                        gameFolderPath.length()
                );

        if (gameFilePath.rfind("%2F", 0) == 0)
        {
            gameFilePath.erase(0, 3);
        }

        size_t pos = 0;

        while ((pos = gameFilePath.find("%2F", pos)) != std::string::npos)
        {
            gameFilePath.replace(pos, 3, "/");
            pos++;
        }

        deleteFilesDisplay.push_back(
                gameFilePath
        );
    }

    std::vector<std::string> backupFiles = deleteFiles;

    for (const auto& entry :
            std::filesystem::recursive_directory_iterator(newGameFilesPath))
    {
        if (!entry.is_regular_file())
            continue;

        std::filesystem::path relativePath =
                std::filesystem::relative(
                        entry.path(),
                        newGameFilesPath
                );

        std::string relativeFile =
                relativePath.generic_string();

        if (relativeFile.rfind(
                "manual_installation_files/",
                0
        ) == 0)
        {
            continue;
        }

        backupFiles.push_back(relativeFile);
        deleteFilesDisplay.push_back(relativeFile);
    }

    WaitForUserChoice(
            [newGameFilesPath, manualFilesPath, deleteFiles, backupFiles](int value)
            {
                auto func = [newGameFilesPath, manualFilesPath, deleteFiles]()
                {
                    std::this_thread::sleep_for(
                            std::chrono::milliseconds(500)
                    );

                    MoveGameFiles(
                            newGameFilesPath.string(),
                            deleteFiles
                    );

                    AddCallback(50028, [manualFilesPath]() {
                        Console::AddConsoleLine(
                                "Mod instalado!",
                                0xFF00FF00
                        );

                        if (std::filesystem::exists(manualFilesPath))
                        {
                            Console::AddConsoleLine(
                                    "Existe uma instalacao manual pendente.",
                                    0xFFFFAA00
                            );

                            Console::AddConsoleLine(
                                    "Va ate a pasta do jogo e leia as instrucoes em manual_installation_files.",
                                    0xFFFFAA00
                            );
                        }

                        ClearCacheFolder();
                    });
                };

                if (value == 1)
                {
                    BackupFiles(backupFiles, [func]()
                    {
                        func();
                    });

                    return;
                }

                func();
            }
    );

    Console::AddConsoleLine(
            "Please wait. Moving to game folder... This can take a while..."
    );

    ShowUserChoice(
            USER_CHOICE_BACKUP_MENU,
            deleteFilesDisplay
    );
}

void App::MoveGameFiles(const std::string& newGameFilesPath, const std::vector<std::string>& deleteFiles)
{
    if (!_consoleActivity || !_moveGameFiles)
        return;

    JNIEnv* env = nullptr;

    if (_javaVM->GetEnv(
            reinterpret_cast<void**>(&env),
            JNI_VERSION_1_6) != JNI_OK)
    {
        return;
    }

    jobjectArray files =
            env->NewObjectArray(
                    static_cast<jsize>(deleteFiles.size()),
                    env->FindClass("java/lang/String"),
                    nullptr
            );

    for (jsize i = 0; i < static_cast<jsize>(deleteFiles.size()); i++)
    {
        jstring file =
                env->NewStringUTF(
                        deleteFiles[i].c_str()
                );

        env->SetObjectArrayElement(
                files,
                i,
                file
        );

        env->DeleteLocalRef(file);
    }

    jstring path =
            env->NewStringUTF(
                    newGameFilesPath.c_str()
            );

    env->CallVoidMethod(
            _consoleActivity,
            _moveGameFiles,
            path,
            files
    );

    env->DeleteLocalRef(path);
    env->DeleteLocalRef(files);
}

std::vector<std::string> App::GetFilesRecursive(const std::string& relativePath)
{
    std::vector<std::string> files;

    if (!_consoleActivity || !_getFilesRecursive)
        return files;

    JNIEnv* env = nullptr;

    if (_javaVM->GetEnv(
            reinterpret_cast<void**>(&env),
            JNI_VERSION_1_6) != JNI_OK)
    {
        return files;
    }

    jstring path =
            env->NewStringUTF(
                    relativePath.c_str()
            );

    jobjectArray result =
            static_cast<jobjectArray>(
                    env->CallObjectMethod(
                            _consoleActivity,
                            _getFilesRecursive,
                            path
                    )
            );

    if (result)
    {
        jsize count =
                env->GetArrayLength(result);

        for (jsize i = 0; i < count; i++)
        {
            jstring file =
                    static_cast<jstring>(
                            env->GetObjectArrayElement(
                                    result,
                                    i
                            )
                    );

            if (!file)
                continue;

            const char* chars =
                    env->GetStringUTFChars(
                            file,
                            nullptr
                    );

            if (chars)
            {
                files.emplace_back(chars);

                env->ReleaseStringUTFChars(
                        file,
                        chars
                );
            }

            env->DeleteLocalRef(file);
        }

        env->DeleteLocalRef(result);
    }

    env->DeleteLocalRef(path);

    return files;
}

void App::ShowUserChoice(int type, const std::vector<std::string>& lines)
{
    if (!_consoleActivity || !_showUserChoice)
        return;

    JNIEnv* env = nullptr;

    if (_javaVM->GetEnv(
            reinterpret_cast<void**>(&env),
            JNI_VERSION_1_6) != JNI_OK)
    {
        return;
    }

    jclass stringClass =
            env->FindClass("java/lang/String");

    jobjectArray javaLines =
            env->NewObjectArray(
                    static_cast<jsize>(lines.size()),
                    stringClass,
                    nullptr
            );

    for (jsize i = 0; i < static_cast<jsize>(lines.size()); i++)
    {
        jstring line =
                env->NewStringUTF(
                        lines[i].c_str()
                );

        env->SetObjectArrayElement(
                javaLines,
                i,
                line
        );

        env->DeleteLocalRef(line);
    }

    env->CallVoidMethod(
            _consoleActivity,
            _showUserChoice,
            static_cast<jint>(type),
            javaLines
    );

    env->DeleteLocalRef(javaLines);
    env->DeleteLocalRef(stringClass);
}

void App::BackupFiles(const std::vector<std::string>& files, std::function<void()> callback)
{
    int callbackId = _nextCallbackId++;

    _callbacks[callbackId] =
            std::move(callback);

    if (!_consoleActivity || !_backupFiles)
        return;

    JNIEnv* env = nullptr;

    if (_javaVM->GetEnv(
            reinterpret_cast<void**>(&env),
            JNI_VERSION_1_6) != JNI_OK)
    {
        return;
    }

    jclass stringClass =
            env->FindClass("java/lang/String");

    jobjectArray javaFiles =
            env->NewObjectArray(
                    static_cast<jsize>(files.size()),
                    stringClass,
                    nullptr
            );

    for (jsize i = 0;
         i < static_cast<jsize>(files.size());
         i++)
    {
        jstring file =
                env->NewStringUTF(
                        files[i].c_str()
                );

        env->SetObjectArrayElement(
                javaFiles,
                i,
                file
        );

        env->DeleteLocalRef(file);
    }

    env->CallVoidMethod(
            _consoleActivity,
            _backupFiles,
            javaFiles,
            static_cast<jint>(callbackId)
    );

    env->DeleteLocalRef(javaFiles);
    env->DeleteLocalRef(stringClass);
}

void App::AddCallback(int callbackId, std::function<void()> callback)
{
    _callbacks[callbackId] =
            std::move(callback);
}


void App::OnCallbackReceived(int callbackId)
{
    auto it = _callbacks.find(callbackId);

    if (it == _callbacks.end())
        return;

    auto callback =
            std::move(it->second);

    _callbacks.erase(it);

    callback();
}

bool App::CreateFolder(const std::string& parentUri, const std::string& folderName)
{
    if (!_consoleActivity || !_createFolder)
        return false;

    JNIEnv* env = nullptr;
    bool attached = false;

    if (_javaVM->GetEnv(
            reinterpret_cast<void**>(&env),
            JNI_VERSION_1_6) != JNI_OK)
    {
        if (_javaVM->AttachCurrentThread(
                &env,
                nullptr) != JNI_OK)
        {
            return false;
        }

        attached = true;
    }

    jstring javaParentUri =
            env->NewStringUTF(
                    parentUri.c_str()
            );

    jstring javaFolderName =
            env->NewStringUTF(
                    folderName.c_str()
            );

    jboolean result =
            env->CallBooleanMethod(
                    _consoleActivity,
                    _createFolder,
                    javaParentUri,
                    javaFolderName
            );

    if (env->ExceptionCheck())
    {
        env->ExceptionDescribe();
        env->ExceptionClear();

        result = JNI_FALSE;
    }

    env->DeleteLocalRef(javaParentUri);
    env->DeleteLocalRef(javaFolderName);

    if (attached)
        _javaVM->DetachCurrentThread();

    return result == JNI_TRUE;
}

void App::ClearCacheFolder()
{
    if (!_consoleActivity || !_clearCacheFolder)
        return;

    JNIEnv* env = nullptr;

    if (_javaVM->GetEnv(
            reinterpret_cast<void**>(&env),
            JNI_VERSION_1_6) != JNI_OK)
    {
        return;
    }

    env->CallVoidMethod(
            _consoleActivity,
            _clearCacheFolder
    );
}