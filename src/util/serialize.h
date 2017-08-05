#ifndef BCUS_SERIALIZE_H
#define BCUS_SERIALIZE_H

#include <algorithm>
#include <assert.h>
#include <ios>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <stdint.h>
#include <string>
#include <string.h>
#include <utility>
#include <vector>
#include "endian.h"

static const unsigned int MAX_SIZE = 0x02000000;

// struct deserialize_type {};
// constexpr deserialize_type deserialize{};

#define READWRITE(obj)          (::ser_read_write(s, (obj), ser_action))
#define READWRITEMANY(...)      (::ser_read_write_many(s, ser_action, __VA_ARGS__))

#define FLATDATA(obj) REF(flat_data((char *)&(obj), (char *)&(obj) + sizeof(obj)))
#define VARINT(obj) REF(var_int_wrap(REF(obj)))

#define ADD_SERIALIZE_METHODS                                         \
    template<typename Stream>                                         \
    void serialize(Stream &s) const {                                 \
        NCONST_PTR(this)->serialization(s, ser_action_serialize());   \
    }                                                                 \
    template<typename Stream>                                         \
    void unserialize(Stream &s) {                                     \
        serialization(s, ser_action_unserialize());                   \
    }

template<typename T>
inline T &REF(const T &val)
{
    return const_cast<T &>(val);
}

template<typename T>
inline T *NCONST_PTR(const T *val)
{
    return const_cast<T *>(val);
}

template<typename Stream> inline void ser_writedata8(Stream &s, uint8_t obj)
{
    s.write((char *)&obj, 1);
}
template<typename Stream> inline void ser_writedata16(Stream &s, uint16_t obj)
{
    obj = endian_swap(obj);
    s.write((char *)&obj, 2);
}
template<typename Stream> inline void ser_writedata32(Stream &s, uint32_t obj)
{
    obj = endian_swap(obj);
    s.write((char *)&obj, 4);
}
template<typename Stream> inline void ser_writedata64(Stream &s, uint64_t obj)
{
    obj = endian_swap(obj);
    s.write((char *)&obj, 8);
}
template<typename Stream> inline uint8_t ser_readdata8(Stream &s)
{
    uint8_t obj;
    s.read((char *)&obj, 1);
    return obj;
}
template<typename Stream> inline uint16_t ser_readdata16(Stream &s)
{
    uint16_t obj;
    s.read((char *)&obj, 2);
    return endian_swap(obj);
}
template<typename Stream> inline uint32_t ser_readdata32(Stream &s)
{
    uint32_t obj;
    s.read((char *)&obj, 4);
    return endian_swap(obj);
}
template<typename Stream> inline uint64_t ser_readdata64(Stream &s)
{
    uint64_t obj;
    s.read((char *)&obj, 8);
    return endian_swap(obj);
}
inline uint64_t ser_double_to_uint64(double x)
{
    union { double x; uint64_t y; } tmp;
    tmp.x = x;
    return tmp.y;
}
inline uint32_t ser_float_to_uint32(float x)
{
    union { float x; uint32_t y; } tmp;
    tmp.x = x;
    return tmp.y;
}
inline double ser_uint64_to_double(uint64_t y)
{
    union { double x; uint64_t y; } tmp;
    tmp.y = y;
    return tmp.x;
}
inline float ser_uint32_to_float(uint32_t y)
{
    union { float x; uint32_t y; } tmp;
    tmp.y = y;
    return tmp.x;
}

