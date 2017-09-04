#include "upnp_loader_thread.h"
#include "loghelper.h"
#include <boost/bind.hpp>
#include "string_helper.h"

namespace bcus {

static void dump(const struct UPNPDev *dev) {
    /*
    struct UPNPDev {
        struct UPNPDev * pNext;
        char * descURL;
        char * st;
        unsigned int scope_id;
        char * usn;
        char buffer[3];
    };
    */
    XLOG(XLOG_DEBUG, "UPNPDev:\n");
    const struct UPNPDev *d = dev;
    while(d) {
        XLOG(XLOG_DEBUG, "    descURL[%s]\n", d->descURL);
        XLOG(XLOG_DEBUG, "    st[%s]\n", d->st);
        XLOG(XLOG_DEBUG, "    scope_id[%d]\n", d->scope_id);
        XLOG(XLOG_DEBUG, "    usn[%d]\n", d->usn);
        XLOG(XLOG_DEBUG, "    buffer[%d]\n", d->buffer);
        d = d->pNext;
    }
}

void upnp_loader_thread::init(const char *name, int port) {
    port_ = string_helper::format("%d", port);
    name_ = name;
    init_resource();
}

void upnp_loader_thread::start() {
    thread_ = boost::thread(boost::bind(&upnp_loader_thread::do_upnp_mapping, this));
}
void upnp_loader_thread::stop() {
    if (dev_ == NULL) {
        return;
    }

    int r = UPNP_DeletePortMapping(urls_.controlURL, data_.first.servicetype, port_.c_str(), "TCP", 0);
    if(r != UPNPCOMMAND_SUCCESS) {
        XLOG(XLOG_DEBUG, "upnp_loader_thread::%s, UPNP_DeletePortMapping failed, ip[%s], port[%s], r[%d-%s]\n",
            __FUNCTION__, externip_, port_.c_str(), r, strupnperror(r));
    }
    freeUPNPDevlist(dev_);
    FreeUPNPUrls(&urls_);
    dev_ = NULL;
}
void upnp_loader_thread::join() {
    thread_.join();
}

void upnp_loader_thread::init_resource() {
    XLOG(XLOG_DEBUG, "upnp_loader_thread::%s, port[%s]\n", __FUNCTION__, port_.c_str());

    dev_ = NULL;
    memset(lanaddr_, sizeof(lanaddr_), 0);
    memset(externip_, sizeof(externip_), 0);

    const char * multicastif = NULL;
    const char * minissdpdpath = NULL;

    int error = 0;
    dev_ = upnpDiscover(2000, multicastif, minissdpdpath, 0, 0, 2, &error);
    if (error) {
        XLOG(XLOG_DEBUG, "upnp_loader_thread::%s, upnpDiscover failed, r[%d-%s]\n", __FUNCTION__, error, strupnperror(error));
        return;
    }
    dump(dev_);

     /*    0 = NO IGD found
     *     1 = A valid connected IGD has been found
     *     2 = A valid IGD has been found but it reported as not connected
     *     3 = an UPnP device has been found but was not recognized as an IGD
     */
    int r = UPNP_GetValidIGD(dev_, &urls_, &data_, lanaddr_, sizeof(lanaddr_));
    if (r != 1) {
        XLOG(XLOG_DEBUG, "upnp_loader_thread::%s, No valid UPnP IGDs found, r[%d-%s]\n", __FUNCTION__, r, strupnperror(r));
        freeUPNPDevlist(dev_);
        dev_ = NULL;
        if (r != 0) {
            FreeUPNPUrls(&urls_);
        }
        return;
    }

    r = UPNP_GetExternalIPAddress(urls_.controlURL, data_.first.servicetype, externip_);
    if(r != UPNPCOMMAND_SUCCESS || externip_[0] == 0) {
        XLOG(XLOG_DEBUG, "upnp_loader_thread::%s, UPNP_GetExternalIPAddress failed, r[%d-%s]\n", __FUNCTION__, r, strupnperror(r));
        freeUPNPDevlist(dev_);
        FreeUPNPUrls(&urls_);
        dev_ = NULL;
        return;
    }

    XLOG(XLOG_DEBUG, "upnp_loader_thread::%s, urls_.controlURL[%s], data_.first.servicetype[%s], externip_[%s]\n",
        __FUNCTION__,  urls_.controlURL, data_.first.servicetype, externip_);

}
void upnp_loader_thread::do_upnp_mapping() {
    while (1) {
        int r = UPNP_AddPortMapping(urls_.controlURL, data_.first.servicetype, port_.c_str(), port_.c_str(), lanaddr_, name_.c_str(), "TCP", 0, "0");
        if(r != UPNPCOMMAND_SUCCESS) {
            XLOG(XLOG_DEBUG, "upnp_loader_thread::%s, UPNP_AddPortMapping failed, ip[%s], port[%s], r[%d-%s]\n",
                __FUNCTION__, externip_, port_.c_str(), r, strupnperror(r));
        }
        XLOG(XLOG_DEBUG, "upnp_loader_thread::%s, UPNP_AddPortMapping succ, ip[%s], port[%s]\n",
            __FUNCTION__, externip_, port_.c_str());

        sleep(20 * 60); // Refresh every 20 minutes
    }
}


}
