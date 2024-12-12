// Copyright (c) 2017-2018 VoiceExpert Squall
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the GPL3 software license, see the accompanying
// file COPYING or http://www.gnu.org/licenses/gpl.html.

#ifndef BITCOIN_AMOUNT_H
#define BITCOIN_AMOUNT_H

#include "serialize.h"

#include <stdlib.h>
#include <string>

typedef int64_t CAmount;

static const CAmount COIN    = 1000000;
static const CAmount CENT    = 10000;
static const CAmount SUBCENT = 100;
static const CAmount MIN_TX_FEE = SUBCENT;

/** No amount larger than this (in satoshi) is valid */
static const CAmount MAX_MONEY = 1000000000 * COIN;
inline bool MoneyRange(const CAmount& nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }

/** Type-safe wrapper class to for fee rates
 * (how much to pay based on transaction size)
 */
class CFeeRate
{
private:
    CAmount nVoiceunsPerK; // unit is satoshis-per-1,000-bytes
public:
    CFeeRate() : nVoiceunsPerK(0) { }
    explicit CFeeRate(const CAmount& _nVoiceunsPerK): nVoiceunsPerK(_nVoiceunsPerK) { }
    CFeeRate(const CAmount& nFeePaid, size_t nSize);
    CFeeRate(const CFeeRate& other) { nVoiceunsPerK = other.nVoiceunsPerK; }

    CAmount GetFee(size_t size) const; // unit returned is satoshis
    CAmount GetFeePerK() const { return GetFee(1000); } // satoshis-per-1000-bytes

    friend bool operator<(const CFeeRate& a, const CFeeRate& b) { return a.nVoiceunsPerK < b.nVoiceunsPerK; }
    friend bool operator>(const CFeeRate& a, const CFeeRate& b) { return a.nVoiceunsPerK > b.nVoiceunsPerK; }
    friend bool operator==(const CFeeRate& a, const CFeeRate& b) { return a.nVoiceunsPerK == b.nVoiceunsPerK; }
    friend bool operator<=(const CFeeRate& a, const CFeeRate& b) { return a.nVoiceunsPerK <= b.nVoiceunsPerK; }
    friend bool operator>=(const CFeeRate& a, const CFeeRate& b) { return a.nVoiceunsPerK >= b.nVoiceunsPerK; }
    std::string ToString() const;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(nVoiceunsPerK);
    }
};

CAmount GetMinFee(size_t nBytes);

#endif //  BITCOIN_AMOUNT_H
