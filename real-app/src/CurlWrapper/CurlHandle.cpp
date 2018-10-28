#include "CurlHandle.h"

#include <curl/curl.h>

#include <cassert>

using namespace miniant;
using namespace miniant::CurlWrapper;

CurlError CreateCurlError(const char* errorBuffer, CURLcode errorCode) {
    assert(errorBuffer != nullptr);

    if (strlen(errorBuffer) > 0) {
        return CurlError(errorBuffer);
    }

    return CurlError(errorCode);
}

CurlHandle::CurlHandle(CURL* curl, std::unique_ptr<char[]>&& errorBuffer) noexcept:
    m_curl(curl), m_errorBuffer(std::move(errorBuffer)) {}

CurlHandle::CurlHandle(CurlHandle&& curl) noexcept:
    m_curl(curl.m_curl),
    m_errorBuffer(std::move(curl.m_errorBuffer)) {
    assert(curl.m_curl != nullptr);
    curl.m_curl = nullptr;
}

CurlHandle::~CurlHandle() {
    if (m_curl != nullptr) {
        curl_easy_cleanup(m_curl);
    }
}

CurlHandle& CurlHandle::operator= (CurlHandle&& rhs) noexcept {
    assert(rhs.m_curl != nullptr);

    m_curl = rhs.m_curl;
    rhs.m_curl = nullptr;
    m_errorBuffer = std::move(rhs.m_errorBuffer);
    return *this;
}

tl::expected<void, CurlError> CurlHandle::SetUrl(const std::string& url) {
    assert(m_curl != nullptr);

    CURLcode errorCode = curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
    if (errorCode != CURLE_OK) {
        return tl::make_unexpected(CreateCurlError(m_errorBuffer.get(), errorCode));
    }

    return {};
}

tl::expected<void, CurlError> CurlHandle::SetUserAgent(const std::string& userAgent) {
    assert(m_curl != nullptr);

    CURLcode errorCode = curl_easy_setopt(m_curl, CURLOPT_USERAGENT, userAgent.c_str());
    if (errorCode != CURLE_OK) {
        return tl::make_unexpected(CreateCurlError(m_errorBuffer.get(), errorCode));
    }

    return {};
}

tl::expected<long, CurlError> CurlHandle::Perform(void* userData, write_callback callback) {
    assert(m_curl != nullptr);

    if (CURLcode errorCode = curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, userData);  errorCode != CURLE_OK) {
        return tl::make_unexpected(CreateCurlError(m_errorBuffer.get(), errorCode));
    }

    if (CURLcode errorCode = curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, callback); errorCode != CURLE_OK) {
        return tl::make_unexpected(CreateCurlError(m_errorBuffer.get(), errorCode));
    }

    if (CURLcode errorCode = curl_easy_perform(m_curl); errorCode != CURLE_OK) {
        return tl::make_unexpected(CreateCurlError(m_errorBuffer.get(), errorCode));
    }

    long responseCode;
    if (CURLcode errorCode = curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &responseCode); errorCode != CURLE_OK) {
        return tl::make_unexpected(CreateCurlError(m_errorBuffer.get(), errorCode));
    }

    return responseCode;
}

tl::expected<void, CurlError> CurlHandle::FollowRedirects(bool enabled) {
    assert(m_curl != nullptr);

    CURLcode errorCode = curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, static_cast<long>(enabled));
    if (errorCode != CURLE_OK) {
        return tl::make_unexpected(CreateCurlError(m_errorBuffer.get(), errorCode));
    }

    return {};
}

void CurlHandle::Reset() {
    assert(m_curl != nullptr);
    curl_easy_reset(m_curl);
}

tl::expected<CurlHandle, CurlError> CurlHandle::Create() {
    CURL* curl = curl_easy_init();
    if (curl == NULL) {
        return tl::make_unexpected(CurlError("Failed to create curl easy handle."));
    }

    auto errorBuffer = std::make_unique<char[]>(CURL_ERROR_SIZE);
    if (curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer.get()) != CURLE_OK) {
        return tl::make_unexpected(CurlError("Failed to assign error buffer to curl handle."));
    }

    return CurlHandle(curl, std::move(errorBuffer));
}

void CurlHandle::InitialiseCurl() {
    curl_global_init(CURL_GLOBAL_ALL);
}

void CurlHandle::CleanupCurl() {
    curl_global_cleanup();
}
