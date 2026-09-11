#include "modData.h"

#include "download.h"

std::vector<ModDataEntry> FindModDatasInGithub(
        const std::string& githubUrl)
{
    std::vector<ModDataEntry> result;

    LOGI("FindModDatasInGithub: %s", githubUrl.c_str());

    const std::string prefix = "https://github.com/";

    if (githubUrl.rfind(prefix, 0) != 0)
    {
        LOGI("Invalid Github URL prefix");

        return result;
    }

    std::string repository =
            githubUrl.substr(prefix.length());

    LOGI("Repository: %s", repository.c_str());

    while (!repository.empty() &&
           repository.back() == '/')
    {
        repository.pop_back();
    }

    if (repository.empty())
    {
        LOGI("Repository is empty");

        return result;
    }

    std::string apiUrl =
            "https://api.github.com/repos/" +
            repository +
            "/releases";

    LOGI("API URL: %s", apiUrl.c_str());

    LOGI("apiUrl == hardcoded: %d",
         apiUrl ==
         "https://api.github.com/repos/Danilo1301/menu-szk-auto-install/releases");

    //

    LOGI("apiUrl length: %zu", apiUrl.length());

    std::string hardcoded =
            "https://api.github.com/repos/Danilo1301/menu-szk-auto-install/releases";

    LOGI("hardcoded length: %zu", hardcoded.length());

    for (size_t i = 0; i < std::max(apiUrl.length(), hardcoded.length()); i++)
    {
        int a = i < apiUrl.length()
                ? static_cast<unsigned char>(apiUrl[i])
                : -1;

        int b = i < hardcoded.length()
                ? static_cast<unsigned char>(hardcoded[i])
                : -1;

        if (a != b)
        {
            LOGI(
                    "DIFFERENCE at %zu: apiUrl=%d hardcoded=%d",
                    i,
                    a,
                    b
            );
        }
    }

    //

    std::string response;

    if (!HttpGet(apiUrl, response))
    {
        LOGI("HttpGet failed");

        return result;
    }

    LOGI(
            "HttpGet succeeded. Response size: %zu",
            response.size()
    );

    LOGI("Response: %s", response.c_str());

    json releases;

    try
    {
        releases = json::parse(response);
    }
    catch (const json::exception& exception)
    {
        LOGI(
                "JSON parse failed: %s",
                exception.what()
        );

        return result;
    }

    LOGI(
            "JSON parsed successfully. Is array: %d",
            releases.is_array()
    );

    if (!releases.is_array())
    {
        LOGI("Github response is not an array");

        return result;
    }

    LOGI(
            "Number of releases: %zu",
            releases.size()
    );

    for (const auto& release : releases)
    {
        LOGI("Processing release");

        if (!release.contains("tag_name"))
        {
            LOGI("Release has no tag_name");

            continue;
        }

        if (!release["tag_name"].is_string())
        {
            LOGI("tag_name is not a string");

            continue;
        }

        std::string version =
                release["tag_name"].get<std::string>();

        LOGI(
                "Release version: %s",
                version.c_str()
        );

        if (!release.contains("assets"))
        {
            LOGI("Release has no assets");

            continue;
        }

        if (!release["assets"].is_array())
        {
            LOGI("Release assets is not an array");

            continue;
        }

        LOGI(
                "Assets: %zu",
                release["assets"].size()
        );

        for (const auto& asset : release["assets"])
        {
            if (!asset.contains("browser_download_url"))
            {
                LOGI("Asset has no browser_download_url");

                continue;
            }

            if (!asset["browser_download_url"].is_string())
            {
                LOGI(
                        "browser_download_url is not a string"
                );

                continue;
            }

            ModDataEntry entry;

            entry.version = version;

            entry.downloadUrl =
                    asset["browser_download_url"]
                            .get<std::string>();

            LOGI(
                    "Found mod: version=%s url=%s",
                    entry.version.c_str(),
                    entry.downloadUrl.c_str()
            );

            result.push_back(entry);
        }
    }

    LOGI(
            "FindModDatasInGithub finished. Entries: %zu",
            result.size()
    );

    return result;
}

