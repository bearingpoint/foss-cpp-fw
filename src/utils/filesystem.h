/*
 * dir.h
 *
 *  Created on: Oct 15, 2015
 *	  Author: bog
 */

#ifndef DIR_H_
#define DIR_H_

#include <string>
#include <vector>
#include <functional>
#include <stdint.h>

class BaseFileReader;

namespace filesystem {

std::vector<std::string> getFiles(std::string const& baseDir, bool includeSubDirs);
std::vector<std::string> getAllFilesFromDir(std::string const& baseDir);
bool isDir(std::string const& dir);
bool pathExists(std::string const& path);
bool mkDir(std::string const& path);
bool mkDirRecursive(std::string const& path);
std::string getFileName(std::string const& path);
std::string stripExt(std::string const& path);
std::string getFileExt(std::string const& path);
std::string getFileDirectory(std::string const& filePath);
unsigned long getFileTimestamp(std::string const& path);	// returns timestamp in seconds (since 1970 or something)
bool touchFile(std::string const& path);
bool copyFile(std::string const& source, std::string const& dest);
bool deleteFile(std::string const& path);
uint64_t getFileSize(std::string const& path);

void applyRecursive(std::string const& baseDir, std::function<void(std::string const& filename)> func);

std::string getHomeDirectory();
bool deleteFolderContent(const std::string& folderPath);
std::vector<std::string> readFileLines(std::string const& filePath);
std::string readFileContent(std::string const& filePath, unsigned startIndex, unsigned endIndex);
std::string normalizePath(std::string const& inPath);

/**
 * Use this class to access files (READ / GETSIZE) that have UNICODE characters in their paths or names.
 * This class DOESN'T DEAL with UNICODE contents within the file, it simply returns the binary data read from the file.
 * The file contents are not read until getContents() is called. Once it's called, the data is buffered in memory and
 * successive calls to getContents() will return the same buffer always.
 *
 * !!! The buffer returned by getContents() only exists for as long as this object is alive !!!
 * !!! Make sure not to acess the buffer after destroying the object !!!
*/
class UnicodeFileReader {
public:
	UnicodeFileReader(std::string const& path);
	~UnicodeFileReader();

	uint64_t getSize() const;
	/** Returns null if file size is zero, or a buffer with the file's contents otherwise, in raw binary form. */
	const unsigned char* getContents() const;

private:
	BaseFileReader* pImpl_;
};

} // namespace

#endif /* DIR_H_ */
