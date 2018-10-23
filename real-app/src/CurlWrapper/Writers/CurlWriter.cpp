#include "CurlWriter.h"

using namespace miniant::CurlWrapper;

tl::expected<long, CurlError> CurlWriter::InitiateRequest(CurlHandle& handle) {
    return handle.Perform(GetWriteData(), GetWriteCallback());
}