template<typename Stream> inline void serialize(Stream& s, char a    ) { ser_writedata8(s, a); }
template<typename Stream> inline void serialize(Stream& s, int8_t a  ) { ser_writedata8(s, a); }
template<typename Stream> inline void serialize(Stream& s, uint8_t a ) { ser_writedata8(s, a); }
template<typename Stream> inline void serialize(Stream& s, int16_t a ) { ser_writedata16(s, a); }
template<typename Stream> inline void serialize(Stream& s, uint16_t a) { ser_writedata16(s, a); }
template<typename Stream> inline void serialize(Stream& s, int32_t a ) { ser_writedata32(s, a); }
template<typename Stream> inline void serialize(Stream& s, uint32_t a) { ser_writedata32(s, a); }
template<typename Stream> inline void serialize(Stream& s, int64_t a ) { ser_writedata64(s, a); }
template<typename Stream> inline void serialize(Stream& s, uint64_t a) { ser_writedata64(s, a); }
template<typename Stream> inline void serialize(Stream& s, float a   ) { ser_writedata32(s, ser_float_to_uint32(a)); }
template<typename Stream> inline void serialize(Stream& s, double a  ) { ser_writedata64(s, ser_double_to_uint64(a)); }
template<typename Stream> inline void serialize(Stream& s, bool a)     { char f=a; ser_writedata8(s, f); }

template<typename Stream> inline void unserialize(Stream& s, char& a    ) { a = ser_readdata8(s); }
template<typename Stream> inline void unserialize(Stream& s, int8_t& a  ) { a = ser_readdata8(s); }
template<typename Stream> inline void unserialize(Stream& s, uint8_t& a ) { a = ser_readdata8(s); }
template<typename Stream> inline void unserialize(Stream& s, int16_t& a ) { a = ser_readdata16(s); }
template<typename Stream> inline void unserialize(Stream& s, uint16_t& a) { a = ser_readdata16(s); }
template<typename Stream> inline void unserialize(Stream& s, int32_t& a ) { a = ser_readdata32(s); }
template<typename Stream> inline void unserialize(Stream& s, uint32_t& a) { a = ser_readdata32(s); }
template<typename Stream> inline void unserialize(Stream& s, int64_t& a ) { a = ser_readdata64(s); }
template<typename Stream> inline void unserialize(Stream& s, uint64_t& a) { a = ser_readdata64(s); }
template<typename Stream> inline void unserialize(Stream& s, float& a   ) { a = ser_uint32_to_float(ser_readdata32(s)); }
template<typename Stream> inline void unserialize(Stream& s, double& a  ) { a = ser_uint64_to_double(ser_readdata64(s)); }
template<typename Stream> inline void unserialize(Stream& s, bool& a)     { char f=ser_readdata8(s); a=f; }

struct ser_action_serialize
{
};

struct ser_action_unserialize
{
};

template<typename Stream, typename T>
inline void ser_read_write(Stream& s, const T& obj, ser_action_serialize ser_action)
{
    ::serialize(s, obj);
}

template<typename Stream, typename T>
inline void ser_read_write(Stream& s, T& obj, ser_action_unserialize ser_action)
{
    ::unserialize(s, obj);
}

/*
 * size_computer
 */
class size_computer
{
public:
    size_computer() : nSize(0) {}

    void write(const char *psz, size_t _nSize)
    {
        this->nSize += _nSize;
    }

    /** Pretend _nSize bytes are written, without specifying them. */
    void seek(size_t _nSize)
    {
        this->nSize += _nSize;
    }

    template<typename T>
    size_computer& operator<<(const T& obj)
    {
        ::serialize(*this, obj);
        return (*this);
    }

    size_t size() const {
        return nSize;
    }

protected:
    size_t nSize;
};

template <typename T>
size_t get_serialize_size(const T& t)
{
    return (size_computer() << t).size();
}

/*
 * var_int
 */
template<typename I>
class var_int
{
protected:
    I &n;
public:
    var_int(I& nIn) : n(nIn) { }

    template<typename Stream>
    void serialize(Stream &s) const {
        write_var_int<Stream, I>(s, n);
    }

    template<typename Stream>
    void unserialize(Stream& s) {
        n = read_var_int<Stream, I>(s);
    }
};

template<typename I>
var_int<I> var_int_wrap(I& n) { return var_int<I>(n); }

template<typename I>
inline unsigned int get_size_of_var_int(I n)
{
    int nRet = 0;
    while (true) {
        nRet++;
        if (n <= 0x7F)
            break;
        n = (n >> 7) - 1;
    }
    return nRet;
}

