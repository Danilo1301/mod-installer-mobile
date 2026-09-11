#include "download.h"

#include "utils.h"

static int DownloadProgress(
        void* userData,
        curl_off_t total,
        curl_off_t downloaded,
        curl_off_t,
        curl_off_t)
{
    auto callback =
            static_cast<DownloadProgressCallback*>(userData);

    if (*callback)
    {
        (*callback)(
                static_cast<int64_t>(downloaded),
                static_cast<int64_t>(total)
        );
    }

    return 0;
}

extern bool DownloadFile(
        const std::string& url,
        const std::string& outputPath,
        DownloadProgressCallback progressCallback)
{
    CURL* curl = curl_easy_init();

    if (!curl)
    {
        LOGI("Failed to initialize curl");
        return false;
    }

    FILE* file = fopen(outputPath.c_str(), "wb");

    if (!file)
    {
        LOGI("Failed to open output file: %s", outputPath.c_str());
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    if (progressCallback)
    {
        curl_easy_setopt(
                curl,
                CURLOPT_XFERINFOFUNCTION,
                DownloadProgress);

        curl_easy_setopt(
                curl,
                CURLOPT_XFERINFODATA,
                &progressCallback);

        curl_easy_setopt(
                curl,
                CURLOPT_NOPROGRESS,
                0L);
    }

    CURLcode result = curl_easy_perform(curl);

    fclose(file);
    curl_easy_cleanup(curl);

    if (result != CURLE_OK)
    {
        LOGI("Download failed: %s", curl_easy_strerror(result));
        return false;
    }

    LOGI("Download completed: %s", outputPath.c_str());

    return true;
}

bool HttpGet(
        const std::string& url,
        std::string& response)
{
    response.clear();

    CURL* curl = curl_easy_init();

    if (!curl)
        return false;

    LOGI("HttpGet URL: [%s]", url.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "ModInstaller/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    CURLcode result = curl_easy_perform(curl);

    long responseCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

    LOGI("HTTP response code: %ld", responseCode);

    if (result != CURLE_OK)
    {
        LOGI("GET failed: %s", curl_easy_strerror(result));

        curl_easy_cleanup(curl);

        return false;
    }

    curl_easy_cleanup(curl);

    return true;
}