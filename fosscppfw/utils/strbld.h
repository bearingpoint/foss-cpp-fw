#pragma once

#include <sstream>
#include <string>

// String Builder class - use to quickly build an std::string in a single line of code
// by concatenating multiple streamable objects of various types
//
// example:
// std::string out = strbld() << "some text " << intVar << " other text " << whatever_other_var << " and so on";
//
class strbld {
public:
	operator std::string() const {
		return sstream_.str();
	}

	bool empty() {
		return size() == 0;
	}

	size_t size() {
		size_t crtPos = sstream_.tellg();
		sstream_.seekg(0, std::ios::end);
		size_t size = sstream_.tellg();
		sstream_.seekg(crtPos, std::ios::beg);
		return size;
	}

	template <class C>
	strbld& operator << (C const& c) {
		sstream_ << c;
		return *this;
	}

private:
	std::stringstream sstream_;
};
