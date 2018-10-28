#pragma once

#include "../ExpectedError.h"

namespace miniant::CurlWrapper {

class CurlError : public ExpectedError {
public:
    explicit CurlError(std::string message) noexcept:
        ExpectedError(std::move(message)) {}

    explicit CurlError(const char* message):
        ExpectedError(message) {}

    explicit CurlError(int curlCode);
};

}
