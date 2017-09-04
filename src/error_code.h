#ifndef BCUS_ERROR_CODE_H
#define BCUS_ERROR_CODE_H

#if defined(WINDOWS) || defined(WIN32)
# include <winsock2.h>
# include <Windows.h>
#else
# include <cerrno>
# include <netdb.h>
# include <string.h>
#endif

#if defined(WINDOWS) || defined(WIN32)
# define BCUS_NATIVE_ERROR(e) e
# define BCUS_SOCKET_ERROR(e) WSA ## e
# define BCUS_NETDB_ERROR(e) WSA ## e
# define BCUS_GETADDRINFO_ERROR(e) WSA ## e
# define BCUS_WIN_OR_POSIX(e_win, e_posix) e_win
#else
# define BCUS_NATIVE_ERROR(e) e
# define BCUS_SOCKET_ERROR(e) e
# define BCUS_NETDB_ERROR(e) e
# define BCUS_GETADDRINFO_ERROR(e) e
# define BCUS_WIN_OR_POSIX(e_win, e_posix) e_posix
#endif

namespace bcus {

typedef int error_code;

class error {
public:
    enum basic_errors
    {
        success = 0,
        access_denied = BCUS_SOCKET_ERROR(EACCES),
        address_family_not_supported = BCUS_SOCKET_ERROR(EAFNOSUPPORT),
        address_in_use = BCUS_SOCKET_ERROR(EADDRINUSE),
        already_connected = BCUS_SOCKET_ERROR(EISCONN),
        already_started = BCUS_SOCKET_ERROR(EALREADY),
        broken_pipe = BCUS_WIN_OR_POSIX(
          BCUS_NATIVE_ERROR(ERROR_BROKEN_PIPE),
          BCUS_NATIVE_ERROR(EPIPE)),
        connection_aborted = BCUS_SOCKET_ERROR(ECONNABORTED),
        connection_refused = BCUS_SOCKET_ERROR(ECONNREFUSED),
        connection_reset = BCUS_SOCKET_ERROR(ECONNRESET),
        bad_descriptor = BCUS_SOCKET_ERROR(EBADF),
        fault = BCUS_SOCKET_ERROR(EFAULT),
        host_unreachable = BCUS_SOCKET_ERROR(EHOSTUNREACH),
        in_progress = BCUS_SOCKET_ERROR(EINPROGRESS),
        interrupted = BCUS_SOCKET_ERROR(EINTR),
        invalid_argument = BCUS_SOCKET_ERROR(EINVAL),
        message_too_long = BCUS_SOCKET_ERROR(EMSGSIZE),
        name_too_long = BCUS_SOCKET_ERROR(ENAMETOOLONG),
        network_down = BCUS_SOCKET_ERROR(ENETDOWN),
        network_reset = BCUS_SOCKET_ERROR(ENETRESET),
        network_unreachable = BCUS_SOCKET_ERROR(ENETUNREACH),
        no_descriptors = BCUS_SOCKET_ERROR(EMFILE),
        no_buffer_space = BCUS_SOCKET_ERROR(ENOBUFS),
        no_memory = BCUS_WIN_OR_POSIX(
          BCUS_NATIVE_ERROR(ERROR_OUTOFMEMORY),
          BCUS_NATIVE_ERROR(ENOMEM)),
        no_permission = BCUS_WIN_OR_POSIX(
          BCUS_NATIVE_ERROR(ERROR_ACCESS_DENIED),
          BCUS_NATIVE_ERROR(EPERM)),
        no_protocol_option = BCUS_SOCKET_ERROR(ENOPROTOOPT),
        not_connected = BCUS_SOCKET_ERROR(ENOTCONN),
        not_socket = BCUS_SOCKET_ERROR(ENOTSOCK),
        operation_aborted = BCUS_WIN_OR_POSIX(
          BCUS_NATIVE_ERROR(ERROR_OPERATION_ABORTED),
          BCUS_NATIVE_ERROR(ECANCELED)),
        operation_not_supported = BCUS_SOCKET_ERROR(EOPNOTSUPP),
        shut_down = BCUS_SOCKET_ERROR(ESHUTDOWN),
        timed_out = BCUS_SOCKET_ERROR(ETIMEDOUT),
        try_again = BCUS_WIN_OR_POSIX(
          BCUS_NATIVE_ERROR(ERROR_RETRY),
          BCUS_NATIVE_ERROR(EAGAIN)),
        would_block = BCUS_SOCKET_ERROR(EWOULDBLOCK)
    };

    enum misc_errors
    {
        already_open = 501,
        eof,
        not_found,
        fd_set_failure
    };

    static inline error_code get_last_error() {
        #if defined(WINDOWS) || defined(WIN32)
            return WSAGetLastError();

        #else
            return errno;
        #endif
    }
    static inline void clear_last_error()
    {
        #if defined(WINDOWS) || defined(WIN32)
            WSASetLastError(0);
        #else
            errno = 0;
        #endif
    }
    static const char *get_error_msg(const error_code &ec);
};

}

#endif
