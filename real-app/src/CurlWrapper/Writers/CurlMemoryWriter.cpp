#include "CurlMemoryWriter.h"

using namespace miniant::CurlWrapper;

size_t WriteToBuffer(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto buffer = reinterpret_cast<std::vector<char>*>(userdata);
    buffer->insert(buffer->end(), ptr, ptr+nmemb);
    return nmemb;
}

std::vector<char>& CurlMemoryWriter::GetBuffer() {
    return m_buffer;
}

CurlHandle::write_callback CurlMemoryWriter::GetWriteCallback() {
    return &WriteToBuffer;
}

void* CurlMemoryWriter::GetWriteData() {
    return &m_buffer;
}
