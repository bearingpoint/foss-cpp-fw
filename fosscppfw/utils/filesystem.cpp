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

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>
#include <filesystem>

#if !defined(__WIN32__) && !defined(__MACOSX__)
	#include <sys/sendfile.h>
#endif

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <fstream>
#include <algorithm>
#include <stdexcept>

namespace filesystem {

bool isDir(std::string const& dir) {
	struct stat fileInfo;
	stat(dir.c_str(), &fileInfo);
	return S_ISDIR(fileInfo.st_mode);
}

bool pathExists(std::string const& path) {
	struct stat fileInfo;
	return stat(path.c_str(), &fileInfo) == 0;
}

bool touchFile(std::string const& path) {
	if (!pathExists(path)) {
		std::ofstream f(path);
		return f.is_open();
	}
	return !utime(path.c_str(), nullptr);
}

bool mkDir(std::string const& path) {
	LOGPREFIX("mkDir");
	if (0 != mkdir(path.c_str()
#ifndef __WIN32__
			, S_IRWXU
#endif
			)) {
		ERRORLOG(errno << ": Could not create directory \"" << path << "\"");
		return false;
	}
	return true;
}

bool mkDirRecursive(std::string const& path) {
	if (path.empty() || path==".")
		return true;
	std::vector<std::string> dirs = strSplit(path, '/');
	// remove empty entries and dot entries
	dirs.erase(std::remove_if(dirs.begin(), dirs.end(), [](auto &s) {
		return s.empty() || s == ".";
	}), dirs.end());
	std::string builtPath = path[0] == '/' ? "/" : "./";	// start with absolute or relative path
	for (auto &d : dirs) {
		builtPath += d + "/";
		if (!pathExists(builtPath))
			if (!mkDir(builtPath))
				return false;
	}
	return true;
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
	LOGPREFIX("getFileTimestamp");
	struct stat fileInfo;
	if (stat(path.c_str(), &fileInfo) < 0) {
		ERRORLOG("Could not stat() file \"" << path << "\"");
		return 0;
	}
#if !defined(__WIN32__) && !defined(__MACOSX__)
	return (unsigned long)fileInfo.st_mtim.tv_sec;
#else
	return (unsigned long)fileInfo.st_mtime;
#endif
}

bool copyFile(std::string const& source, std::string const& dest) {
	LOGPREFIX("copyFile");
	bool ret = true;
	int fs = open(source.c_str(), O_RDONLY, 0);
	if (fs < 0) {
		ERRORLOG(errno << ": Could not open source file \"" << source <<"\"");
		ret = false;
	}
	int fd = open(dest.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		ERRORLOG(errno << ": Could not open destination file \"" << dest <<"\"");
		ret = false;
	}

	if (ret) {
		struct stat stat_source;
		if (fstat(fs, &stat_source) < 0) {
			ERRORLOG(errno << ": Could not stat() source file \"" << source << "\"");
			ret = false;
		}
#if defined(__WIN32__) || defined(__MACOSX__)
		if (ret) {
			char* data = new char[stat_source.st_size];
			ret = (read(fs, data, stat_source.st_size) >= 0);
			if (ret)
				ret = (write(fd, data, stat_source.st_size) >= 0);
		}
#else
		if (ret) {
			ret = (sendfile(fd, fs, 0, stat_source.st_size) >= 0);
		}
#endif
		if (!ret) {
			ERRORLOG(errno << ": Could not copy file \n" <<
				"\"" << source <<"\" -> \"" << dest << "\"");
		}
	}

	close(fs);
	close(fd);

	return ret;
}

bool deleteFile(std::string const& path) {
	LOGPREFIX("deleteFile");
	if (unlink(path.c_str()) < 0) {
		ERRORLOG(errno << ": Could not delete file \"" << path << "\"");
		return false;
	}
	return true;
}

uint64_t getFileSize(std::string const& path) {
	LOGPREFIX("getFileSize");
	UnicodeFileReader fr(path);
	return fr.getSize();
}

std::vector<std::string> getFiles(std::string const& baseDir, bool includeSubDirs) {
	LOGPREFIX("getFiles");
	std::vector<std::string> files;
	DIR *dp;
	struct dirent *dirp;
	std::string separator = (baseDir.back() != '\\') ? "\\" : "";
	if ((dp = opendir(baseDir.c_str())) == NULL) {
		ERRORLOG(errno << ": Could not open directory \"" << baseDir << "\"\n");
	} else {
		while ((dirp = readdir(dp)) != NULL) {
			if (dirp->d_name != std::string(".") && dirp->d_name != std::string("..")) {
				auto path = baseDir + separator + dirp->d_name;
				if (!isDir(path) || includeSubDirs)
					files.push_back(path);
			}
		}
		closedir(dp);
	}
	return files;
}

std::vector<std::string> getAllFilesFromDir(std::string const& baseDir) {
	std::vector<std::string> filesPath = {};
	for (auto &path: getFiles(baseDir, true)) {
		if (isDir(path)) {
			std::vector<std::string> innerFilesPath = getAllFilesFromDir(path);
			filesPath.reserve(filesPath.size() + innerFilesPath.size());
			std::move(innerFilesPath.begin(), innerFilesPath.end(), std::back_inserter(filesPath));
		} else {
			filesPath.push_back(path);
		}
	}
	return filesPath;
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
