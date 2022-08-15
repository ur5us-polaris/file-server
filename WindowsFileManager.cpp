#include "WindowsFileManager.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <stdexcept>
#include <fstream>
#include <list>
#include <algorithm>
#include <mutex>
#include <filesystem>


#pragma region static_variables_declerations
std::list<std::filesystem::path> WindowsFileManager::lockedFiles;
std::mutex WindowsFileManager::lockedFilesMutex;
#pragma endregion


WindowsFileManager::WindowsFileManager(std::string root)
{
    // char abs_path[MAX_PATH] = { 0 };
    // GetFullPathNameA((pwd() + "\\" + root).c_str(), MAX_PATH, abs_path, NULL);
    // this->root = std::string(abs_path);
    std::cout << std::filesystem::current_path().string() << std::endl;
    this->root = std::filesystem::absolute(std::filesystem::current_path() / root);
}

FileOpResult WindowsFileManager::saveFile(userid_t user_id, std::string file_name, size_t file_size, buffer_t data_in)
{
    std::filesystem::path user_dir_full = getFolderPathForUser(user_id);
    std::filesystem::path file_path_full = user_dir_full / file_name;

    // Wait if file is locked
    while (isFileLocked(file_path_full) == true)
    {
        std::cout << "File " << file_path_full << " is locked, waiting " << std::to_string(SECONDS_WAIT_ON_LOCKED) << " seconds..." << std::endl;
        Sleep(SECONDS_WAIT_ON_LOCKED);
    }
    if (this->isPathSafe(file_path_full, user_dir_full) == false)
    {
        std::cout << "Cannot save file: path '" + file_path_full.string() + "' is outside of root folder" << std::endl;
        return FileOpResult::path_invalid;
    }
    PathStatus user_folder_status = getPathStatus(user_dir_full, true);
    if (user_folder_status == PathStatus::bad_path)
    {
        std::cout << "Cannot save file: path '" + user_dir_full.string() + "' is invalid" << std::endl;
        return FileOpResult::path_invalid;
    }
    else if (user_folder_status == PathStatus::not_exist)
    {
        std::cout << "Creating user folder for user id: " << std::to_string(user_id) << std::endl;
        std::filesystem::create_directory(user_dir_full);
        //int success = CreateDirectoryA(user_dir_full.c_str(), NULL);
    }


    lockFile(file_path_full);
    std::ofstream fd(user_dir_full / file_name, std::ios::binary);
    if (!fd)
    {
        throw std::invalid_argument("Cannot save file: failed to open file at path '" + (user_dir_full / file_name).string() + "'");
        unlockFile(file_path_full);
    }
    fd.write(data_in, file_size);
    std::cout << "File saved to path " << user_dir_full << file_name << std::endl;
    fd.close();
    unlockFile(file_path_full);
    return FileOpResult::success;
}

//FileOpResult WindowsFileManager::getFile(userid_t user_id, std::string file_name, size_t data_out_size, buffer_t data_out, start_pos_t start_pos)
//{
//    std::string user_dir_full = getFolderPathForUser(user_id);
//    std::string file_path_full = user_dir_full + file_name;
//    while (isFileLocked(file_path_full) == true)
//    {
//        std::cout << "File " << file_path_full << " is locked, waiting " << std::to_string(SECONDS_WAIT_ON_LOCKED) << " seconds..." << std::endl;
//        Sleep(SECONDS_WAIT_ON_LOCKED);
//    }
//
//    if (this->isPathSafe(file_path_full, user_dir_full) == false)
//    {
//        std::cout << "Cannot read file: path '" + file_path_full + "' is outside of root folder" << std::endl;
//        return FileOpResult::path_invalid;
//    }
//    
//    PathStatus file_status = getPathStatus(file_path_full, false);
//    if (file_status == PathStatus::bad_path)
//    {
//        std::cout << "Cannot read file: path '" + file_path_full + "' is invalid" << std::endl;
//        return FileOpResult::path_invalid;
//    }
//    else if (file_status == PathStatus::not_exist)
//    {
//        std::cout << "Cannot read file: path '" + file_path_full + "' not found" << std::endl;
//        return FileOpResult::not_exist;
//    }
//    lockFile(file_path_full);
//    std::ifstream fd(file_path_full, std::ios::binary);
//    fd.unsetf(std::ios::skipws);
//    if (!fd)
//    {
//        std::cout << "Cannot read file: failed to open file at path '" + file_path_full + "'" << std::endl;
//        unlockFile(file_path_full);
//        return FileOpResult::path_invalid;
//    }
//    fd.seekg(start_pos);
//    std::cout << "Reading byte" << std::to_string(start_pos) << "-" << std::to_string(start_pos + data_out_size) << " from " << file_path_full << "..." << std::endl;
//    fd.read(data_out, data_out_size);
//    fd.close();
//    if (fd.eof() == true)
//    {
//        std::cout << "File " << file_path_full << " read succesfully" << std::endl;
//        unlockFile(file_path_full);
//        return FileOpResult::success;
//    }
//    std::cout << "File " << file_path_full << " read partially" << std::endl;
//    unlockFile(file_path_full);
//    return FileOpResult::success_not_eof;
//}

