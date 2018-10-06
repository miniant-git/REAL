#pragma once

#include <cinttypes>
#include <string>
#include <optional>

namespace miniant::AutoUpdater {

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

    static std::optional<Version> Parse(const std::string& versionString);
    static std::optional<Version> Find(const std::string& string);

private:
    uint16_t m_major;
    uint16_t m_minor;
    uint16_t m_patch;
};

}
