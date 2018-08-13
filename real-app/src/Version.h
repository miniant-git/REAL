#pragma once

#include <cinttypes>
#include <string>
#include <optional>

namespace miniant::WasapiLatency {

class Version {
public:
    constexpr Version(uint16_t major, uint16_t minor, uint16_t patch):
        m_major(major), m_minor(minor), m_patch(patch) {}

    std::string ToString() const;

    bool operator==(const Version& rhs) const;
    bool operator!=(const Version& rhs) const;
    bool operator< (const Version& rhs) const;
    bool operator<=(const Version& rhs) const;
    bool operator> (const Version& rhs) const;
    bool operator>=(const Version& rhs) const;

    static std::optional<Version> Parse(const std::string& versionString);

private:
    uint16_t m_major;
    uint16_t m_minor;
    uint16_t m_patch;
};

}
