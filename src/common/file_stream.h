#pragma once

#include <string>

class file_stream
{
public:
    file_stream(const std::string &file_name, const std::string &open_mode = "wb+")
    {
        file_ = fopen(file_name.c_str(), open_mode.c_str());
    }
    ~file_stream()
    {
        if (file_)
        {
            fclose(file_);
        }
    }

    operator bool()
    {
        return file_ != NULL;
    }

    int seek(size_t pos, int type = SEEK_SET)
    {
        return ::fseek(file_, pos, type);
    }

    void read(char* pch, int size)
    {
        if (!file_)
            throw std::ios_base::failure("file_stream::read : file handle is NULL");
        if (fread(pch, 1, size, file_) != size)
            throw std::ios_base::failure(feof(file_) ? "file_stream::read : end of file" : "file_stream::read : fread failed");
    }

    void write(const char* pch, int size)
    {
        if (!file_)
            throw std::ios_base::failure("file_stream::write : file handle is NULL");
        if (fwrite(pch, 1, size, file_) != size)
            throw std::ios_base::failure("file_stream::write : write failed");
    }

    template<typename T>
    file_stream& operator<<(const T& obj)
    {
        // Serialize to this stream
        if (!file_)
            throw std::ios_base::failure("file_stream::operator<<: file handle is NULL");
        ::serialize(*this, obj);
        return (*this);
    }

    template<typename T>
    file_stream& operator >> (T& obj)
    {
        // Unserialize from this stream
        if (!file_)
            throw std::ios_base::failure("file_stream::operator>>: file handle is NULL");
        ::unserialize(*this, obj);
        return (*this);
    }
private:
    FILE *file_;
};
