#include "error_code.h"
#include <map>

namespace bcus {

#define ERROR_CASE_MSG(code) case code : {return #code; }


const char *error::get_error_msg(const error_code &ec) {
    if (ec == 0) {
        return "success";
    }
#ifdef WIN32
    //static std::map<DWORD, char *> map_thread_error;
    //DWORD threadid = GetCurrentThreadId();
    //std::map<DWORD, char *>::iterator itr = map_thread_error.find(threadid);
    LPVOID  error_msg;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER  | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
                    ec, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &error_msg, 0, NULL );
    static char sz[128] = {0};
    strcpy(sz, (const char *)error_msg);
    LocalFree(error_msg);
    return sz;
#else
    return strerror(ec);
#endif
}

  /*
const char *error::get_error_msg(const error_code &ec)
{
    switch(ec)
    {
        ERROR_CASE_MSG(success)
        ERROR_CASE_MSG(access_denied)
        ERROR_CASE_MSG(address_family_not_supported)
        ERROR_CASE_MSG(address_in_use)
        ERROR_CASE_MSG(already_connected)
        ERROR_CASE_MSG(already_started)
        ERROR_CASE_MSG(broken_pipe)
        ERROR_CASE_MSG(connection_aborted)
        ERROR_CASE_MSG(connection_refused)
        ERROR_CASE_MSG(connection_reset)
        ERROR_CASE_MSG(bad_descriptor)
        ERROR_CASE_MSG(fault)
        ERROR_CASE_MSG(host_unreachable)
        ERROR_CASE_MSG(in_progress)
        ERROR_CASE_MSG(interrupted)
        ERROR_CASE_MSG(invalid_argument)
        ERROR_CASE_MSG(message_too_long)
        ERROR_CASE_MSG(name_too_long)
        ERROR_CASE_MSG(network_down)
        ERROR_CASE_MSG(network_reset)
        ERROR_CASE_MSG(network_unreachable)
        ERROR_CASE_MSG(no_descriptors)
        ERROR_CASE_MSG(no_buffer_space)
        ERROR_CASE_MSG(no_memory)
        ERROR_CASE_MSG(no_permission)
        ERROR_CASE_MSG(no_protocol_option)
        ERROR_CASE_MSG(not_connected)
        ERROR_CASE_MSG(not_socket)
        ERROR_CASE_MSG(operation_aborted)
        ERROR_CASE_MSG(operation_not_supported)
        ERROR_CASE_MSG(shut_down)
        ERROR_CASE_MSG(timed_out)
        //ERROR_CASE_MSG(try_again)
        ERROR_CASE_MSG(would_block)
        ERROR_CASE_MSG(already_open)
        ERROR_CASE_MSG(eof)
        ERROR_CASE_MSG(not_found)
        ERROR_CASE_MSG(fd_set_failure)
        default:
            return "unknow error";
    }
}    */

}
