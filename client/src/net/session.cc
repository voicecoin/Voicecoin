#include "session.h"

namespace bcus {

session::session()
{
    static uint32_t sm_session_id = 0;
    session_id_ = (time(NULL) << 16) | (++sm_session_id);
}

}
