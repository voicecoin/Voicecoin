#ifndef SERIAL_TYPE_H
#define SERIAL_TYPE_H

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
#include <boost/shared_ptr.hpp>

class size_computer;

template<typename Stream, typename I>
void write_var_int(Stream& os, I n);

template<typename I>
void write_var_int(size_computer &s, I n);

template<typename Stream, typename I>
I read_var_int(Stream& is);

template<typename Stream, typename T, typename A>
void serialize(Stream& os, const std::vector<T, A>& v);

template<typename Stream, typename C>
void serialize(Stream& os, const std::basic_string<C>& str);


template<typename Stream, typename T, typename A>
inline void unserialize(Stream& is, std::vector<T, A>& v);

template<typename Stream, typename C>
void unserialize(Stream& is, std::basic_string<C>& str);


template<typename Stream, typename T> void
serialize(Stream& os, const boost::shared_ptr<T>& p);

template<typename Stream, typename T>
void serialize(Stream& os, const T& a);

template<typename Stream, typename T>
void unserialize(Stream& is, T& a);

template<typename Stream>
void serialize_many(Stream& s);

template<typename Stream, typename Arg>
void serialize_many(Stream& s, Arg&& arg);

template<typename Stream, typename Arg, typename... Args>
void serialize_many(Stream& s, Arg&& arg, Args&&... args);

template<typename Stream>
void unserialize_many(Stream& s);

template<typename Stream, typename Arg>
void unserialize_many(Stream& s, Arg& arg);

template<typename Stream, typename Arg, typename... Args>
void unserialize_many(Stream& s, Arg& arg, Args&... args);


template<typename Stream, typename T>
void unserialize(Stream& is, boost::shared_ptr<T>& p);


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

#endif


