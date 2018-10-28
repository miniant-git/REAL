#pragma once

#include "CurlError.h"

#include <tl/expected.hpp>

#include <memory>

namespace miniant::CurlWrapper {

class CurlHandle {
public:
    using write_callback = size_t (*)(char* ptr, size_t size, size_t nmemb, void* userdata);

    CurlHandle(CurlHandle&& curl) noexcept;
    ~CurlHandle();

    CurlHandle& operator= (CurlHandle&& rhs) noexcept;

    tl::expected<void, CurlError> SetUrl(const std::string& url);
    tl::expected<void, CurlError> SetUserAgent(const std::string& userAgent);

    tl::expected<long, CurlError> Perform(void* userData, write_callback callback);

    tl::expected<void, CurlError> FollowRedirects(bool enabled);
    void Reset();

    static tl::expected<CurlHandle, CurlError> Create();

    static void InitialiseCurl();
    static void CleanupCurl();

private:
    explicit CurlHandle(void* curl, std::unique_ptr<char[]>&& errorBuffer) noexcept;

    void* m_curl;
    std::unique_ptr<char[]> m_errorBuffer;
};

}
