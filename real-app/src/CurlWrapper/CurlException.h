#pragma once

#include <curl/curl.h>

#include <stdexcept>

namespace miniant::CurlWrapper {

class CurlException : public std::runtime_error {
public:
    explicit CurlException(const std::string& what_arg);
    explicit CurlException(const char* what_arg);
    explicit CurlException(CURLcode code);
};

}
