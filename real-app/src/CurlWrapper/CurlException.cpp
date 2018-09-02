#include "CurlException.h"

using namespace miniant::CurlWrapper;

CurlException::CurlException(const std::string& what_arg) :
    std::runtime_error(what_arg) {}

CurlException::CurlException(const char* what_arg) :
    std::runtime_error(what_arg) {}

CurlException::CurlException(CURLcode code) :
    std::runtime_error(curl_easy_strerror(code)) {}
