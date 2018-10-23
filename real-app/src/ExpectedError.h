#pragma once

#include <string>

namespace miniant {

class ExpectedError {
public:
    explicit ExpectedError(std::string message) noexcept:
        m_message(std::move(message)) {}

    explicit ExpectedError(const char* message):
        m_message(message) {}

    ExpectedError(const ExpectedError&) = default;
    ExpectedError(ExpectedError&&) noexcept = default;

    ExpectedError& operator= (const ExpectedError&) = default;
    ExpectedError& operator= (ExpectedError&&) noexcept = default;

    virtual ~ExpectedError() = default;

    virtual std::string GetMessage() const {
        return m_message;
    }

private:
    std::string m_message;
};

}
