#include "AutoUpdater.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <optional>
#include <vector>

using namespace miniant::WasapiLatency;

using json = nlohmann::json;
using Buffer = std::vector<char>;

size_t PopulateBuffer(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto buffer = reinterpret_cast<Buffer*>(userdata);
    buffer->insert(buffer->end(), ptr, ptr+nmemb);
    return nmemb;
}

std::optional<const json*> FindExecutableAsset(const json& response) {
    for (const auto& asset : response["assets"]) {
        if (asset["name"] == "update")
            return { &asset };
    }

    return {};
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
    std::cout << bodyString.substr(notesStart, notesEnd - notesStart) << std::endl;
}

AutoUpdater::AutoUpdater(Version currentVersion, std::filesystem::path executable):
    m_currentVersion(std::move(currentVersion)),
    m_executable(std::move(executable)) {
    curl_global_init(CURL_GLOBAL_ALL);

    const std::filesystem::path executableToDelete = m_executable.string() + "~DELETE";
    if (std::filesystem::exists(executableToDelete)) {
        try {
            std::filesystem::remove(executableToDelete);
        } catch (std::filesystem::filesystem_error& error) {
            std::cout << "Error: " << error.what() << std::endl;
        }
    }
}
AutoUpdater::~AutoUpdater() {
    curl_global_cleanup();
}

bool AutoUpdater::Update() const {
    if (this->UpdateUpdater())
        return true;

    CURL* curl = curl_easy_init();
    if (curl == nullptr)
        return false;

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.github.com/repos/miniant-git/REAL/releases/latest");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "real_updater_v1");

    Buffer buffer;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, PopulateBuffer);

    CURLcode status = curl_easy_perform(curl);
    if (status != CURLE_OK) {
        std::cout << "Could not access the update server." << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    long responseCode;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
    if (responseCode != 200) {
        std::cout << "Check for updates failed." << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    json response = json::parse(buffer);
    std::optional<Version> latestVersion = Version::Parse(response["tag_name"]);
    if (!latestVersion || m_currentVersion >= latestVersion.value()) {
        std::cout << "The application is up-to-date." << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }
    std::cout << "A new update is available!" << std::endl;
    DisplayReleaseNotes(response["body"]);
    std::cout << "Do you want to update to " << latestVersion.value().ToString() << "? [y/N]: ";
    char line[5];
    std::cin.getline(line, 5);
    std::string prompt(line);
    std::transform(prompt.begin(), prompt.end(), prompt.begin(), std::tolower);
    if (prompt != "y" && prompt != "yes") {
        std::cout << "No: Keeping the current version." << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    std::optional<const json*> executable = FindExecutableAsset(response);
    if (!executable) {
        std::cout << "Error: misconfigured update assets." << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    std::filesystem::path tempExecutable(m_executable.string() + "~TEMP");

    const char* url = static_cast<std::string>((*executable.value())["browser_download_url"]).c_str();
    FILE* executableFile;
    fopen_s(&executableFile, tempExecutable.string().c_str(), "wb");
    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, executableFile);

    std::cout << "Downloading update..." << std::endl;
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    fclose(executableFile);

    try {
        std::filesystem::rename(m_executable, m_executable.string() + "~DELETE");
        std::filesystem::rename(tempExecutable, m_executable);
    } catch (std::filesystem::filesystem_error& error) {
        std::cout << "Error: " << error.what() << std::endl;
        return false;
    }

    std::cout << "Updated successfully! Restart the application to apply changes." << std::endl;
    return true;
}

bool AutoUpdater::UpdateUpdater() const {
    CURL* curl = curl_easy_init();
    if (curl == nullptr)
        return false;

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.github.com/repos/miniant-git/REAL/releases/tags/updater-v2");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "real_updater_v1");

    Buffer buffer;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, PopulateBuffer);

    CURLcode status = curl_easy_perform(curl);
    if (status != CURLE_OK) {
        curl_easy_cleanup(curl);
        return false;
    }

    long responseCode;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
    if (responseCode != 200) {
        curl_easy_cleanup(curl);
        return false;
    }

    json response = json::parse(buffer);
    std::optional<Version> version = Version::Find(response["name"]);
    if (!version) {
        curl_easy_cleanup(curl);
        return false;
    }

    std::cout << "A new update is available!" << std::endl;
    DisplayReleaseNotes(response["body"]);
    std::cout << "Do you want to update to " << version.value().ToString() << "? [y/N]: ";
    char line[5];
    std::cin.getline(line, 5);
    std::string prompt(line);
    std::transform(prompt.begin(), prompt.end(), prompt.begin(), std::tolower);
    if (prompt != "y" && prompt != "yes") {
        std::cout << "No: Keeping the current version." << std::endl;
        curl_easy_cleanup(curl);
        return true;
    }

    std::optional<const json*> executable = FindExecutableAsset(response);
    if (!executable) {
        std::cout << "Error: misconfigured update assets." << std::endl;
        curl_easy_cleanup(curl);
        return true;
    }

    std::filesystem::path tempExecutable(m_executable.string() + "~TEMP");

    const char* url = static_cast<std::string>((*executable.value())["browser_download_url"]).c_str();
    FILE* executableFile;
    fopen_s(&executableFile, tempExecutable.string().c_str(), "wb");
    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, executableFile);

    std::cout << "Downloading update..." << std::endl;
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    fclose(executableFile);

    try {
        std::filesystem::rename(m_executable, m_executable.string() + "~DELETE");
        std::filesystem::rename(tempExecutable, m_executable);
    } catch (std::filesystem::filesystem_error& error) {
        std::cout << "Error: " << error.what() << std::endl;
        return true;
    }

    std::cout << "Updated successfully! Restart the application to apply changes." << std::endl;
    return true;
}
