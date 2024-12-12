// Copyright (c) 2017-2018 VoiceExpert Squall
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_AUXPOW_H
#define BITCOIN_AUXPOW_H

#include "wallet.h"


class CAuxPow : public CMerkleTx
{
public:
    CAuxPow(const CTransaction& txIn) : CMerkleTx(txIn)
    {
    }

    CAuxPow() :CMerkleTx()
    {
    }

    // Merkle branch with root vchAux
    // root must be present inside the coinbase
    std::vector<uint256> vChainMerkleBranch;
    // Index of chain in chains merkle tree
    unsigned int nChainIndex;
    CBlockHeader parentBlockHeader;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(*(CMerkleTx*)this);
        READWRITE(vChainMerkleBranch);
        READWRITE(nChainIndex);
        // Always serialize the saved parent block as header so that the size of CAuxPow
        // is consistent.
        READWRITE(parentBlockHeader);
    }

    bool Check(uint256 hashAuxBlock, int nChainID);

    uint256 GetParentBlockHash()
    {
        return parentBlockHeader.GetHash();
    }
};

template<typename Stream> void SerReadWrite(Stream& s, boost::shared_ptr<CAuxPow>& pobj, int nType, int nVersion, CSerActionSerialize ser_action)
{
    if (nVersion & BLOCK_VERSION_AUXPOW) {
        ::Serialize(s, *pobj, nType, nVersion);
    }
}

template<typename Stream> void SerReadWrite(Stream& s, boost::shared_ptr<CAuxPow>& pobj, int nType, int nVersion, CSerActionUnserialize ser_action)
{
    if (nVersion & BLOCK_VERSION_AUXPOW) {
        pobj.reset(new CAuxPow());
        ::Unserialize(s, *pobj, nType, nVersion);
    } else
        pobj.reset();
}


class CSizeComputer;
class CDataStream;
class CAutoFile;

template void SerReadWrite<CSizeComputer>(CSizeComputer& s, boost::shared_ptr<CAuxPow>& pobj, int nType, int nVersion, CSerActionSerialize ser_action);
template void SerReadWrite<CDataStream>(CDataStream& s, boost::shared_ptr<CAuxPow>& pobj, int nType, int nVersion, CSerActionSerialize ser_action);
template void SerReadWrite<CAutoFile>(CAutoFile& s, boost::shared_ptr<CAuxPow>& pobj, int nType, int nVersion, CSerActionSerialize ser_action);

//template void SerReadWrite<CSizeComputer>(CSizeComputer& s, boost::shared_ptr<CAuxPow>& pobj, int nType, int nVersion, CSerActionUnserialize ser_action);
template void SerReadWrite<CDataStream>(CDataStream& s, boost::shared_ptr<CAuxPow>& pobj, int nType, int nVersion, CSerActionUnserialize ser_action);
template void SerReadWrite<CAutoFile>(CAutoFile& s, boost::shared_ptr<CAuxPow>& pobj, int nType, int nVersion, CSerActionUnserialize ser_action);

extern void RemoveMergedMiningHeader(std::vector<unsigned char>& vchAux);

#endif
