#pragma once

#include "Version.h"

namespace miniant::WasapiLatency {

class AutoUpdater {
public:
    AutoUpdater(Version currentVersion);
    ~AutoUpdater();

    bool Update() const;

private:
    Version m_currentVersion;

    bool UpdateUpdater() const;
};

}
