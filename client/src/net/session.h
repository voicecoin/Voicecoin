#ifndef BCUS_NET_SESSION_H
#define BCUS_NET_SESSION_H

#include "platform.h"

namespace bcus {

class session {
public:
    session();
    virtual ~session() { }
    uint32_t session_id() const { return session_id_; }
protected:
    uint32_t session_id_;
};

}

#endif