FileOpResult WindowsFileManager::getFile(userid_t user_id, std::string file_name, size_t* data_size_out, buffer_t* data_out)
{
    std::filesystem::path user_dir_full = getFolderPathForUser(user_id);
    std::filesystem::path file_path_full = user_dir_full / file_name;
    while (isFileLocked(file_path_full) == true)
    {
        std::cout << "File " << file_path_full << " is locked, waiting " << std::to_string(SECONDS_WAIT_ON_LOCKED) << " seconds..." << std::endl;
        Sleep(SECONDS_WAIT_ON_LOCKED);
    }

    if (this->isPathSafe(file_path_full, user_dir_full) == false)
    {
        std::cout << "Cannot read file: path '" + file_path_full.string() + "' is outside of root folder" << std::endl;
        return FileOpResult::path_invalid;
    }

    PathStatus file_status = getPathStatus(file_path_full, false);
    if (file_status == PathStatus::bad_path)
    {
        std::cout << "Cannot read file: path '" + file_path_full.string() + "' is invalid" << std::endl;
        return FileOpResult::path_invalid;
    }
    else if (file_status == PathStatus::not_exist)
    {
        std::cout << "Cannot read file: path '" + file_path_full.string() + "' not found" << std::endl;
        return FileOpResult::not_exist;
    }
    lockFile(file_path_full);
    std::ifstream fd(file_path_full, std::ios::binary);
    if (!fd)
    {
        std::cout << "Cannot read file: failed to open file at path '" + file_path_full.string() + "'" << std::endl;
        unlockFile(file_path_full);
        return FileOpResult::path_invalid;
    }
    // get file size
    *data_size_out = std::filesystem::file_size(file_path_full);
    std::cout << "File " << file_path_full << " is of size " << std::to_string(*data_size_out) << std::endl;
    *data_out = new char[*data_size_out]();
    fd.read(*data_out, *data_size_out);
    fd.close();
    std::cout << "File " << file_path_full << " read succesfully" << std::endl;
    unlockFile(file_path_full);
    return FileOpResult::success;
}

FileOpResult WindowsFileManager::deleteFile(userid_t user_id, std::string file_name)
{
    std::filesystem::path user_dir_full = getFolderPathForUser(user_id);
    std::filesystem::path file_path_full = user_dir_full / file_name;
    // Wait if file is locked
    while (isFileLocked(file_path_full) == true)
    {
        std::cout << "File " << file_path_full << " is locked, waiting " << std::to_string(SECONDS_WAIT_ON_LOCKED) << " seconds..." << std::endl;
        Sleep(SECONDS_WAIT_ON_LOCKED);
    }
    if (this->isPathSafe(file_path_full, user_dir_full) == false)
    {
        std::cout << "Cannot delete file: path '" + file_path_full.string() + "' is outside of root folder" << std::endl;
        return FileOpResult::path_invalid;
    }
    
    PathStatus file_status = getPathStatus(file_path_full, false);
    if (file_status == PathStatus::bad_path)
    {
        std::cout << "Cannot delete file: path '" + file_path_full.string() + "' is invalid" << std::endl;
        return FileOpResult::path_invalid;
    }
    else if (file_status == PathStatus::not_exist)
    {
        std::cout << "Cannot delete file: path '" + file_path_full.string() + "' not found" << std::endl;
        return FileOpResult::not_exist;
    }
    lockFile(file_path_full);
    int result = std::filesystem::remove(file_path_full);
    std::cout << "File " << file_path_full << " deleted succesfully" << std::endl;
    unlockFile(file_path_full);
    FileOpResult::success;
}