template<typename Stream, typename I>
void write_var_int(Stream& os, I n)
{
    unsigned char tmp[(sizeof(n) * 8 + 6) / 7];
    int len = 0;
    while (true) {
        tmp[len] = (n & 0x7F) | (len ? 0x80 : 0x00);
        if (n <= 0x7F)
            break;
        n = (n >> 7) - 1;
        len++;
    }
    do {
        ser_writedata8(os, tmp[len]);
    } while (len--);
}

template<typename I>
inline void write_var_int(size_computer &s, I n)
{
    s.seek(get_size_of_var_int<I>(n));
}

template<typename Stream, typename I>
I read_var_int(Stream& is)
{
    I n = 0;
    while (true) {
        unsigned char chData = ser_readdata8(is);
        if (n > (std::numeric_limits<I>::max() >> 7)) {
            throw std::ios_base::failure("read_var_int(): size too large");
        }
        n = (n << 7) | (chData & 0x7F);
        if (chData & 0x80) {
            if (n == std::numeric_limits<I>::max()) {
                throw std::ios_base::failure("read_var_int(): size too large");
            }
            n++;
        }
        else {
            return n;
        }
    }
}

/**
 * flat_data
 */
class flat_data
{
protected:
    char* pbegin;
    char* pend;
public:
    flat_data(void* pbeginIn, void* pendIn) : pbegin((char*)pbeginIn), pend((char*)pendIn) { }
    template <class T, class TAl>
    explicit flat_data(std::vector<T, TAl> &v)
    {
        pbegin = (char*)v.data();
        pend = (char*)(v.data() + v.size());
    }
    char* begin() { return pbegin; }
    const char* begin() const { return pbegin; }
    char* end() { return pend; }
    const char* end() const { return pend; }

    template<typename Stream>
    void serialize(Stream& s) const
    {
        s.write(pbegin, pend - pbegin);
    }

    template<typename Stream>
    void userialize(Stream& s)
    {
        s.read(pbegin, pend - pbegin);
    }
};

/**
* limited_string
*/
template<size_t Limit>
class limited_string
{
protected:
    std::string& string;
public:
    limited_string(std::string& _string) : string(_string) {}

    template<typename Stream>
    void unserialize(Stream& s)
    {
        size_t size = read_var_int<Stream, uint32_t>(is);
        if (size > Limit) {
            throw std::ios_base::failure("string length limit exceeded");
        }
        string.resize(size);
        if (size != 0)
            s.read((char*)&string[0], size);
    }

    template<typename Stream>
    void serialize(Stream& s) const
    {
        write_var_int(s, string.size());
        if (!string.empty())
            s.write((char*)&string[0], string.size());
    }
};

/**
 * vector
 */
template<typename Stream, typename T, typename A>
void serialize_impl(Stream& os, const std::vector<T, A>& v, const unsigned char&)
{
    write_var_int(os, v.size());
    if (!v.empty())
        os.write((char*)&v[0], v.size() * sizeof(T));
}

template<typename Stream, typename T, typename A, typename V>
void serialize_impl(Stream& os, const std::vector<T, A>& v, const V&)
{
    write_var_int(os, v.size());
    for (typename std::vector<T, A>::const_iterator vi = v.begin(); vi != v.end(); ++vi)
        ::serialize(os, (*vi));
}

template<typename Stream, typename T, typename A>
inline void serialize(Stream& os, const std::vector<T, A>& v)
{
    serialize_impl(os, v, T());
}

template<typename Stream, typename T, typename A>
void unserialize_impl(Stream& is, std::vector<T, A>& v, const unsigned char&)
{
    v.clear();
    unsigned int nSize = read_var_int<Stream, uint32_t>(is);
    unsigned int i = 0;
    while (i < nSize)
    {
        unsigned int blk = std::min(nSize - i, (unsigned int)(1 + 4999999 / sizeof(T)));
        v.resize(i + blk);
        is.read((char*)&v[i], blk * sizeof(T));
        i += blk;
    }
}

