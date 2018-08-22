/*
 * dir.h
 *
 *  Created on: Oct 15, 2015
 *      Author: bog
 */

#ifndef DIR_H_
#define DIR_H_

#include <string>
#include <vector>
#include <functional>

namespace filesystem {

std::vector<std::string> getFiles(std::string const& baseDir, bool includeSubDirs);
bool isDir(std::string const& dir);
bool pathExists(std::string const& path);
bool mkDir(std::string const& path);
bool mkDirRecursive(std::string const& path);
std::string getFileName(std::string const& path);
std::string stripExt(std::string const& path);
std::string getFileExt(std::string const& path);
unsigned long getFileTimestamp(std::string const& path);	// returns timestamp in seconds (since 1970 or something)
bool touchFile(std::string const& path);
bool copyFile(std::string const& source, std::string const& dest);
bool deleteFile(std::string const& path);
uint64_t getFileSize(std::string const& path);

void applyRecursive(std::string const& baseDir, std::function<void(std::string const& filename)> func);

} // namespace

#endif /* DIR_H_ */
