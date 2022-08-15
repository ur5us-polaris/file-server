#pragma once
#include <string>
#include <Windows.h>
#include <list>
#include <mutex>
#include <filesystem>

#define DEFAULT_ROOT "."
#define LOCK_EXT ".lock"
#define SECONDS_WAIT_ON_LOCKED 1
#define MAX_FILE_SIZE 0xffffffff


typedef unsigned int userid_t;
typedef char* buffer_t;
typedef unsigned long long start_pos_t;

enum class FileOpResult { success, success_not_eof, not_exist, path_invalid, unknown_fail };
enum class PathStatus { exists, not_exist, bad_path };


class WindowsFileManager
{
private:
	static std::list<std::filesystem::path> lockedFiles;
	static std::mutex lockedFilesMutex;
	std::filesystem::path root;
	bool isPathSafe(std::filesystem::path, std::filesystem::path);
	std::filesystem::path getFolderPathForUser(userid_t);
	PathStatus getPathStatus(std::filesystem::path, bool is_dir = false);

	bool lockFile(std::filesystem::path);
	bool unlockFile(std::filesystem::path);
	bool isFileLocked(std::filesystem::path);

public:
	WindowsFileManager() : WindowsFileManager(DEFAULT_ROOT) {};
	WindowsFileManager(std::string);
	std::filesystem::path getRoot();

	FileOpResult saveFile(userid_t, std::string, size_t, buffer_t);
	//FileOpResult getFile(userid_t, std::string, size_t, buffer_t, start_pos_t start_pos = 0);
	FileOpResult getFile(userid_t, std::string, size_t*, buffer_t*);
	FileOpResult deleteFile(userid_t, std::string);
	FileOpResult dirList(userid_t, std::string*);
};

