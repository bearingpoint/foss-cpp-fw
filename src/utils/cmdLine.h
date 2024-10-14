/*
 * cmdLine.h
 *
 *  Created on: Oct 1, 2015
 *	  Author: bog
 */

#ifndef CMDLINE_H_
#define CMDLINE_H_

#include <string>
#include <map>
#include <vector>

struct CmdLineOption {
	bool requireValue = false;	// specify true if the --option should be followed by a value
	bool mandatory = false;		// specify true if the --option must be present

	bool isOn = false;			// this will be set to true if the --option was present in the command line
	std::string value;			// for options that requireValues, this will contain the extracted value after parsing
};

/**
 * Parses the command line, returning information about --options and the free arguments in the order they appear.
 * For example:
 * ./command freeArg1 --flag1 --flag2 freeArg2 --optionWith value freeArg3
 * given the following configuration:
 * 		"flag1": {},
 * 		"flag2": {},
 * 		"flag3": {}
 * 		"optionWith": {requireValue: true}
 * will set the "isOn" entries for the "flag1" and "flag2" (but not "flag3" since it's not present in the command line),
 * and will return the following in out_freeArgs:
 * 		["freeArg1", "freeArg2", "freeArg3"]
 * "value" is not considered as a free argument, but as the value for --optionWith.
 * Returns true on success and false if any of the following occurs:
 * 		- a "mandatory" option was not found in the command line
 * 		- an option with "requireValue"=true didn't have a value after it (either another --flag or the end of the command line)
 */
bool parseCommandLine(int argc, char* argv[], std::map<std::string, CmdLineOption> &opts, std::vector<std::string> &out_freeArgs);

#endif /* CMDLINE_H_ */
