#include <boost/asio.hpp>
#include "dir_reader.h"
#include "loghelper.h"
#include <cstring>
#include <filesystem>

namespace bcus {

static inline char *replace(char from, char to, const char *in, char *out) {
    const char *end = in + strlen(in);
    for(const char *p = in; p < end; ++p) {
        *(out + (p - in)) = (*p == from) ? to : *p;
    }
    return out;
}
dir_reader::dir_reader(const char* filter) : currentindex_(0)
{
    filter_ = (filter == NULL || filter[0] == 0) ? "*" : filter;
}
dir_reader::~dir_reader()
{
}
bool dir_reader::get_next_file_name(char *filename, int max_len)
{
    if (filenames_.empty() || currentindex_ >= filenames_.size())
        return false;
    snprintf(filename, max_len, "%s", filenames_.at(currentindex_++).c_str());
    return true;
}
bool dir_reader::get_first_file_name(char *filename, int max_len)
{
    if (filenames_.empty())
        return false;
    currentindex_ = 0;
    snprintf(filename, max_len, "%s", filenames_.at(currentindex_++).c_str());
    return true;
}
bool dir_reader::open(const char* path)
{
    if (access(path,0) < 0) {
        return false;
    }
    read_files(path);
    return true;
}

#if defined(WIN32) || defined(WINDOWS)
void dir_reader::read_files(const char *path)
{
    std::string filepath = path;

#ifdef WIN32
    char path_new[256] = {0};
    replace('/', '\\', path, path_new);
    strcat(path_new, "\\*.*");
    path = path_new;
#endif

    WIN32_FIND_DATA fd;


    std::wstring wide_path = std::filesystem::path(path).wstring(); // C++17 中的 std::filesystem
    wide_path += L"\\*.*";
    HANDLE handle = FindFirstFileW(wide_path.c_str(), &fd);


    //HANDLE handle = FindFirstFile(path, &fd);
    if (handle == INVALID_HANDLE_VALUE ) {
        XLOG(XLOG_WARNING,"dir_reader::%s, open dir error. path[%s]\n", __FUNCTION__, path);
        return;
    }

    do {
        if (0 == strcmp((const char *)(fd.cFileName),".") || 0 == strcmp((const char *)(fd.cFileName),"..") ) {
            continue;
        }
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            read_files((filepath + "\\" + (const char *)(fd.cFileName)).c_str());
            continue;
        }
        filenames_.push_back(filepath + "\\" + (const char *)(fd.cFileName));
    }while(FindNextFile(handle, &fd));
}

#else
void dir_reader::read_files(const char *path)
{
    struct dirent *pdirent;
    struct stat statbuf;
    DIR *dir = opendir(path);
    if (NULL == dir) {
        XLOG(XLOG_WARNING,"dir_reader::%s, open dir error. path[%s]\n", __FUNCTION__, path);
        return;
    }

    while (NULL != (pdirent = readdir(dir))) {
        if ((0 == strcmp(pdirent->d_name, ".")) || (0 == strcmp(pdirent->d_name, "..")))
            continue;

        std::string filepath = path;
        if (filepath.at(filepath.length()-1) != '/') {
            filepath.append("/");
        }
        filepath.append(pdirent->d_name);
        if(0 > stat(filepath.c_str(), &statbuf)) {
            XLOG(XLOG_WARNING, "dir_reader::%s.   stat file error: %s\n", __FUNCTION__, filepath.c_str());
            continue;
        }

        if(S_ISDIR(statbuf.st_mode)) {
            read_files(filepath.c_str());
        } else {
            //XLOG(XLOG_TRACE,"dir_reader::%s.   fileName[%s]\n",__FUNCTION__, filepath.c_str());
            if (filter_.empty() || 0 == fnmatch(filter_.c_str(), pdirent->d_name, FNM_PATHNAME|FNM_PERIOD)) {
                //XLOG(XLOG_TRACE,"dir_reader::%s.   matched fileName[%s]\n",__FUNCTION__, filepath.c_str());
                filenames_.push_back(filepath);
            }
        }
    }
    closedir(dir);
}
#endif

}