template<typename Stream, typename T, typename A, typename V>
void unserialize_impl(Stream& is, std::vector<T, A>& v, const V&)
{
    v.clear();
    unsigned int nSize = read_var_int<Stream, uint32_t>(is);
    unsigned int i = 0;
    unsigned int nMid = 0;
    while (nMid < nSize)
    {
        nMid += 5000000 / sizeof(T);
        if (nMid > nSize)
            nMid = nSize;
        v.resize(nMid);
        for (; i < nMid; i++)
            unserialize(is, v[i]);
    }
}

template<typename Stream, typename T, typename A>
inline void unserialize(Stream& is, std::vector<T, A>& v)
{
    unserialize_impl(is, v, T());
}

/**
 * string
 */
template<typename Stream, typename C>
void serialize(Stream& os, const std::basic_string<C>& str)
{
    write_var_int(os, str.size());
    if (!str.empty())
        os.write((char*)&str[0], str.size() * sizeof(str[0]));
}

template<typename Stream, typename C>
void unserialize(Stream& is, std::basic_string<C>& str)
{
    unsigned int nSize = read_var_int<Stream, uint32_t>(is);
    str.resize(nSize);
    if (nSize != 0)
        is.read((char*)&str[0], nSize * sizeof(str[0]));
}

/**
 * prevector
 */
#if 0
template<typename Stream, unsigned int N, typename T>
void serialize_impl(Stream& os, const prevector<N, T>& v, const unsigned char&)
{
    write_var_int(os, v.size());
    if (!v.empty())
        os.write((char*)&v[0], v.size() * sizeof(T));
}

template<typename Stream, unsigned int N, typename T, typename V>
void serialize_impl(Stream& os, const prevector<N, T>& v, const V&)
{
    write_var_int(os, v.size());
    for (typename prevector<N, T>::const_iterator vi = v.begin(); vi != v.end(); ++vi)
        ::serialize(os, (*vi));
}

template<typename Stream, unsigned int N, typename T>
inline void serialize(Stream& os, const prevector<N, T>& v)
{
    serialize_impl(os, v, T());
}


template<typename Stream, unsigned int N, typename T>
void unserialize_impl(Stream& is, prevector<N, T>& v, const unsigned char&)
{
    // Limit size per read so bogus size value won't cause out of memory
    v.clear();
    unsigned int nSize = read_var_int(is);
    unsigned int i = 0;
    while (i < nSize)
    {
        unsigned int blk = std::min(nSize - i, (unsigned int)(1 + 4999999 / sizeof(T)));
        v.resize(i + blk);
        is.read((char*)&v[i], blk * sizeof(T));
        i += blk;
    }
}

template<typename Stream, unsigned int N, typename T, typename V>
void unserialize_impl(Stream& is, prevector<N, T>& v, const V&)
{
    v.clear();
    unsigned int nSize = read_var_int(is);
    unsigned int i = 0;
    unsigned int nMid = 0;
    while (nMid < nSize)
    {
        nMid += 5000000 / sizeof(T);
        if (nMid > nSize)
            nMid = nSize;
        v.resize(nMid);
        for (; i < nMid; i++)
            unserialize(is, v[i]);
    }
}

template<typename Stream, unsigned int N, typename T>
inline void unserialize(Stream& is, prevector<N, T>& v)
{
    unserialize_impl(is, v, T());
}

#endif

/**
 * pair
 */
template<typename Stream, typename K, typename T>
void serialize(Stream& os, const std::pair<K, T>& item)
{
    serialize(os, item.first);
    serialize(os, item.second);
}

template<typename Stream, typename K, typename T>
void unserialize(Stream& is, std::pair<K, T>& item)
{
    unserialize(is, item.first);
    unserialize(is, item.second);
}

/**
 * map
 */
