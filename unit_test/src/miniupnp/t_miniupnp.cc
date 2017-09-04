#include <gtest/gtest.h>

#include "loghelper.h"

#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/miniwget.h>
#include <miniupnpc/upnpcommands.h>
#include <miniupnpc/upnperrors.h>

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

TEST(miniupnp, t1) {
    XLOG(XLOG_DEBUG, "%s\n", typeid(this).name());

    const char * multicastif = NULL;
    const char * minissdpdpath = NULL;
    char lanaddr[64] = {0};

    int error = 0;
    struct UPNPDev *dev = upnpDiscover(2000, multicastif, minissdpdpath, 0, 0, 2, &error);
    if (error) {
        XLOG(XLOG_DEBUG, "%s, upnpDiscover failed, r[%d-%s]\n", typeid(this).name(), error, strupnperror(error));
        return;
    }
    dump(dev);

    struct UPNPUrls urls;
    struct IGDdatas data;

     /*    0 = NO IGD found
     *     1 = A valid connected IGD has been found
     *     2 = A valid IGD has been found but it reported as not connected
     *     3 = an UPnP device has been found but was not recognized as an IGD
     */
    int r = UPNP_GetValidIGD(dev, &urls, &data, lanaddr, sizeof(lanaddr));
    if (r != 1) {
        XLOG(XLOG_DEBUG, "%s, No valid UPnP IGDs found, r[%d-%s]\n", typeid(this).name(), r, strupnperror(r));
        freeUPNPDevlist(dev);
        if (r != 0) {
            FreeUPNPUrls(&urls);
        }
        return;
    }

    char externalIPAddress[40] = {0};
    r = UPNP_GetExternalIPAddress(urls.controlURL, data.first.servicetype, externalIPAddress);
    if(r != UPNPCOMMAND_SUCCESS || externalIPAddress[0] == 0) {
        XLOG(XLOG_DEBUG, "%s, UPNP_GetExternalIPAddress failed, r[%d-%s]\n", typeid(this).name(), r, strupnperror(r));
        freeUPNPDevlist(dev);
        FreeUPNPUrls(&urls);
        return;
    }

    XLOG(XLOG_DEBUG, "%s, updp externalIPAddress[%s]\n", typeid(this).name(), externalIPAddress);

    const char *port = "8008";
    const char *desc = "test hhh 1.0.0.0";
    while (1) {
        r = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype,
                                    port, port, lanaddr, desc, "TCP", 0, "0");

        if(r!=UPNPCOMMAND_SUCCESS) {
            XLOG(XLOG_DEBUG, "%s, UPNP_AddPortMapping failed, ip[%s], port[%s], r[%d-%s]\n",
                typeid(this).name(), externalIPAddress, port, r, strupnperror(r));
        }
        XLOG(XLOG_DEBUG, "%s, UPNP_AddPortMapping succ, ip[%s], port[%s]\n",
            typeid(this).name(), externalIPAddress, port);

        break; //sleep(20*60); // Refresh every 20 minutes
    }

    freeUPNPDevlist(dev);
    FreeUPNPUrls(&urls);
}

