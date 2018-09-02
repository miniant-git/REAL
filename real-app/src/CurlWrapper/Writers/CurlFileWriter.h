#pragma once

#include "CurlWriter.h"

#include <filesystem>
#include <fstream>

namespace miniant::CurlWrapper {

class CurlFileWriter : public CurlWriter {
public:
    CurlFileWriter(std::filesystem::path filepath);

    void Close();

protected:
    virtual CurlHandle::write_callback GetWriteCallback() override;
    virtual void* GetWriteData() override;

private:
    std::ofstream m_file;
};

}
