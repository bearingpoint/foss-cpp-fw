#pragma once

#include <string>

namespace net {

struct result {
	enum result_code {
		ok,				// the operation succeeded
		err_timeout,	// the operation failed due to a timeout while waiting for remote response
		err_aborted,	// the operation failed due to the connection being forcefully closed
		err_refused,	// the connection attempt failed because the host refused to connect
		err_unreachable,// host cannot be resolved or no route can be established
		err_portInUse,	// the requested port is already in use, we can't listen on it
		err_unknown		// the operation failed for an unknown cause
	} code;
	std::string message = "";

	result(result_code code, std::string msg)
		: code(code), message(msg) {}
	result(result_code code)
		: code(code) {}

	result& operator=(result const& r) {
		code = r.code;
		message = r.message;
		return *this;
	}

	bool operator == (result_code c) const {
		return this->code == c;
	}
	bool operator != (result_code c) const {
		return !operator==(c);
	}
};

}
