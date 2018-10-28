#include "CurlError.h"

#include <curl/curl.h>

#include <cassert>

using namespace miniant::CurlWrapper;

CurlError::CurlError(int curlCode):
    ExpectedError(curl_easy_strerror(static_cast<CURLcode>(curlCode))) {
    assert(curlCode >= 0 && curlCode < CURL_LAST);
}
