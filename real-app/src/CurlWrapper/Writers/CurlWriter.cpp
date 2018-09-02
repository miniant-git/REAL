#include "CurlWriter.h"

using namespace miniant::CurlWrapper;

long CurlWriter::InitiateRequest(CurlHandle& handle) {
    return handle.Perform(GetWriteData(), GetWriteCallback());
}
