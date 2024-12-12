#ifndef BCUS_SLICE_H
#define BCUS_SLICE_H

// slice is a simple structure containing a pointer into some external
// storage and a size.    The user of a slice must ensure that the slice
// is not used after the corresponding external storage has been
// deallocated.
//
// Multiple threads can invoke const methods on a slice without
// external synchronization, but if any of the threads may call a
// non-const method, all threads accessing the same slice must use
// external synchronization.

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <string>
#include <vector>

namespace bcus {

class slice {
 public:
    // Create an empty slice.
    slice() : data_(""), size_(0) {}

    // Create a slice that refers to d[0,n-1].
    slice(const void* d, size_t n) : data_((const char *)d), size_(n) {}

    // Create a slice that refers to the contents of "s"
    slice(const std::string& s) : data_(s.data()), size_(s.size()) {}

    // Create a slice that refers to s[0,strlen(s)-1]
    slice(const char* s) : data_(s), size_(strlen(s)) {}

    // Return a pointer to the beginning of the referenced data
    const char* data() const { return data_; }
    const char* c_str() const { return data_; }

    // Return the length (in bytes) of the referenced data
    size_t size() const { return size_; }
    size_t length() const { return size_; }

    // Return true iff the length of the referenced data is zero
    bool empty() const { return size_ == 0; }

    // Return the ith byte in the referenced data.
    char operator[](size_t n) const {
        assert(n < size());
        return data_[n];
    }

    // Change this slice to refer to an empty array
    void clear() { data_ = ""; size_ = 0; }

    // Drop the first "n" bytes from this slice.
    void remove_prefix(size_t n) {
        assert(n <= size());
        data_ += n;
        size_ -= n;
    }

    // Return a string that contains the copy of the referenced data.
    std::string str() const { return std::string(data_, size_); }

    // Three-way comparison.    Returns value:
    //     <    0 iff "*this" <    "b",
    //     == 0 iff "*this" == "b",
    //     >    0 iff "*this" >    "b"
    int compare(const slice& b) const;

    // Return true iff "x" is a prefix of "*this"
    bool starts_with(const slice& x) const {
        return ((size_ >= x.size_) &&
                        (memcmp(data_, x.data_, x.size_) == 0));
    }

    slice &trim(bool left=true, bool right=true) {
        const char *delims = " \t\r\n";
        if (left) {
            while (size_) {
                if (::strchr(delims, *data_) == NULL) break;
                data_++;
                --size_;
            }
        }
        if (right) {
            while (size_) {
                size_t i = size_ - 1;
                if (::strchr(delims, data_[i]) == NULL) break;
                size_ = i;
            }
        }
        return *this;
    }

    int find(char ch) const {
        if (size_ == 0) return -1;
        char *ptr = (char *)::memchr(data_, ch, size_);
        return (ptr ? ptr - data_ : -1);
    }

    int find(const slice& sub) const {
        if (sub.empty()) return -1;
        char ch = sub.data_[0];
        if (sub.size_ == 1) return this->find(ch);

        const char *ptr = data_;
        const char *end = data_ + size_;
        for (;;++ptr) {
            if (end < ptr + sub.size_) return -1;
            ptr = (char *)::memchr(ptr, ch, end - ptr);
            if (!ptr) return -1;
            if (end < ptr + sub.size_) return -1;
            if (::memcmp(ptr, sub.data_, sub.size_) == 0) return ptr - data_;
        }
    }

 private:
    const char* data_;
    size_t size_;
};

inline int slice::compare(const slice& b) const {
    const int min_len = (size_ < b.size_) ? size_ : b.size_;
    int r = memcmp(data_, b.data_, min_len);
    if (r == 0) {
        if (size_ < b.size_) r = -1;
        else if (size_ > b.size_) r = +1;
    }
    return r;
}

/// helper for stream-style output
template<class Stream>
Stream& operator << (Stream &os, const bcus::slice &sc) {
    if (sc.size() > 0) os.write(sc.c_str(), sc.size());
    return os;
}

inline bool operator==(const bcus::slice& x, const bcus::slice& y) {
    return ((x.size() == y.size()) &&
                    (memcmp(x.c_str(), y.c_str(), x.size()) == 0));
}

inline bool operator!=(const bcus::slice& x, const bcus::slice& y) {
    return !(x == y);
}
inline bool operator<(const bcus::slice& x, const bcus::slice& y) {
    if (x.length() == y.length()) {
        return strncmp(x.c_str(), y.c_str(), x.length()) < 0;
    } else if(x.length() < y.length()) {
        int r = strncmp(x.c_str(), y.c_str(), x.length());
        return r == 0 ? false : r < 0;
    } else {
        int r = strncmp(x.c_str(), y.c_str(), y.length());
        return r == 0 ? true : r < 0;
    }
}

}


#endif
