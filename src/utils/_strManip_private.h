#pragma once

#include <sstream>

template<typename T>
std::string strJoin(std::vector<T> const& parts, std::string const& delimiter) {
	std::stringstream ss;
	for (unsigned i=0; i<parts.size(); i++) {
		ss << parts[i];
		if (i < parts.size() - 1) {
			ss << delimiter;
		}
	}
	return ss.str();
}
