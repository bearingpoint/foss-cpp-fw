/*
 * filesystem.cpp
 *
 *  Created on: Oct 15, 2015
 *	  Author: bog
 */

#ifdef __WIN32__
	#define UNICODE
	#include <windows.h>
#endif

#include "filesystem.h"
#include "log.h"
#include "strManip.h"
#include "strbld.h"
#include "wstrconv.h"

#include <filesystem>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <fstream>
#include <algorithm>
#include <stdexcept>

namespace filesystem {

bool isDir(std::string const& path) {
	return std::filesystem::is_directory(path);
}

bool pathExists(std::string const& path) {
	return std::filesystem::exists(path);
}

bool touchFile(std::string const& path) {
	std::filesystem::last_write_time(path, std::filesystem::file_time_type::clock::now());
	return true;
}

bool mkDir(std::string const& path) {
	return std::filesystem::create_directory(path);
}

bool mkDirRecursive(std::string const& path) {
	return std::filesystem::create_directories(path);
}

std::string getFileName(std::string const& path) {
	if (path.find('/') != path.npos)
		return path.c_str() + path.find_last_of('/') + 1;
	else if (path.find("\\"))
		return path.c_str() + path.find_last_of('\\') + 1;
	else
		return path;
}

std::string stripExt(std::string const& path) {
	auto ppos = path.find_last_of('.');
	if (ppos != path.npos)
		return path.substr(0, ppos);
	else
		return path;
}

std::string getFileExt(std::string const& path) {
	auto dotPos = path.find_last_of('.');
	// check if a slash or back-slash occur after the dot, in that case there's no extension
	// ex: some/path.dir/file -> no extension instead of "dir/file"
	auto spos = path.find_last_of('/');
	auto bspos = path.find_last_of('\\');
	if (dotPos == path.npos || (spos != path.npos && spos > dotPos)
		|| (bspos != path.npos && bspos > dotPos))
		return "";
	return path.substr(dotPos + 1);
}

unsigned long getFileTimestamp(std::string const& path) {
	return std::filesystem::last_write_time(path).time_since_epoch().count();
}

bool copyFile(std::string const& source, std::string const& dest) {
	return std::filesystem::copy_file(source, dest);
}

bool deleteFile(std::string const& path) {
	return std::filesystem::remove(path);
}

uint64_t getFileSize(std::string const& path) {
	LOGPREFIX("getFileSize");
	UnicodeFileReader fr(path);
	return fr.getSize();
}

std::vector<std::string> getFiles(std::string const& baseDir, bool includeSubDirs) {
	std::vector<std::string> entries;
	for (std::filesystem::directory_entry const& entry : std::filesystem::directory_iterator(baseDir)) {
		if (
			entry.is_regular_file() 
			|| (entry.is_directory() && includeSubDirs)
		) {
			entries.push_back(entry.path().string());
		}
	}
	return entries;
}

std::vector<std::string> getAllFilesFromDir(std::string const& baseDir) {
	std::vector<std::string> entries;
	for (std::filesystem::directory_entry const& entry : std::filesystem::recursive_directory_iterator(baseDir)) {
		if (entry.is_regular_file()) {
			entries.push_back(entry.path().string());
		}
	}
	return entries;
}

void applyRecursive(std::string const& baseDir, std::function<void(std::string const& filename)> func) {
	std::vector<std::string> files = getFiles(baseDir, true);
	for (auto &f : files) {
		if (isDir(f))
			applyRecursive(f, func);
		else
			func(f);
	}
}

std::string getFileDirectory(std::string const& filePath) {
	auto fn = getFileName(filePath);
	return filePath.substr(0, filePath.size() - fn.size());
}

std::string getHomeDirectory() {
	std::string configFilePath;
#ifdef __WIN32__
	std::string homeDrive = getenv("HOMEDRIVE");
	std::string homePath = getenv("HOMEPATH");
	configFilePath = homeDrive + "/" + homePath;
#else
	configFilePath = getenv("HOME");
#endif
	return configFilePath;
}

bool deleteFolderContent(const std::string& folderPath) {
	LOGPREFIX("deleteFolderContent");
	try {
		std::filesystem::path folder(folderPath);

		if (!std::filesystem::exists(folder) || !std::filesystem::is_directory(folder)) {
			ERRORLOG(errno << "Invalid folder path: " << folderPath << std::endl);
			return false;
		}

		for (const auto& entry : std::filesystem::recursive_directory_iterator(folder)) {
			try {
				if (std::filesystem::is_regular_file(entry.path())) {
					std::filesystem::remove(entry.path()); // Delete regular files
				} else if (std::filesystem::is_directory(entry.path())) {
					std::filesystem::remove_all(entry.path()); // Recursively delete directories
				}
			} catch (const std::exception& ex) {
				ERRORLOG(errno << "Error deleting " << entry.path() << ": " << ex.what() << std::endl);
				return false;
			}
		}
		LOGLN("Folder " << folder << " cleaned up successfully." << std::endl);
	} catch (const std::exception& ex) {
		ERRORLOG(errno << "Error: " << ex.what() << std::endl);
		return false;
	}
	return true;
}

std::vector<std::string> readFileLines(std::string const& filePath) {
	std::ifstream inputFile(filePath);
	std::string line;
	std::vector<std::string> lines;

	if (inputFile.is_open()) {
		while (std::getline(inputFile, line)) {
			lines.push_back(line);
		}
		inputFile.close();
	} else {
		throw std::runtime_error("Unable to open file \n");
	}
	return lines;
}

std::string readFileContent(std::string const& path, unsigned startIndex, unsigned endIndex) {
	std::ifstream file(path);
	if(file.is_open()) {
		std::string content;
		content.resize(endIndex - startIndex);
		file.read(&content[0], endIndex - startIndex);
		return content;
	} else {
		throw std::runtime_error("Unable to open file " + path + "\n");
	}
}
std::string normalizePath(std::string const& inPath) {
	std::filesystem::path path(inPath);
	std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(path); // does not throw an exception if path does not exists
	return canonicalPath.make_preferred().string();
}

} // namespace