template<typename Stream, typename K, typename T, typename Pred, typename A>
void serialize(Stream& os, const std::map<K, T, Pred, A>& m)
{
    write_var_int(os, m.size());
    for (typename std::map<K, T, Pred, A>::const_iterator mi = m.begin(); mi != m.end(); ++mi)
        serialize(os, (*mi));
}

template<typename Stream, typename K, typename T, typename Pred, typename A>
void unserialize(Stream& is, std::map<K, T, Pred, A>& m)
{
    m.clear();
    unsigned int nSize = read_var_int<Stream, uint32_t>(is);
    typename std::map<K, T, Pred, A>::iterator mi = m.begin();
    for (unsigned int i = 0; i < nSize; i++)
    {
        std::pair<K, T> item;
        unserialize(is, item);
        mi = m.insert(mi, item);
    }
}

/**
 * set
 */
template<typename Stream, typename K, typename Pred, typename A>
void serialize(Stream& os, const std::set<K, Pred, A>& m)
{
    write_var_int(os, m.size());
    for (typename std::set<K, Pred, A>::const_iterator it = m.begin(); it != m.end(); ++it)
        serialize(os, (*it));
}

template<typename Stream, typename K, typename Pred, typename A>
void unserialize(Stream& is, std::set<K, Pred, A>& m)
{
    m.clear();
    unsigned int nSize = read_var_int<Stream, uint32_t>(is);
    typename std::set<K, Pred, A>::iterator it = m.begin();
    for (unsigned int i = 0; i < nSize; i++)
    {
        K key;
        unserialize(is, key);
        it = m.insert(it, key);
    }
}

/**
* shared_ptr
*/
template<typename Stream, typename T> void
serialize(Stream& os, const std::shared_ptr<T>& p)
{
    serialize(os, *p);
}

template<typename Stream, typename T>
void unserialize(Stream& is, std::shared_ptr<T>& p)
{
    //p = std::make_shared<const T>(deserialize, is);
    p.reset(new T);
    unserialize(is, *p);
}

/**
* unique_ptr
*/
template<typename Stream, typename T> void
serialize(Stream& os, const std::unique_ptr<T>& p)
{
    serialize(os, *p);
}

template<typename Stream, typename T>
void unserialize(Stream& is, std::unique_ptr<T>& p)
{
    //p.reset(new T(deserialize, is));
    p.reset(new T);
    unserialize(is, *p);
}

/*
* member functon
*/
template<typename Stream, typename T>
inline void serialize(Stream& os, const T& a)
{
    a.serialize(os);
}

template<typename Stream, typename T>
inline void unserialize(Stream& is, T& a)
{
    a.unserialize(is);
}

template<typename Stream>
void serialize_many(Stream& s)
{
}

template<typename Stream, typename Arg>
void serialize_many(Stream& s, Arg&& arg)
{
    ::serialize(s, std::forward<Arg>(arg));
}

template<typename Stream, typename Arg, typename... Args>
void serialize_many(Stream& s, Arg&& arg, Args&&... args)
{
    ::serialize(s, std::forward<Arg>(arg));
    ::serialize_many(s, std::forward<Args>(args)...);
}

template<typename Stream>
inline void unserialize_many(Stream& s)
{
}

template<typename Stream, typename Arg>
inline void unserialize_many(Stream& s, Arg& arg)
{
    ::unserialize(s, arg);
}

template<typename Stream, typename Arg, typename... Args>
inline void unserialize_many(Stream& s, Arg& arg, Args&... args)
{
    ::unserialize(s, arg);
    ::unserialize_many(s, args...);
}

template<typename Stream, typename... Args>
inline void ser_read_write_many(Stream& s, ser_action_serialize ser_action, Args&&... args)
{
    ::serialize_many(s, std::forward<Args>(args)...);
}

template<typename Stream, typename... Args>
inline void ser_read_write_many(Stream& s, ser_action_unserialize ser_action, Args&... args)
{
    ::unserialize_many(s, args...);
}

#endif // BCUS_SERIALIZE_H
