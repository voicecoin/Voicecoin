#ifndef BCUS_COMMON_SINGLETON_H
#define BCUS_COMMON_SINGLETON_H
#include <stdio.h>
#include "noncopyable.h"

namespace bcus {

template <typename T>
class singleton : public bcus::noncopyable
{
public:
    singleton() {}
private:
    struct object_creator {
        // This constructor does nothing more than ensure that instance()
        //  is called before main() begins, thus creating the static
        //  T object before multithreading race issues can come up.
        object_creator() {
            singleton<T>::get_instance();
        }
        inline void do_nothing() const {
        }
    };
    static object_creator create_object;

public:
    typedef T object_type;

    static T* get_instance() {
        static object_type obj;
        create_object.do_nothing();
        return &obj;
    }
};

template <typename T>
typename singleton<T>::object_creator singleton<T>::create_object;

}

#endif
