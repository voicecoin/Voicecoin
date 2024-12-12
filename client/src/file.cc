#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "platform.h"
#include <stdexcept>
#include "error_code.h"

#ifndef BCUS_MAX_FILE_PATH_LEN
#define BCUS_MAX_FILE_PATH_LEN 256
#endif

namespace bcus {

static inline char *replace(char from, char to, const char *in, char *out) {
    const char *end = in + strlen(in);
    for(const char *p = in; p < end; ++p) {
        *(out + (p - in)) = (*p == from) ? to : *p;
    }
    return out;
}
static inline char* split_file_name(const char *szpath, char *szdir, char *szfilename)
{
    char szbuf[BCUS_MAX_FILE_PATH_LEN+1] = {0};
    strncpy(szbuf, szpath, BCUS_MAX_FILE_PATH_LEN);
    char *p = strrchr(szbuf, BCUS_LOG_DIRSPLITOR);
    if (p) {
        if (szfilename)
            strcpy(szfilename, p + 1);
        *p = 0;
        if (szdir)
            strcpy(szdir,szbuf);
        return szfilename;
    }
    return NULL;
}
static inline bool file_exist(const char* szfile)
{
    return access(szfile,0) != -1;
}
static int make_directory(const char* szpath)
{
    if (file_exist(szpath))
        return 0;
    char szparentdir[BCUS_MAX_FILE_PATH_LEN+1] = {0};
    split_file_name(szpath,szparentdir,NULL);
    if (szparentdir[0]!=0 && !file_exist(szparentdir)) {
        int error = make_directory(szparentdir);
        if (0 != error) {
            return error;
        }
    }
    #ifdef WIN32
    if (mkdir(szpath) != 0) { return GetLastError(); }
    #else
    if (0 != mkdir(szpath, 0755)) {return errno; }
    #endif

    return 0;
}


file::file() : file_(NULL)
{
    memset(filename_, 0, sizeof(filename_));
}
file::~file()
{
}
bool file::open(const char *filename, const char *mode, bool auto_created)
{
#ifdef WIN32
    filename = replace('/', '\\', filename, filename_);
#else
    strcpy(filename_, filename);
#endif

    file_ = fopen(filename, mode);
    if (NULL != file_) {
        return true;
    }
    if (!auto_created && NULL == file_) {
        return false;
    }

    char szbuf[BCUS_MAX_FILE_PATH_LEN+1];
    strncpy(szbuf, filename, BCUS_MAX_FILE_PATH_LEN);
    char *p = strrchr(szbuf, BCUS_LOG_DIRSPLITOR);
    if (p != NULL) {
        *p = '\0';
        int code = make_directory(szbuf);
        if (code != 0) {
            fprintf(stdout, "make_directory[%s] failed, error[%d]\n", szbuf, code);
        }
    }
    file_ = fopen(filename, mode);
    return NULL != file_;
}
void file::close()
{
    if (file_) {
        fclose(file_);
        file_ = NULL;
    }
}
unsigned int file::append(const void *buf, unsigned int len)
{
    if (!file_)
        throw std::runtime_error("file havn't be opened");

    ::fseek(file_, 0, SEEK_END);
    return write(buf, len);
}
unsigned int  file::write(const char *buf)
{
    return write(buf, strlen(buf));
}
unsigned int file::write(const void *buf, unsigned int len)
{
    if (!file_)
        throw std::runtime_error("file havn't be opened");
    return fwrite(buf, 1, len, file_);
}
unsigned int file::write_format(const char *format, ...)
{
    if (!file_)
        throw std::runtime_error("file havn't be opened");
    va_list va;
    va_start( va, format );
    int len = vfprintf( file_, format, va );
    va_end( va );
    return len > 0 ? len : 0;
}
void file::flush()
{
    if (!file_)
        throw std::runtime_error("file havn't be opened");
    fflush(file_);
}
unsigned int file::read(void *buf, unsigned int max_len, uint64_t pos)
{
    setpos(pos);
    return file::read(buf, max_len);
}
unsigned int  file::read(void *buf, unsigned int max_len)
{
    if (!file_)
        throw std::runtime_error("file havn't be opened");
    return fread(buf, 1, max_len, file_);
}
unsigned int file::read_line(char *buf, unsigned int max_len)
{
    if (!file_)
        return 0;

    *buf = 0;
    char format[32] = {0};
    snprintf(format, sizeof(format), "%%%u[^\n]\n", max_len);   //  %32[^\n]\n
    //fprintf(stdout, "file::%s, format[%s]\n", __FUNCTION__, format);
    fscanf(file_, format, buf);
    return strlen(buf);
}
void file::clear()
{
    if (!file_)
        throw std::runtime_error("file havn't be opened");
    ::ftruncate(::fileno(file_), 0);
    ::fseek(file_, 0, SEEK_SET);
    ::fflush(file_);
}
uint64_t file::getpos()
{
    fpos_t current;
    fgetpos(file_, &current);
    uint64_t pos = FPOS_GET(current);
    return pos;
}
void file::setpos(uint64_t pos)
{
    fpos_t current;
    FPOS_SET(current, pos);
    fsetpos(file_, &current);
}
bool file::is_ready()
{
    return file_ != NULL;
}
bool file::is_end() {
    return feof(file_) != 0;
}
bool file::is_error()
{
    return ferror(file_) != 0;
}
uint64_t file::size()
{
    return bcus::file::size(filename_);
}
uint64_t file::size(const char *filename)
{
#ifdef WIN32
    char tmp[512] = {0};
    filename = replace('/', '\\', filename, tmp);
#endif

    struct stat statbuf;
    if (0 != stat(filename, &statbuf )) {
        fprintf(stdout, "code[%d], msg[%s]\n", bcus::error::get_last_error(),
            bcus::error::get_error_msg(bcus::error::get_last_error()));
        return 0;
    }

    return statbuf.st_size;
}
void file::remove(const char *filename)
{
#ifdef WIN32
    char tmp[512] = {0};
    filename = replace('/', '\\', filename, tmp);
#endif
    if (!file_exist(filename))
        return;

    unlink(filename);
}

}
