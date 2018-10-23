#pragma once

#include "ExpectedError.h"

#include <tl/expected.hpp>

#include <cinttypes>

namespace miniant::AutoUpdater {

class VersionError : public ExpectedError {
public:
    explicit VersionError(const char* message):
        ExpectedError(message) {}
};

class Version {
public:
    constexpr Version() noexcept:
        m_major(0), m_minor(0), m_patch(0) {};

    constexpr Version(uint16_t major, uint16_t minor, uint16_t patch) noexcept:
        m_major(major), m_minor(minor), m_patch(patch) {}

    std::string ToString() const;

    bool operator==(const Version& rhs) const noexcept;
    bool operator!=(const Version& rhs) const noexcept;
    bool operator< (const Version& rhs) const noexcept;
    bool operator<=(const Version& rhs) const noexcept;
    bool operator> (const Version& rhs) const noexcept;
    bool operator>=(const Version& rhs) const noexcept;

    static tl::expected<Version, VersionError> Parse(const std::string& versionString);
    static tl::expected<Version, VersionError> Find(const std::string& string);

private:
    uint16_t m_major;
    uint16_t m_minor;
    uint16_t m_patch;
};

}
