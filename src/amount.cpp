// Copyright (c) 2017-2018 VoiceExpert Squall
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the GPL3 software license, see the accompanying
// file COPYING or http://www.gnu.org/licenses/gpl.html.

#include "amount.h"
#include "primitives/block.h"
#include "tinyformat.h"

CFeeRate::CFeeRate(const CAmount& nFeePaid, size_t nSize)
{
    if (nSize > 0)
        nVoiceunsPerK = nFeePaid*1000/nSize;
    else
        nVoiceunsPerK = 0;
}

CAmount CFeeRate::GetFee(size_t nSize) const
{
    CAmount nFee = nVoiceunsPerK*nSize / 1000;

    if (nFee == 0 && nVoiceunsPerK > 0)
        nFee = nVoiceunsPerK;

    return std::max(nFee, GetMinFee(nSize));
}

std::string CFeeRate::ToString() const
{
    return strprintf("%d.%06d VC/kB", nVoiceunsPerK / COIN, nVoiceunsPerK % COIN);
}

CAmount GetMinFee(size_t nBytes)
{
    // Base fee is either MIN_TX_FEE or MIN_RELAY_TX_FEE
    CAmount nBaseFee = MIN_TX_FEE;
    CAmount nMinFee = (1 + nBytes / (10 * 1024)) * nBaseFee; // 1 subcent per 10 kb of data

    if (!MoneyRange(nMinFee))
        nMinFee = MAX_MONEY;
    return nMinFee;
}
