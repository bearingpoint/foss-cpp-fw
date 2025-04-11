#pragma once

#include <cassert>

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

	result(result_code code, std::string const& msg)
		: code(code), message(msg) {}
	explicit result(result_code code)
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

inline std::string errorString(net::result const& err) {
	switch (err.code) {
		case net::result::ok: return "[OK]";
		case net::result::err_timeout: return "[Timeout] " + err.message;
		case net::result::err_aborted: return "[Aborted] " + err.message;
		case net::result::err_refused: return "[Refused] " + err.message;
		case net::result::err_portInUse: return "[PortInUse] " + err.message;
		case net::result::err_unreachable: return "[HostUnreachable] " + err.message;
		case net::result::err_unknown: return "[Unknown] " + err.message;
		default:
			assert(false && "unhandled error type");
			return "[Unknown] " + err.message;
	}
}

}
