#include "CurlFileWriter.h"

using namespace miniant::CurlWrapper;

size_t WriteToFile(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto file = static_cast<std::ofstream*>(userdata);
    if (file->is_open()) {
        file->write(ptr, nmemb);
        return nmemb;
    }

    return 0;
}

CurlFileWriter::CurlFileWriter(std::filesystem::path filepath):
    m_file(filepath, std::ofstream::binary) {}

void CurlFileWriter::Close() {
    m_file.close();
}

CurlHandle::write_callback CurlFileWriter::GetWriteCallback() {
    return &WriteToFile;
}

void* CurlFileWriter::GetWriteData() {
    return &m_file;
}
