#pragma once

#include "../CurlHandle.h"

namespace miniant::CurlWrapper {

class CurlWriter {
public:
    virtual ~CurlWriter() = default;

    long InitiateRequest(CurlHandle& handle);

protected:
    virtual CurlHandle::write_callback GetWriteCallback() = 0;
    virtual void* GetWriteData() = 0;
};

}
