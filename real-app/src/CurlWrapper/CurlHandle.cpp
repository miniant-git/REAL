#include "CurlHandle.h"

#include "CurlException.h"

using namespace miniant::CurlWrapper;

CurlException CreateCurlException(const char* errorBuffer, CURLcode errorCode) {
    if (strlen(errorBuffer) > 0)
        return CurlException(errorBuffer);

    return CurlException(errorCode);
}

CurlHandle::CurlHandle() {
    m_curl = curl_easy_init();
    if (m_curl == NULL)
        throw CurlException("Failed to create curl easy handle.");

    m_errorBuffer = std::make_unique<char[]>(CURL_ERROR_SIZE);
    if (curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, m_errorBuffer.get()) != CURLE_OK)
        throw CurlException("Failed to assign error buffer to curl handle.");
}

CurlHandle::~CurlHandle() noexcept {
    curl_easy_cleanup(m_curl);
}

void CurlHandle::SetUrl(const std::string& url) {
    CURLcode errorCode = curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
    if (errorCode != CURLE_OK)
        throw CreateCurlException(m_errorBuffer.get(), errorCode);
}

void CurlHandle::SetUserAgent(const std::string& userAgent) {
    CURLcode errorCode = curl_easy_setopt(m_curl, CURLOPT_USERAGENT, userAgent.c_str());
    if (errorCode != CURLE_OK)
        throw CreateCurlException(m_errorBuffer.get(), errorCode);
}

long CurlHandle::Perform(void* userData, write_callback callback) {
    if (CURLcode errorCode = curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, userData);  errorCode != CURLE_OK)
        throw CreateCurlException(m_errorBuffer.get(), errorCode);

    if (CURLcode errorCode = curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, callback); errorCode != CURLE_OK)
        throw CreateCurlException(m_errorBuffer.get(), errorCode);

    if (CURLcode errorCode = curl_easy_perform(m_curl); errorCode != CURLE_OK)
        throw CreateCurlException(m_errorBuffer.get(), errorCode);

    long responseCode;
    if (CURLcode errorCode = curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &responseCode); errorCode != CURLE_OK)
        throw CreateCurlException(m_errorBuffer.get(), errorCode);

    return responseCode;
}

void CurlHandle::FollowRedirects(bool enabled) {
    long enable = 0L;
    if (enabled)
        enable = 1L;

    CURLcode errorCode = curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, enable);
    if (errorCode != CURLE_OK)
        throw CreateCurlException(m_errorBuffer.get(), errorCode);
}

void CurlHandle::Reset() {
    curl_easy_reset(m_curl);
}
