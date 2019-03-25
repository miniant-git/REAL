#pragma once

#include "ExpectedError.h"
#include "Version.h"

#include <optional>

namespace miniant::AutoUpdater {

struct UpdateInfo {
    Version version;
    std::string downloadUrl;
    std::optional<std::string> releaseNotes;
};

class AutoUpdaterError : public ExpectedError {
public:
    explicit AutoUpdaterError(std::string message) noexcept:
        ExpectedError(std::move(message)) {}

    explicit AutoUpdaterError(const char* message):
        ExpectedError(message) {}

    explicit AutoUpdaterError(const ExpectedError& error):
        ExpectedError(error.GetMessage()) {}
};

class AutoUpdater {
public:
    AutoUpdater();
    ~AutoUpdater();

    std::optional<std::string> IsAppSuperseded();

    tl::expected<bool, AutoUpdaterError> CleanupPreviousSetup();
    tl::expected<UpdateInfo, AutoUpdaterError> GetUpdateInfo() const;
    tl::expected<void, AutoUpdaterError> ApplyUpdate(const UpdateInfo& info) const;
};

}
