#ifdef __WIN32__
	#define UNICODE
	#include <windows.h>
#endif

#include "filesystem.h"
#include "strbld.h"
#include "wstrconv.h"

#include <string>
#include <stdexcept>
#include <thread>
#include <fcntl.h>

#ifndef WIN32_MSVC
#include <unistd.h>
#include <sys/stat.h>
#endif

class BaseFileReader {
public:
	virtual ~BaseFileReader() {
		if (buffer_) {
			free(buffer_);
		}
	}

	uint64_t getSize() const {
		return size_;
	}

	const unsigned char* getBuffer() {
		if (!hasContents_) {
			allocBuffer();
			readFile(buffer_);
			hasContents_ = true;
		}
		return const_cast<const unsigned char*>(buffer_);
	}

protected:
	uint64_t size_ = 0;
	virtual void readFile(unsigned char* buffer) = 0;

private:
	bool hasContents_ = false;
	unsigned char* buffer_ = nullptr;
	void allocBuffer() {
		buffer_ = (unsigned char*)malloc(size_);
	}
};

#ifdef __WIN32__

class WindowsFileReader : public BaseFileReader {
public:
	WindowsFileReader(
		std::string const& path,
		filesystem::FileAccessOptions options = filesystem::FileAccessOptions()
	): path_(path) {
		char absolutePath[2048];
		if (path.size() < MAX_PATH) {
			DWORD res = GetFullPathNameA(path.c_str(), sizeof(absolutePath), absolutePath, NULL);
			if (res == 0) {
				throw std::runtime_error(std::string("Unable to convert path to full path: ") + path);
			}
		} else {
			// if the provided path is longer, we have no choice but to assume it's a full path.
			// if the assumption proves wrong, the file will not be accessible
			// see this for more info: https://stackoverflow.com/questions/28137284/does-getfullpathname-work-with-relative-paths-longer-than-max-path
			strncpy(absolutePath, path.c_str(), sizeof(absolutePath));
		}
		auto wPath = str2Wstr(std::string("\\\\?\\") + absolutePath);
		do {
			file_ = CreateFile(wPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (file_ == INVALID_HANDLE_VALUE) {
				if (options.numberOfRetries > 0) {
					std::this_thread::sleep_for(std::chrono::seconds(options.sleepBetweenRetriesSec));
				}
				options.numberOfRetries--;
			}
		} while (file_ == INVALID_HANDLE_VALUE && options.numberOfRetries >= 0);
		if (file_ == INVALID_HANDLE_VALUE) {
			throw std::runtime_error(std::string("Unable to open file ") + path);
		}
		DWORD sizeHigh = 0;
		DWORD size = GetFileSize(file_, &sizeHigh);
		size_ = (((uint64_t)sizeHigh) << 32) | size;
	}

	virtual ~WindowsFileReader() override {
		CloseHandle(file_);
		file_ = INVALID_HANDLE_VALUE;
	}

protected:
	void readFile(unsigned char* buffer) override {
		if (size_ > UINT32_MAX) {
			throw std::runtime_error(std::string("Reading files larger than 4GB is not supported on Windows") + path_);
		}
		DWORD bytesRead;
		BOOL ok = ReadFile(file_, buffer, (DWORD)size_, &bytesRead, 0);
		if (!ok) {
			throw std::runtime_error(
				strbld() << "Failed to read from "
					<< path_ << "(errno: " << GetLastError()
					<< ") - Read " << bytesRead << " out of " << size_
			);
		}
	}

private:
	HANDLE file_ = INVALID_HANDLE_VALUE;
	std::string path_;
};

#else

class LinuxFileReader : public BaseFileReader {
public:
	LinuxFileReader(
		std::string const& path,
		filesystem::FileAccessOptions options = filesystem::FileAccessOptions()
	): path_(path) {
		do {
			file_ = fopen(path.c_str(), "rb");
			if (!file_) {
				if (options.numberOfRetries > 0) {
					std::this_thread::sleep_for(std::chrono::seconds(options.sleepBetweenRetriesSec));
				}
				options.numberOfRetries--;
			}
		} while (!file_ && options.numberOfRetries >= 0);
		if (!file_) {
			throw std::runtime_error(std::string("Unable to open file ") + path);
		}
		int fs = open(path.c_str(), O_RDONLY, 0);
		if (fs < 0) {
			throw std::runtime_error(strbld() << errno << ": Could not open file \"" << path <<"\"");
		}
		struct stat stat_source;
		if (fstat(fs, &stat_source) < 0) {
			throw std::runtime_error(strbld() << errno << ": Could not stat() file \"" << path << "\"");
		}
		close(fs);
		size_ = stat_source.st_size;
	}

	virtual ~LinuxFileReader() override {
		fclose(file_);
		file_ = nullptr;
	}

protected:
	void readFile(unsigned char* buffer) override {
		size_t readCount = fread(buffer, 1, size_, file_);
		if (readCount != size_) {
			throw std::runtime_error(
				strbld() << "Failed to read from "
					<< path_ << " - Read " << readCount << " out of " << size_
			);
		}
	}

private:
	FILE *file_ = nullptr;
	std::string path_;
};

#endif

filesystem::UnicodeFileReader::UnicodeFileReader(
	std::string const& path,
	filesystem::FileAccessOptions options /* = filesystem::FileAccessOptions() */
) {
#ifdef __WIN32__
	pImpl_ = new WindowsFileReader(path, options);
#else
	pImpl_ = new LinuxFileReader(path, options);
#endif
}

uint64_t filesystem::UnicodeFileReader::getSize() const {
	return pImpl_->getSize();
}

/** Returns null if file size is zero. */
const unsigned char* filesystem::UnicodeFileReader::getContents() const {
	return pImpl_->getBuffer();
}

filesystem::UnicodeFileReader::~UnicodeFileReader() {
	delete pImpl_;
	pImpl_ = nullptr;
}
