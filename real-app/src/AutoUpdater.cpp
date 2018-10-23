#include "AutoUpdater.h"

#include "Windows/Filesystem.h"

#include "CurlWrapper/Writers/CurlFileWriter.h"
#include "CurlWrapper/Writers/CurlMemoryWriter.h"

#include <nlohmann/json.hpp>

using namespace miniant::AutoUpdater;
using namespace miniant::CurlWrapper;
using namespace miniant::Windows::Filesystem;

using json = nlohmann::json;

tl::expected<std::tuple<Version, json>, AutoUpdaterError> GetUpdaterRelease(CurlHandle& curl) {
    curl.Reset();
    curl.SetUrl("https://api.github.com/repos/miniant-git/REAL/releases/tags/updater-v2");
    curl.SetUserAgent("real_updater_v1");

    CurlMemoryWriter memoryWriter;
    tl::expected responseCode = memoryWriter.InitiateRequest(curl);
    if (!responseCode) {
        return tl::make_unexpected(AutoUpdaterError(responseCode.error()));
    }

    if (*responseCode != 200) {
        return tl::make_unexpected(AutoUpdaterError("GET request failed."));
    }

    json response = json::parse(memoryWriter.GetBuffer());
    tl::expected version = Version::Find(response["name"]);
    if (!version) {
        return tl::make_unexpected(AutoUpdaterError(version.error()));
    }

    return { { std::move(*version), std::move(response) } };
}

tl::expected<std::tuple<Version, json>, AutoUpdaterError> GetUpdateRelease(CurlHandle& curl) {
    tl::expected updaterRelease = GetUpdaterRelease(curl);
    if (updaterRelease) {
        return updaterRelease;
    }

    curl.Reset();
    curl.SetUrl("https://api.github.com/repos/miniant-git/REAL/releases/latest");
    curl.SetUserAgent("real_updater_v1");

    CurlMemoryWriter memoryWriter;
    tl::expected responseCode = memoryWriter.InitiateRequest(curl);
    if (!responseCode) {
        return tl::make_unexpected(AutoUpdaterError(responseCode.error()));
    }

    if (*responseCode != 200) {
        return tl::make_unexpected(AutoUpdaterError("GET request failed."));
    }

    json response = json::parse(memoryWriter.GetBuffer());
    tl::expected latestVersion = Version::Parse(response["tag_name"]);
    if (!latestVersion) {
        return tl::make_unexpected(AutoUpdaterError(latestVersion.error()));
    }

    return { { std::move(*latestVersion), std::move(response) } };
}

tl::expected<std::string, AutoUpdaterError> FindUpdateAssetUrl(const json& response) {
    for (const auto& asset : response["assets"]) {
        if (asset["name"] == "update") {
            return { asset["browser_download_url"] };
        }
    }

    return tl::make_unexpected(AutoUpdaterError("Could not find update asset URL."));
}

WindowsString GetAppTempDirectory() {
    return GetTempDirectory() + TEXT("miniant\\REAL\\");
}

tl::expected<std::string, AutoUpdaterError> GetReleaseNotes(const json& body) {
    static const std::string notesStartMarker("\r\n[//]: # (begin_release_notes)");
    static const std::string notesEndMarker("\r\n[//]: # (end_release_notes)");

    std::string bodyString(body);
    size_t notesStart = bodyString.find(notesStartMarker);
    size_t notesEnd = bodyString.rfind(notesEndMarker);

    if (notesStart == std::string::npos || notesEnd == std::string::npos) {
        return tl::make_unexpected(AutoUpdaterError("Could not find release notes."));
    }

    notesStart += notesStartMarker.length();
    return bodyString.substr(notesStart, notesEnd - notesStart);
}

AutoUpdater::AutoUpdater() {
    curl_global_init(CURL_GLOBAL_ALL);
}
AutoUpdater::~AutoUpdater() {
    curl_global_cleanup();
}

tl::expected<bool, AutoUpdaterError> AutoUpdater::CleanupPreviousSetup() {
    const WindowsString executableToDelete = GetExecutablePath() + TEXT("~DELETE");
    if (IsFile(executableToDelete)) {
        if (!DeleteFile(executableToDelete)) {
            return tl::make_unexpected(AutoUpdaterError("Could not delete temporary file."));
        }

        return true;
    }

    return false;
}

tl::expected<UpdateInfo, AutoUpdaterError> AutoUpdater::GetUpdateInfo() const {
    tl::expected curl = CurlHandle::Create();
    if (!curl) {
        return tl::make_unexpected(AutoUpdaterError(curl.error()));
    }

    tl::expected release = GetUpdateRelease(*curl);
    if (!release) {
        return tl::make_unexpected(AutoUpdaterError(release.error()));
    }

    auto[version, response] = std::move(*release);
    tl::expected downloadUrl = FindUpdateAssetUrl(response);
    if (!downloadUrl) {
        return tl::make_unexpected(AutoUpdaterError(downloadUrl.error()));
    }

    UpdateInfo info;
    info.version = std::move(version);
    info.downloadUrl = std::move(*downloadUrl);
    if (tl::expected releaseNotes = GetReleaseNotes(response["body"]); releaseNotes) {
        info.releaseNotes = *releaseNotes;
    }

    return info;
}

tl::expected<void, AutoUpdaterError> AutoUpdater::ApplyUpdate(const UpdateInfo& info) const {
    tl::expected curl = CurlHandle::Create();
    if (!curl) {
        return tl::make_unexpected(AutoUpdaterError(curl.error()));
    }

    curl->SetUrl(info.downloadUrl);
    curl->FollowRedirects(true);

    WindowsString tempDirectory = GetAppTempDirectory();
    if (!CreateDirectory(tempDirectory)) {
        return tl::make_unexpected(AutoUpdaterError("Could not create temporary app directory."));
    }

    std::filesystem::path updateFile(tempDirectory + TEXT("update.zip"));
    CurlFileWriter fileWriter(updateFile);
    fileWriter.InitiateRequest(*curl);
    fileWriter.Close();

    WindowsString executable = GetExecutablePath();
    std::optional<WindowsString> executableDirectory = GetParentDirectory(executable);
    if (!executableDirectory) {
        return tl::make_unexpected(AutoUpdaterError("Could not get the application's executable file directory."));
    }

    WindowsString renameCommand = GetRenameCommand(executable, TEXT("REAL.exe~DELETE"));
    WindowsString extractCommand = GetExtractZipCommand(updateFile, *executableDirectory);
    WindowsString deleteCommand = GetDeleteCommand(updateFile);
    if (!ExecuteCommand(
        renameCommand + TEXT(" && ") + extractCommand + TEXT(" && ") + deleteCommand,
        !CanWriteTo(executable) || !CanWriteTo(*executableDirectory))) {
        return tl::make_unexpected(AutoUpdaterError("A filesystem error was encountered during the update procedure."));
    }

    return {};
}
