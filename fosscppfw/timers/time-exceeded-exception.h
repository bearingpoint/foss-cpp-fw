#pragma once

#include <stdexcept>

namespace blfw {

class TimeExceededException : public std::runtime_error {
public:
	explicit TimeExceededException(std::string const& message) : std::runtime_error(message) {};
};

} // namespace blfw