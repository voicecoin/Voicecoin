#ifndef BCUS_NONCOPYABLE_H
#define BCUS_NONCOPYABLE_H

namespace bcus {

class noncopyable {
protected:
    noncopyable() {}
    ~noncopyable() {}
private:
    noncopyable(const noncopyable&);
    const noncopyable& operator=(const noncopyable&);
};
}

#endif
