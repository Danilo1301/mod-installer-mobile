#include "zip.h"

#include <android/log.h>
#include <filesystem>
#include <string>

#include "mz.h"
#include "mz_strm.h"
#include "mz_zip.h"
#include "mz_zip_rw.h"

std::string SanitizeFilename(const std::string& filename)
{
    std::string result = filename;

    for (char& character : result)
    {
        if (static_cast<unsigned char>(character) >= 128)
            character = '_';
    }

    return result;
}

bool ExtractZip(const std::string& zipPath, const std::string& outputDirectory)
{
    namespace fs = std::filesystem;

    void* reader = mz_zip_reader_create();

    if (!reader)
    {
        LOGI("Failed to create ZIP reader");
        return false;
    }

    int32_t result = mz_zip_reader_open_file(
        reader,
        zipPath.c_str()
    );

    if (result != MZ_OK)
    {
        LOGI("Failed to open ZIP: %s", zipPath.c_str());

        mz_zip_reader_delete(&reader);

        return false;
    }

    std::error_code error;

    fs::create_directories(
        fs::path(outputDirectory),
        error
    );

    if (error)
    {
        LOGI(
            "Failed to create extraction directory: %s",
            error.message().c_str()
        );

        mz_zip_reader_close(reader);
        mz_zip_reader_delete(&reader);

        return false;
    }

    result = mz_zip_reader_goto_first_entry(reader);

    if (result != MZ_OK && result != MZ_END_OF_LIST)
    {
        LOGI("Failed to read first ZIP entry: %d", result);

        mz_zip_reader_close(reader);
        mz_zip_reader_delete(&reader);

        return false;
    }

    while (result == MZ_OK)
    {
        mz_zip_file* fileInfo = nullptr;

        result = mz_zip_reader_entry_get_info(
            reader,
            &fileInfo
        );

        if (result != MZ_OK || !fileInfo || !fileInfo->filename)
        {
            LOGI("Failed to get ZIP entry information");

            mz_zip_reader_close(reader);
            mz_zip_reader_delete(&reader);

            return false;
        }

        std::string filename = SanitizeFilename(fileInfo->filename);

        LOGI("ZIP entry: %s", filename.c_str());

        fs::path relativePath(filename);

        if (relativePath.is_absolute())
        {
            LOGI("Rejected absolute ZIP path: %s", filename.c_str());

            mz_zip_reader_close(reader);
            mz_zip_reader_delete(&reader);

            return false;
        }

        fs::path outputPath =
            fs::path(outputDirectory) / relativePath;

        fs::path normalizedOutput =
            fs::weakly_canonical(outputPath, error);

        if (error)
        {
            LOGI(
                "Failed to normalize ZIP path: %s",
                error.message().c_str()
            );

            mz_zip_reader_close(reader);
            mz_zip_reader_delete(&reader);

            return false;
        }

        fs::path normalizedRoot =
            fs::weakly_canonical(
                fs::path(outputDirectory),
                error
            );

        if (error)
        {
            LOGI(
                "Failed to normalize extraction directory: %s",
                error.message().c_str()
            );

            mz_zip_reader_close(reader);
            mz_zip_reader_delete(&reader);

            return false;
        }

        auto [rootEnd, outputEnd] = std::mismatch(
            normalizedRoot.begin(),
            normalizedRoot.end(),
            normalizedOutput.begin(),
            normalizedOutput.end()
        );

        if (rootEnd != normalizedRoot.end())
        {
            LOGI(
                "Rejected ZIP path outside extraction directory: %s",
                filename.c_str()
            );

            mz_zip_reader_close(reader);
            mz_zip_reader_delete(&reader);

            return false;
        }

        bool isDirectory =
            mz_zip_reader_entry_is_dir(reader) == MZ_OK;

        if (isDirectory)
        {
            fs::create_directories(
                outputPath,
                error
            );

            if (error)
            {
                LOGI(
                    "Failed to create directory: %s",
                    outputPath.string().c_str()
                );

                mz_zip_reader_close(reader);
                mz_zip_reader_delete(&reader);

                return false;
            }
        }
        else
        {
            fs::create_directories(
                outputPath.parent_path(),
                error
            );

            if (error)
            {
                LOGI(
                    "Failed to create parent directory: %s",
                    error.message().c_str()
                );

                mz_zip_reader_close(reader);
                mz_zip_reader_delete(&reader);

                return false;
            }

            result = mz_zip_reader_entry_save_file(
                reader,
                outputPath.string().c_str()
            );

            if (result != MZ_OK)
            {
                LOGI(
                    "Failed to extract file: %s",
                    filename.c_str()
                );

                mz_zip_reader_close(reader);
                mz_zip_reader_delete(&reader);

                return false;
            }
        }

        result = mz_zip_reader_goto_next_entry(reader);
    }

    if (result != MZ_END_OF_LIST)
    {
        LOGI(
            "ZIP extraction failed while reading entries: %d",
            result
        );

        mz_zip_reader_close(reader);
        mz_zip_reader_delete(&reader);

        return false;
    }

    mz_zip_reader_close(reader);
    mz_zip_reader_delete(&reader);

    LOGI(
        "ZIP extracted successfully: %s",
        outputDirectory.c_str()
    );

    return true;
}