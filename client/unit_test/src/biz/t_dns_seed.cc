#include <gtest/gtest.h>

#include "loghelper.h"
#include "dns_seed.h"
#include <boost/bind.hpp>
#include <boost/thread.hpp>


TEST(biz_dns_seed, c1) {
    XLOG(XLOG_DEBUG, "%s\n", typeid(this).name());

    boost::asio::io_service io_service;
    bcus::dns_seed seed(io_service);
    seed.resolve("www.baidu.com", "80");
    seed.resolve("seed.bitcoin.sipa.be", "8333");
    seed.resolve("dnsseed.bluematt.me", "8333");
    seed.resolve("dnsseed.bitcoin.dashjr.org", "8333");

    //vSeeds.emplace_back("seed.bitcoin.sipa.be", true); // Pieter Wuille, only supports x1, x5, x9, and xd
    //vSeeds.emplace_back("dnsseed.bluematt.me", true); // Matt Corallo, only supports x9
    //vSeeds.emplace_back("dnsseed.bitcoin.dashjr.org", false); // Luke Dashjr
    //vSeeds.emplace_back("seed.bitcoinstats.com", true); // Christian Decker, supports x1 - xf
    //vSeeds.emplace_back("seed.bitcoin.jonasschnelli.ch", true); // Jonas Schnelli, only supports x1, x5, x9, and xd
    //vSeeds.emplace_back("seed.btc.petertodd.org", true); //
}

