#include "AutoUpdater.h"

#include "Windows/Filesystem.h"

#include "CurlWrapper/CurlHandle.h"
#include "CurlWrapper/Writers/CurlFileWriter.h"
#include "CurlWrapper/Writers/CurlMemoryWriter.h"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <optional>

using namespace miniant::AutoUpdater;
using namespace miniant::CurlWrapper;
using namespace miniant::Windows::Filesystem;

using json = nlohmann::json;

std::optional<json> GetUpdaterRelease(CurlHandle& curl) {
    curl.Reset();
    curl.SetUrl("https://api.github.com/repos/miniant-git/REAL/releases/tags/updater-v2");
    curl.SetUserAgent("real_updater_v1");

    CurlMemoryWriter memoryWriter;
    long responseCode = memoryWriter.InitiateRequest(curl);
    if (responseCode != 200) {
        return {};
    }

    json response = json::parse(memoryWriter.GetBuffer());
    std::optional<Version> version = Version::Find(response["name"]);
    if (!version) {
        return {};
    }

    return response;
}

std::optional<json> GetUpdateRelease(const Version& currentVersion, CurlHandle& curl) {
    std::optional<json> updaterRelease = GetUpdaterRelease(curl);
    if (updaterRelease)
        return updaterRelease;

    curl.Reset();
    curl.SetUrl("https://api.github.com/repos/miniant-git/REAL/releases/latest");
    curl.SetUserAgent("real_updater_v1");

    CurlMemoryWriter memoryWriter;
    long responseCode = memoryWriter.InitiateRequest(curl);
    if (responseCode != 200) {
        spdlog::get("app_out")->info("Check for updates failed.");
        return {};
    }

    json response = json::parse(memoryWriter.GetBuffer());
    std::optional<Version> latestVersion = Version::Parse(response["tag_name"]);
    if (!latestVersion || currentVersion >= latestVersion.value()) {
        spdlog::get("app_out")->info("The application is up-to-date.");
        return {};
    }

    return response;
}

std::optional<const json*> FindExecutableAsset(const json& response) {
    for (const auto& asset : response["assets"]) {
        if (asset["name"] == "update")
            return { &asset };
    }

    return {};
}

WindowsString GetAppTempDirectory() {
    return GetTempDirectory() + TEXT("miniant\\REAL\\");
}

void DisplayReleaseNotes(const json& body) {
    static const std::string notesStartMarker("\r\n[//]: # (begin_release_notes)");
    static const std::string notesEndMarker("\r\n[//]: # (end_release_notes)");

    std::string bodyString(body);
    size_t notesStart = bodyString.find(notesStartMarker);
    size_t notesEnd = bodyString.rfind(notesEndMarker);

    if (notesStart == std::string::npos || notesEnd == std::string::npos)
        return;

    notesStart += notesStartMarker.length();
    spdlog::get("app_out")->info(bodyString.substr(notesStart, notesEnd - notesStart));
}

AutoUpdater::AutoUpdater(Version currentVersion):
    m_currentVersion(std::move(currentVersion)) {
    curl_global_init(CURL_GLOBAL_ALL);

    const WindowsString executableToDelete = GetExecutablePath() + TEXT("~DELETE");
    if (IsFile(executableToDelete)) {
        if (!DeleteFile(executableToDelete))
            spdlog::get("app_out")->info("Error: Could not delete temporary file: {}", std::filesystem::path(executableToDelete).string());
    }
}
AutoUpdater::~AutoUpdater() {
    curl_global_cleanup();
}

bool AutoUpdater::Update() const {
    CurlHandle curl;
    std::optional<json> response = GetUpdateRelease(m_currentVersion, curl);
    if (!response)
        return true;

    auto app_out = spdlog::get("app_out");

    app_out->info("A new update is available!");
    DisplayReleaseNotes(response.value()["body"]);
    std::optional<Version> latestVersion = Version::Parse(response.value()["tag_name"]);
    app_out->info("Do you want to update to {}? [y/N] : ", latestVersion.value().ToString());
    char line[5];
    std::cin.getline(line, 5);
    std::string prompt(line);
    std::transform(prompt.begin(), prompt.end(), prompt.begin(), std::tolower);
    if (prompt != "y" && prompt != "yes") {
        app_out->info("No: Keeping the current version.");
        return false;
    }

    std::optional<const json*> executableAsset = FindExecutableAsset(response.value());
    if (!executableAsset) {
        app_out->info("Error: misconfigured update assets.");
        return false;
    }

    WindowsString tempDirectory = GetAppTempDirectory();
    if (!CreateDirectory(tempDirectory)) {
        app_out->info("Error: Could not create temporary app directory: {}", std::filesystem::path(tempDirectory).string());
        return false;
    }

    std::filesystem::path tempExecutable(tempDirectory + TEXT("update"));
    const char* url = static_cast<std::string>((*executableAsset.value())["browser_download_url"]).c_str();
    curl.Reset();
    curl.SetUrl(url);
    curl.FollowRedirects(true);

    CurlFileWriter fileWriter(tempExecutable);
    app_out->info("Downloading update...");
    fileWriter.InitiateRequest(curl);
    fileWriter.Close();

    WindowsString executable = GetExecutablePath();
    std::optional<WindowsString> executableDirectory = GetParentDirectory(executable);
    WindowsString renameCommand = GetRenameCommand(executable, TEXT("REAL.exe~DELETE"));
    WindowsString moveCommand = GetMoveCommand(tempExecutable, executable);
    if (!ExecuteCommand(
        renameCommand + TEXT(" & ") + moveCommand,
        !CanWriteTo(executable) || !executableDirectory || !CanWriteTo(executableDirectory.value()))) {
        app_out->info("A filesystem error was encountered during the update procedure.");
        return false;
    }

    app_out->info("Updated successfully! Restart the application to apply changes.");
    return true;
}
