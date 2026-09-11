#pragma once

#include <jni.h>
#include <string>
#include <android/log.h>
#include <curl/curl.h>
#include <filesystem>
#include <sstream>
#include <thread>
#include <chrono>

#define LOG_TAG "ModInstaller"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#include "json/json.hpp"

using json = nlohmann::json;

inline JavaVM* _javaVM = nullptr;

inline jobject _consoleActivity = nullptr;

inline jmethodID _addConsoleLine = nullptr;
inline jmethodID _setConsoleLine = nullptr;
inline jmethodID _moveGameFiles = nullptr;
inline jmethodID _getFilesRecursive = nullptr;
inline jmethodID _showUserChoice = nullptr;
inline jmethodID _backupFiles = nullptr;
inline jmethodID _createFolder = nullptr;
inline jmethodID _clearCacheFolder = nullptr;