FileOpResult WindowsFileManager::dirList(userid_t user_id, std::string* files_out)
{
    std::filesystem::path user_dir_full = getFolderPathForUser(user_id);
    if (this->isPathSafe(user_dir_full, user_dir_full) == false)
    {
        std::cout << "Cannot get file list: path '" + user_dir_full.string() + "' is outside of root folder" << std::endl;
        return FileOpResult::path_invalid;
    }
    PathStatus user_folder_status = getPathStatus(user_dir_full, true);
    if (user_folder_status == PathStatus::bad_path)
    {
        std::cout << "Cannot get file list: path '" + user_dir_full.string() + "' is invalid" << std::endl;
        return FileOpResult::path_invalid;
    }
    else if (user_folder_status == PathStatus::not_exist)
    {
        std::cout << "Cannot get file list: path '" + user_dir_full.string() + "' does not exist" << std::endl;
        return FileOpResult::not_exist;
    }

    files_out->clear();
    for (auto const& dir_entry : std::filesystem::directory_iterator(user_dir_full))
    {
        *files_out += dir_entry.path().filename().string() + "\n";
    }
    //std::string search_path = user_dir_full.string() + "*";
    //WIN32_FIND_DATAA fd;
    //HANDLE hFind = ::FindFirstFileA(search_path.c_str(), &fd);
    //if (hFind != INVALID_HANDLE_VALUE) {
    //    do {
    //        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
    //            files_out->push_back(fd.cFileName);
    //        }
    //    } while (::FindNextFileA(hFind, &fd));
    //    ::FindClose(hFind);
    //}
    return FileOpResult::success;
}

PathStatus WindowsFileManager::getPathStatus(std::filesystem::path file_path, bool is_dir)
{  
    if (std::filesystem::exists(file_path) == false)
    {
        // No file exists in path
        return PathStatus::not_exist;
    }

    if (std::filesystem::is_directory(file_path))
    {
        // Path is a directory
        if (is_dir == true)
            return PathStatus::exists;
        return PathStatus::bad_path;
    }
    // Path is a file
    if (is_dir == true)
        return PathStatus::bad_path;
    return PathStatus::exists;

    //DWORD ftyp = GetFileAttributesA(file_path.c_str());
    //if (ftyp == INVALID_FILE_ATTRIBUTES)
    //{
    //    DWORD dwError = GetLastError();
    //    if (dwError == ERROR_FILE_NOT_FOUND || dwError == ERROR_PATH_NOT_FOUND)
    //    {
    //        return PathStatus::not_exist;
    //    }
    //    else
    //    {
    //        // Might be permissions issue
    //        return PathStatus::bad_path;
    //    }
    //}
    //else
    //{
    //    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
    //    {
    //        // this is a directory
    //        if (is_dir == true)
    //            return PathStatus::exists;
    //        else
    //            return PathStatus::bad_path;
    //    }
    //    else
    //    {
    //        // this is an ordinary file
    //        if (is_dir == true)
    //            return PathStatus::bad_path;
    //        else
    //            return PathStatus::exists;
    //    }
    //}
}

std::filesystem::path WindowsFileManager::getRoot()
{
    return this->root;
}

bool WindowsFileManager::isPathSafe(std::filesystem::path path, std::filesystem::path safe_path)
{
    std::filesystem::path abs_path = std::filesystem::absolute(path);
    return (abs_path.parent_path().compare(safe_path) == 0) || (safe_path.compare(path) == 0);
}

std::filesystem::path WindowsFileManager::getFolderPathForUser(userid_t user_id)
{
    return this->root / std::to_string(user_id);
}

bool WindowsFileManager::isFileLocked(std::filesystem::path file_path)
{
    return std::find(WindowsFileManager::lockedFiles.begin(), WindowsFileManager::lockedFiles.end(), file_path) != WindowsFileManager::lockedFiles.end();
}

bool WindowsFileManager::lockFile(std::filesystem::path file_path)
{
    WindowsFileManager::lockedFilesMutex.lock();
    if (WindowsFileManager::isFileLocked(file_path) == true)
    {
        WindowsFileManager::lockedFilesMutex.unlock();
        return false;
    }
    WindowsFileManager::lockedFiles.push_back(file_path);
    WindowsFileManager::lockedFilesMutex.unlock();
    return true;
}

bool WindowsFileManager::unlockFile(std::filesystem::path file_path)
{
    WindowsFileManager::lockedFilesMutex.lock();
    if (WindowsFileManager::isFileLocked(file_path) == false)
    {
        WindowsFileManager::lockedFilesMutex.unlock();
        return true;
    }
    WindowsFileManager::lockedFiles.remove(file_path);
    WindowsFileManager::lockedFilesMutex.unlock();
    return true;
}
