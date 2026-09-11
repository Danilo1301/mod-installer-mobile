#pragma once

#include "pch.h"

using DownloadProgressCallback = std::function<void(int64_t downloaded, int64_t total)>;

bool DownloadFile(const std::string& url, const std::string& outputPath, DownloadProgressCallback progressCallback = nullptr);

bool HttpGet(const std::string& url, std::string& response);