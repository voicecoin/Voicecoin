// Copyright (c) 2017-2018 VoiceExpert Squall
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the GPL3 software license, see the accompanying
// file COPYING or http://www.gnu.org/licenses/gpl.html.

#include "chain.h"
#include "auxpow.h"
#include "txdb.h"

using namespace std;

/**
 * CChain implementation
 */
void CChain::SetTip(CBlockIndex *pindex) {
    if (pindex == NULL) {
        vChain.clear();
        return;
    }
    vChain.resize(pindex->nHeight + 1);
    while (pindex && vChain[pindex->nHeight] != pindex) {
        vChain[pindex->nHeight] = pindex;
        pindex = pindex->pprev;
    }
}

CBlockLocator CChain::GetLocator(const CBlockIndex *pindex) const {
    int nStep = 1;
    std::vector<uint256> vHave;
    vHave.reserve(32);

    if (!pindex)
        pindex = Tip();
    while (pindex) {
        vHave.push_back(pindex->GetBlockHash());
        // Stop when we have added the genesis block.
        if (pindex->nHeight == 0)
            break;
        // Exponentially larger steps back, plus the genesis block.
        int nHeight = std::max(pindex->nHeight - nStep, 0);
        if (Contains(pindex)) {
            // Use O(1) CChain index if possible.
            pindex = (*this)[nHeight];
        } else {
            // Otherwise, use O(log n) skiplist.
            pindex = pindex->GetAncestor(nHeight);
        }
        if (vHave.size() > 10)
            nStep *= 2;
    }

    return CBlockLocator(vHave);
}

const CBlockIndex *CChain::FindFork(const CBlockIndex *pindex) const {
    if (pindex->nHeight > Height())
        pindex = pindex->GetAncestor(Height());
    while (pindex && !Contains(pindex))
        pindex = pindex->pprev;
    return pindex;
}

std::string CDiskBlockIndex::ToString() const
{
    std::string str = "CDiskBlockIndex(";
    str += CBlockIndex::ToString();
    str += strprintf("\n                hashBlock=%s, hashPrev=%s, hashParentBlock=%s)",
        GetBlockHash().ToString(),
        hashPrev.ToString(),
        (auxpow.get() != NULL) ? auxpow->GetParentBlockHash().ToString() : "-");
    return str;
}

CBlockHeader CBlockIndex::GetBlockHeader(const std::map<uint256, boost::shared_ptr<CAuxPow> >& mapDirtyAuxPow) const
{
    CBlockHeader block;

    if (nVersion & BLOCK_VERSION_AUXPOW) {
        bool foundInDirty = false;
        {
            LOCK(cs_main);
            std::map<uint256, boost::shared_ptr<CAuxPow> >::const_iterator it = mapDirtyAuxPow.find(*phashBlock);
            if (it != mapDirtyAuxPow.end()) {
                block.auxpow = it->second;
                foundInDirty = true;
            }
        }
        if (!foundInDirty) {
            CDiskBlockIndex diskblockindex;
            // auxpow is not in memory, load CDiskBlockHeader
            // from database to get it

            pblocktree->ReadDiskBlockIndex(*phashBlock, diskblockindex);
            block.auxpow = diskblockindex.auxpow;
        }
    }

    block.nVersion       = nVersion;
    if (pprev)
        block.hashPrevBlock = pprev->GetBlockHash();
    block.hashMerkleRoot = hashMerkleRoot;
    block.nTime          = nTime;
    block.nBits          = nBits;
    block.nNonce         = nNonce;
    block.nFlags         = nFlags;
    return block;
}
