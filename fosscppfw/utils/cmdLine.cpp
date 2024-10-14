/*
 * cmdLine.cpp
 *
 *  Created on: Oct 1, 2015
 *	  Author: bog
 */

#include "cmdLine.h"
#include "log.h"
#include <cstring>


bool parseCommandLine(int argc, char* argv[], std::map<std::string, CmdLineOption> &opts, std::vector<std::string> &out_freeArgs) {
	for (int i=1; i<argc; i++) {
		if (strstr(argv[i], "--") == argv[i]) { // starts with --
			std::string const optName = argv[i]+2;
			if (opts[optName].requireValue && (argc == i+1 || strstr(argv[i+1], "--") == argv[i+1])) {
				ERRORLOG("Expected value after " << argv[i]);
				return false;
			}
			opts[optName].isOn = true;
			if (opts[optName].requireValue) {
				opts[optName].value = argv[i+1];
				i++;
			}
		} else {
			out_freeArgs.push_back(argv[i]);
		}
	}
	for (auto &o : opts) {
		if (o.second.mandatory && !o.second.isOn) {
			ERRORLOG("Missing mandatory argument --" << o.first);
			return false;
		}
	}
	return true;
}
