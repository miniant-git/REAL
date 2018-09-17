#pragma once

#include <stdexcept>

namespace miniant::Windows {

class Exception : std::runtime_error {
public:
    Exception() noexcept;
};

}
