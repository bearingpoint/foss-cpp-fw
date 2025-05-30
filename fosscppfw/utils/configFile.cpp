/*
 * configFile.cpp
 *
 *  Created on: Oct 2, 2015
 *	  Author: bog
 */

#include "configFile.h"
#include "log.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

bool parseConfigFile(std::string const& filePath, std::map<std::string, std::string> &opts, std::vector<std::string> const& requiredOpts) {
	std::ifstream f(filePath);
	if (!f.is_open()) {
		ERRORLOG("Cannot read config file \"" << filePath << "\"");
		return false;
	}

	LOGLN("Parsing config file \"" << filePath << "\"...");

	std::string line;
	try {
		while (std::getline(f, line)) {
			if (line.empty() || line[0] == '#')  // ignore comments and empty lines
				continue;

			std::stringstream ss(line);

			std::string tokenName, equalSign, value;
			ss >> tokenName >> equalSign;
			std::ws(ss);	// skip whitespace after =
			std::getline(ss, value);

			if (equalSign != "=") {
				ERRORLOG("Invalid line in config file:\n"<<line);
			}

			opts[tokenName] = value;
		}
		LOGLN("Finished parsing config file.");
	} catch (std::runtime_error const& err) {
		ERRORLOG("FAILED parsing config file \"" << filePath <<"\"\n" << err.what());
		return false;
	}

	for (auto &o : requiredOpts) {
		if (opts.find(o) == opts.end()) {
			ERRORLOG("Missing required key \"" << o << "\" from config file !!!");
			return false;
		}
	}
	return true;
}
