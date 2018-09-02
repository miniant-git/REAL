#pragma once

#include "CurlWriter.h"

#include <vector>

namespace miniant::CurlWrapper {

class CurlMemoryWriter : public CurlWriter {
public:
    std::vector<char>& GetBuffer();

protected:
    virtual CurlHandle::write_callback GetWriteCallback() override;
    virtual void* GetWriteData() override;

private:
    std::vector<char> m_buffer;
};

}
