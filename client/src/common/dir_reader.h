#ifndef BCUS_COMMON_DIR_READER_H
#define BCUS_COMMON_DIR_READER_H

#include <stdio.h>
#include <string>
#include <vector>
#include "platform.h"

namespace bcus {

class dir_reader {
  public:
    dir_reader(const char* filter = NULL);
    virtual ~dir_reader();
  public:
    bool open(const char* path);
    bool get_first_file_name(char *filename, int max_len = 255);
    bool get_next_file_name(char *filename, int max_len = 255);
  private:
    void read_files(const char*path);
  private:
    std::string path_;
    std::string filter_;
    std::vector<std::string> filenames_;
    unsigned int currentindex_;
};

}
#endif
