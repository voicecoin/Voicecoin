#include "socket_ops.h"
#include <stdio.h>
#include <cstring>

#if defined(WINDOWS) || defined(WIN32)
#  define FD_SETSIZE 1024
#  include <WinSock2.h>
#  include <ws2tcpip.h>
#  include <Windows.h>
#  include <Iphlpapi.h>
#else
  # include <sys/ioctl.h>
  # include <sys/types.h>
  # include <sys/stat.h>
  # include <fcntl.h>
  # if defined(__hpux)
     # include <sys/time.h>
  # endif
  # if !defined(__hpux) || defined(__SELECT)
    # include <sys/select.h>
  # endif
  # include <sys/socket.h>
  # include <sys/uio.h>
  # include <sys/un.h>
  # include <netinet/in.h>
  # include <arpa/inet.h>
  # include <netdb.h>
  # include <net/if.h>
  # include <limits.h>
  # include <unistd.h>
  # include <sys/types.h>
  # include <sys/ioctl.h>
  # include <netinet/tcp.h>
  # include <ifaddrs.h>
#endif

#if defined(WIN32) || defined(WINDOWS)
//#pragma comment(lib,"ws2_32.lib")
#endif


namespace bcus {

#if defined(WINDOWS) || defined(WIN32)

int socket_ops::get_interface_ip(const char *interface1, char *outip, error_code& ec) {
    PIP_ADAPTER_INFO adapter_info = new IP_ADAPTER_INFO();
    DWORD adapter_size = sizeof(adapter_info);
    int ret = ::GetAdaptersInfo(adapter_info, &adapter_size);
    int adapter_num = 0;
    if (ERROR_BUFFER_OVERFLOW == ret)
    {
        delete adapter_info;
        adapter_info = (PIP_ADAPTER_INFO)(new char[adapter_size]);
        ret = ::GetAdaptersInfo(adapter_info, &adapter_size);
    }
    if (ERROR_SUCCESS != ret) {
        return -1;
    }

    while (adapter_info) {
        IP_ADDR_STRING *ip_addr_list =&(adapter_info->IpAddressList);
        if (0 == strcmp(interface1, adapter_info->AdapterName)) {
            strcpy(outip, ip_addr_list->IpAddress.String);
            delete adapter_info;
            return 0;
        }
        //fprintf(stdout, "[%s]: [%s], [%s]\n", adapter_info->AdapterName,
        //    ip_addr_list->IpAddress.String, adapter_info->Description);
        adapter_info = adapter_info->Next;
    }
    if (adapter_info)
    {
        delete adapter_info;
    }
    return -1;
}

int socket_ops::get_local_ip(const char *mask, char *outip, bcus::error_code& ec) {
    char host_name[255] = {0};
    char *local_ip = NULL;
    if(::gethostname(host_name,sizeof(host_name)) == -1) {
        ec = bcus::error::get_last_error();
        return -1;
    }

    struct hostent *hp = ::gethostbyname(host_name);
    if(NULL == hp) {
        ec = bcus::error::get_last_error();
        return -1;
    }
    char **pptr = hp->h_addr_list;
    for(; *pptr != NULL; pptr++)
    {
        local_ip = inet_ntoa(*(struct in_addr *)*pptr);
        if(0 == strncmp(local_ip, mask, strlen(mask)))
        {
            strcpy(outip, local_ip);
            return 0;
        }
    }
    ec = bcus::error::get_last_error();
    return -1;
}

#else
int socket_ops::get_interface_ip(const char *interface, char *outip, bcus::error_code& ec)
{
    struct ifaddrs *if_addrs = NULL;
    ::getifaddrs(&if_addrs);

    int ret = -1;
    for (struct ifaddrs *ifa = if_addrs; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
        {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET)  //ipv4
        {
            if (0 == strcmp(interface, ifa->ifa_name)) {
                void *pp = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                char this_ip[INET_ADDRSTRLEN];
                ::inet_ntop(AF_INET, pp, this_ip, INET_ADDRSTRLEN);
                strcpy(outip, this_ip);
                ret = 0;
                break;
            }
        }
    }
    if (if_addrs != NULL) {
        ::freeifaddrs(if_addrs);
    }
    return ret;
}

int  socket_ops::get_local_ip(const char *mask, char *outip, error_code& ec) {
    struct ifaddrs *if_addrs = NULL;
    ::getifaddrs(&if_addrs);

    int ret = -1;
    for (struct ifaddrs *ifa = if_addrs; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr)
        {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            void *pp = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char this_ip[INET_ADDRSTRLEN];
            ::inet_ntop(AF_INET, pp, this_ip, INET_ADDRSTRLEN);
            if (0 == strncmp(this_ip, mask, strlen(mask))) {
                strcpy(outip, this_ip);
                ret = 0;
                break;
            }
        }
    }
    if (if_addrs != NULL) {
        ::freeifaddrs(if_addrs);
    }
    return ret;
}
#endif

int socket_ops::gethostbyname(const char *host_name, char *outip, bcus::error_code& ec) {
    struct hostent *hp = ::gethostbyname(host_name);
    if(NULL == hp) {
        ec = bcus::error::get_last_error();
        return -1;
    }
    for(char **pptr = hp->h_addr_list; *pptr != NULL; pptr++)
    {
        ec = 0;
        char *ip = inet_ntoa(*(struct in_addr *)*pptr);
        strcpy(outip, ip);
        return 0;
    }
    ec = bcus::error::get_last_error();
    return -1;
}

}

