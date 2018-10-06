#pragma once

#include "Version.h"

namespace miniant::AutoUpdater {

struct UpdateInfo {
    Version version;
    std::string downloadUrl;
    std::string releaseNotes;
};

class AutoUpdater {
public:
    AutoUpdater();
    ~AutoUpdater();

    std::optional<UpdateInfo> GetUpdateInfo() const;
    bool ApplyUpdate(const UpdateInfo& info) const;
};

}