//exemplo de api
/*
[
  {
    "url": "https://api.github.com/repos/Danilo1301/police-mod-szk-auto-install/releases/386454753",
    "assets_url": "https://api.github.com/repos/Danilo1301/police-mod-szk-auto-install/releases/386454753/assets",
    "upload_url": "https://uploads.github.com/repos/Danilo1301/police-mod-szk-auto-install/releases/386454753/assets{?name,label}",
    "html_url": "https://github.com/Danilo1301/police-mod-szk-auto-install/releases/tag/0.2",
    "id": 386454753,
    "author": {
      "login": "Danilo1301",
      "id": 26677310,
      "node_id": "MDQ6VXNlcjI2Njc3MzEw",
      "avatar_url": "https://avatars.githubusercontent.com/u/26677310?v=4",
      "gravatar_id": "",
      "url": "https://api.github.com/users/Danilo1301",
      "html_url": "https://github.com/Danilo1301",
      "followers_url": "https://api.github.com/users/Danilo1301/followers",
      "following_url": "https://api.github.com/users/Danilo1301/following{/other_user}",
      "gists_url": "https://api.github.com/users/Danilo1301/gists{/gist_id}",
      "starred_url": "https://api.github.com/users/Danilo1301/starred{/owner}{/repo}",
      "subscriptions_url": "https://api.github.com/users/Danilo1301/subscriptions",
      "organizations_url": "https://api.github.com/users/Danilo1301/orgs",
      "repos_url": "https://api.github.com/users/Danilo1301/repos",
      "events_url": "https://api.github.com/users/Danilo1301/events{/privacy}",
      "received_events_url": "https://api.github.com/users/Danilo1301/received_events",
      "type": "User",
      "user_view_type": "public",
      "site_admin": false
    },
    "node_id": "RE_kwDOUUVT984XCNTh",
    "tag_name": "0.2",
    "target_commitish": "main",
    "name": "0.2",
    "draft": false,
    "immutable": false,
    "prerelease": false,
    "created_at": "2026-09-10T04:08:13Z",
    "updated_at": "2026-09-10T16:37:27Z",
    "published_at": "2026-09-10T16:37:27Z",
    "assets": [
      {
        "url": "https://api.github.com/repos/Danilo1301/police-mod-szk-auto-install/releases/assets/555390305",
        "id": 555390305,
        "node_id": "RA_kwDOUUVT984hGpVh",
        "name": "Mod.Policia.test.2.zip",
        "label": null,
        "uploader": {
          "login": "Danilo1301",
          "id": 26677310,
          "node_id": "MDQ6VXNlcjI2Njc3MzEw",
          "avatar_url": "https://avatars.githubusercontent.com/u/26677310?v=4",
          "gravatar_id": "",
          "url": "https://api.github.com/users/Danilo1301",
          "html_url": "https://github.com/Danilo1301",
          "followers_url": "https://api.github.com/users/Danilo1301/followers",
          "following_url": "https://api.github.com/users/Danilo1301/following{/other_user}",
          "gists_url": "https://api.github.com/users/Danilo1301/gists{/gist_id}",
          "starred_url": "https://api.github.com/users/Danilo1301/starred{/owner}{/repo}",
          "subscriptions_url": "https://api.github.com/users/Danilo1301/subscriptions",
          "organizations_url": "https://api.github.com/users/Danilo1301/orgs",
          "repos_url": "https://api.github.com/users/Danilo1301/repos",
          "events_url": "https://api.github.com/users/Danilo1301/events{/privacy}",
          "received_events_url": "https://api.github.com/users/Danilo1301/received_events",
          "type": "User",
          "user_view_type": "public",
          "site_admin": false
        },
        "content_type": "application/x-zip-compressed",
        "state": "uploaded",
        "size": 103922,
        "digest": "sha256:d522daf0f745e61de70cca081f41cc8be57e159517e2470db689e04d0d4dd226",
        "download_count": 0,
        "created_at": "2026-09-10T16:37:22Z",
        "updated_at": "2026-09-10T16:37:23Z",
        "browser_download_url": "https://github.com/Danilo1301/police-mod-szk-auto-install/releases/download/0.2/Mod.Policia.test.2.zip"
      }
    ],
    "tarball_url": "https://api.github.com/repos/Danilo1301/police-mod-szk-auto-install/tarball/0.2",
    "zipball_url": "https://api.github.com/repos/Danilo1301/police-mod-szk-auto-install/zipball/0.2",
    "body": ""
  },
  {
    "url": "https://api.github.com/repos/Danilo1301/police-mod-szk-auto-install/releases/385999567",
    "assets_url": "https://api.github.com/repos/Danilo1301/police-mod-szk-auto-install/releases/385999567/assets",
    "upload_url": "https://uploads.github.com/repos/Danilo1301/police-mod-szk-auto-install/releases/385999567/assets{?name,label}",
    "html_url": "https://github.com/Danilo1301/police-mod-szk-auto-install/releases/tag/0.1",
    "id": 385999567,
    "author": {
      "login": "Danilo1301",
      "id": 26677310,
      "node_id": "MDQ6VXNlcjI2Njc3MzEw",
      "avatar_url": "https://avatars.githubusercontent.com/u/26677310?v=4",
      "gravatar_id": "",
      "url": "https://api.github.com/users/Danilo1301",
      "html_url": "https://github.com/Danilo1301",
      "followers_url": "https://api.github.com/users/Danilo1301/followers",
      "following_url": "https://api.github.com/users/Danilo1301/following{/other_user}",
      "gists_url": "https://api.github.com/users/Danilo1301/gists{/gist_id}",
      "starred_url": "https://api.github.com/users/Danilo1301/starred{/owner}{/repo}",
      "subscriptions_url": "https://api.github.com/users/Danilo1301/subscriptions",
      "organizations_url": "https://api.github.com/users/Danilo1301/orgs",
      "repos_url": "https://api.github.com/users/Danilo1301/repos",
      "events_url": "https://api.github.com/users/Danilo1301/events{/privacy}",
      "received_events_url": "https://api.github.com/users/Danilo1301/received_events",
      "type": "User",
      "user_view_type": "public",
      "site_admin": false
    },
    "node_id": "RE_kwDOUUVT984XAeLP",
    "tag_name": "0.1",
    "target_commitish": "main",
    "name": "0.1",
    "draft": false,
    "immutable": false,
    "prerelease": false,
    "created_at": "2026-09-10T04:08:13Z",
    "updated_at": "2026-09-10T04:08:52Z",
    "published_at": "2026-09-10T04:08:52Z",
    "assets": [
      {
        "url": "https://api.github.com/repos/Danilo1301/police-mod-szk-auto-install/releases/assets/554199952",
        "id": 554199952,
        "node_id": "RA_kwDOUUVT984hCGuQ",
        "name": "Mod.Policia.test.1.zip",
        "label": null,
        "uploader": {
          "login": "Danilo1301",
          "id": 26677310,
          "node_id": "MDQ6VXNlcjI2Njc3MzEw",
          "avatar_url": "https://avatars.githubusercontent.com/u/26677310?v=4",
          "gravatar_id": "",
          "url": "https://api.github.com/users/Danilo1301",
          "html_url": "https://github.com/Danilo1301",
          "followers_url": "https://api.github.com/users/Danilo1301/followers",
          "following_url": "https://api.github.com/users/Danilo1301/following{/other_user}",
          "gists_url": "https://api.github.com/users/Danilo1301/gists{/gist_id}",
          "starred_url": "https://api.github.com/users/Danilo1301/starred{/owner}{/repo}",
          "subscriptions_url": "https://api.github.com/users/Danilo1301/subscriptions",
          "organizations_url": "https://api.github.com/users/Danilo1301/orgs",
          "repos_url": "https://api.github.com/users/Danilo1301/repos",
          "events_url": "https://api.github.com/users/Danilo1301/events{/privacy}",
          "received_events_url": "https://api.github.com/users/Danilo1301/received_events",
          "type": "User",
          "user_view_type": "public",
          "site_admin": false
        },
        "content_type": "application/x-zip-compressed",
        "state": "uploaded",
        "size": 103922,
        "digest": "sha256:d522daf0f745e61de70cca081f41cc8be57e159517e2470db689e04d0d4dd226",
        "download_count": 1,
        "created_at": "2026-09-10T04:08:25Z",
        "updated_at": "2026-09-10T04:08:27Z",
        "browser_download_url": "https://github.com/Danilo1301/police-mod-szk-auto-install/releases/download/0.1/Mod.Policia.test.1.zip"
      }
    ],
    "tarball_url": "https://api.github.com/repos/Danilo1301/police-mod-szk-auto-install/tarball/0.1",
    "zipball_url": "https://api.github.com/repos/Danilo1301/police-mod-szk-auto-install/zipball/0.1",
    "body": ""
  }
]
 */