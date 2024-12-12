// Copyright (c) 2014 The Bitcoin Core developers
// Distributed under the GPL3 software license, see the accompanying
// file COPYING or http://www.gnu.org/licenses/gpl.html.

#include "primitives/transaction.h"
#include "main.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(main_tests)

// voicecoin: TODO - add a complex test to block value generation.

//BOOST_AUTO_TEST_CASE(subsidy_limit_test)
//{
//    CAmount nSum = 0;
//    for (int nHeight = 0; nHeight < 14000000; nHeight += 1000) {
//        CAmount nSubsidy = GetBlockValue(nHeight, 0);
//        BOOST_CHECK(nSubsidy <= 50 * COIN);
//        nSum += nSubsidy * 1000;
//        BOOST_CHECK(MoneyRange(nSum));
//    }
//    BOOST_CHECK(nSum == 2099999997690000ULL);
//}

BOOST_AUTO_TEST_SUITE_END()
