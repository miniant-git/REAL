#pragma once

#include "Version.h"

#include <filesystem>

namespace miniant::WasapiLatency {

class AutoUpdater {
public:
    AutoUpdater(Version currentVersion, std::filesystem::path executable);
    ~AutoUpdater();

    bool Update() const;

private:
    Version m_currentVersion;

    std::filesystem::path m_executable;

    bool UpdateUpdater() const;
};

}
