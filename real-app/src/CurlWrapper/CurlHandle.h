#pragma once

#include <curl/curl.h>

#include <memory>
#include <string>

namespace miniant::CurlWrapper {

class CurlHandle {
public:
    using write_callback = size_t (*)(char* ptr, size_t size, size_t nmemb, void* userdata);

    CurlHandle();
    ~CurlHandle() noexcept;

    void SetUrl(const std::string& url);
    void SetUserAgent(const std::string& userAgent);

    long Perform(void* userData, write_callback callback);

    void FollowRedirects(bool enabled);
    void Reset();

private:
    CURL* m_curl;
    std::unique_ptr<char[]> m_errorBuffer;
};

}
