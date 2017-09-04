#ifndef BCUS_LOG_FILE_H
#define BCUS_LOG_FILE_H

#include <string>
#include <vector>
#include <stdint.h>

#ifdef LINUX
#define FPOS_SET(fposValue,longValue)   (fposValue).__pos=(longValue)
#define FPOS_GET(fposValue) (fposValue).__pos
#else
#define FPOS_SET(fposValue,longValue)   (fposValue)=(longValue)
#define FPOS_GET(fposValue) (fposValue)
#endif

namespace bcus {

class file {
public:
    file();
    ~file();
    bool           open(const char *filename, const char *mode, bool auto_created = false);
    unsigned int    append(const void *buf, unsigned int len);
    unsigned int    write(const void *buf, unsigned int len);
    unsigned int    write(const char *buf);
    unsigned int    write_format(const char *format, ...);
    unsigned int    read(void *buf, unsigned int max_len, uint64_t pos);
    unsigned int    read(void *buf, unsigned int max_len);
    unsigned int    read_line(char *buf, unsigned int max_len);
    void           flush();
    void           close();
    void           setpos(uint64_t pos);
    bool           is_ready();
    bool           is_end();
    bool           is_error();
    void           clear();

    uint64_t         size();
    uint64_t         getpos();
    static void      remove(const char *filename);
    static uint64_t  size(const char *filename);

    FILE *get() {return file_;}
    const char *name() const { return filename_; }
private:
    FILE *file_;
    char filename_[512];
};

}

#endif

