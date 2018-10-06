#include "AutoUpdater.h"

#include "Windows/Filesystem.h"

#include "CurlWrapper/CurlHandle.h"
#include "CurlWrapper/Writers/CurlFileWriter.h"
#include "CurlWrapper/Writers/CurlMemoryWriter.h"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using namespace miniant::AutoUpdater;
using namespace miniant::CurlWrapper;
using namespace miniant::Windows::Filesystem;

using json = nlohmann::json;

std::optional<std::tuple<Version, json>> GetUpdaterRelease(CurlHandle& curl) {
    curl.Reset();
    curl.SetUrl("https://api.github.com/repos/miniant-git/REAL/releases/tags/updater-v2");
    curl.SetUserAgent("real_updater_v1");

    CurlMemoryWriter memoryWriter;
    long responseCode = memoryWriter.InitiateRequest(curl);
    if (responseCode != 200) {
        return std::nullopt;
    }

    json response = json::parse(memoryWriter.GetBuffer());
    std::optional<Version> version = Version::Find(response["name"]);
    if (!version) {
        return std::nullopt;
    }

    return { { std::move(*version), std::move(response) } };
}

std::optional<std::tuple<Version, json>> GetUpdateRelease(CurlHandle& curl) {
    std::optional<std::tuple<Version, json>> updaterRelease = GetUpdaterRelease(curl);
    if (updaterRelease) {
        return updaterRelease;
    }

    curl.Reset();
    curl.SetUrl("https://api.github.com/repos/miniant-git/REAL/releases/latest");
    curl.SetUserAgent("real_updater_v1");

    CurlMemoryWriter memoryWriter;
    long responseCode = memoryWriter.InitiateRequest(curl);
    if (responseCode != 200) {
        spdlog::get("app_out")->info("Check for updates failed.");
        return std::nullopt;
    }

    json response = json::parse(memoryWriter.GetBuffer());
    std::optional<Version> latestVersion = Version::Parse(response["tag_name"]);
    if (!latestVersion) {
        return std::nullopt;
    }

    return { { std::move(*latestVersion), std::move(response) } };
}

std::optional<std::string> FindUpdateAssetUrl(const json& response) {
    for (const auto& asset : response["assets"]) {
        if (asset["name"] == "update") {
            return { asset["browser_download_url"] };
        }
    }

    return std::nullopt;
}

WindowsString GetAppTempDirectory() {
    return GetTempDirectory() + TEXT("miniant\\REAL\\");
}

std::optional<std::string> GetReleaseNotes(const json& body) {
    static const std::string notesStartMarker("\r\n[//]: # (begin_release_notes)");
    static const std::string notesEndMarker("\r\n[//]: # (end_release_notes)");

    std::string bodyString(body);
    size_t notesStart = bodyString.find(notesStartMarker);
    size_t notesEnd = bodyString.rfind(notesEndMarker);

    if (notesStart == std::string::npos || notesEnd == std::string::npos) {
        return std::nullopt;
    }

    notesStart += notesStartMarker.length();
    return bodyString.substr(notesStart, notesEnd - notesStart);
}

AutoUpdater::AutoUpdater() {
    curl_global_init(CURL_GLOBAL_ALL);

    const WindowsString executableToDelete = GetExecutablePath() + TEXT("~DELETE");
    if (IsFile(executableToDelete)) {
        if (!DeleteFile(executableToDelete)) {
            spdlog::get("app_out")->info("Error: Could not delete temporary file: {}", std::filesystem::path(executableToDelete).string());
        }
    }
}
AutoUpdater::~AutoUpdater() {
    curl_global_cleanup();
}

std::optional<UpdateInfo> AutoUpdater::GetUpdateInfo() const {
    CurlHandle curl;
    std::optional<std::tuple<Version, json>> release = GetUpdateRelease(curl);
    if (!release) {
        return std::nullopt;
    }

    auto app_out = spdlog::get("app_out");

    auto[version, response] = std::move(*release);
    std::optional<std::string> downloadUrl = FindUpdateAssetUrl(response);
    if (!downloadUrl) {
        app_out->info("Error: misconfigured update assets.");
        return std::nullopt;
    }

    std::optional<std::string> releaseNotes = GetReleaseNotes(response["body"]);
    if (!releaseNotes) {
        app_out->info("Warning: missing release notes");
    }

    UpdateInfo info;
    info.version = std::move(version);
    info.downloadUrl = std::move(*downloadUrl);
    info.releaseNotes = std::move(*releaseNotes);
    return info;
}

bool AutoUpdater::ApplyUpdate(const UpdateInfo& info) const {
    auto app_out = spdlog::get("app_out");

    CurlHandle curl;
    curl.SetUrl(info.downloadUrl);
    curl.FollowRedirects(true);

    WindowsString tempDirectory = GetAppTempDirectory();
    if (!CreateDirectory(tempDirectory)) {
        app_out->info("Error: Could not create temporary app directory: {}", std::filesystem::path(tempDirectory).string());
        return false;
    }

    std::filesystem::path tempExecutable(tempDirectory + TEXT("update"));
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
