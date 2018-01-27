// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the GPL3 software license, see the accompanying
// file COPYING or http://www.gnu.org/licenses/gpl.html.

#include "pow.h"

#include "bignum.h"
#include "chain.h"
#include "chainparams.h"
#include "primitives/block.h"
#include "auxpow.h"
#include "uint256.h"
#include "util.h"

#include <algorithm>

// ppcoin: find last block index up to pindex
const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake)
{
    while (pindex && pindex->pprev && (pindex->IsProofOfStake() != fProofOfStake))
        pindex = pindex->pprev;
    return pindex;
}

unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast,const CBlockHeader *pblock, bool fProofOfStake)
{
    const CChainParams& params = Params();
    unsigned int nProofOfWorkLimit = params.ProofOfWorkLimit().GetCompact();
    int nInterval = params.TargetTimespan()/params.TargetSpacing();

    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;

    // Only change once per difficulty adjustment interval
    if ((pindexLast->nHeight+1) % nInterval != 0)
    {
	//LogPrintf("CalculateNextWorkRequired [%u]\n", pindexLast->nBits);
        return pindexLast->nBits;
    }
    
    int blockstogoback = nInterval-1;
    if ((pindexLast->nHeight+1) != nInterval)
        blockstogoback = nInterval;

    // Go back by what we want to be 14 days worth of blocks
    const CBlockIndex* pindexFirst = pindexLast;
    for (int i = 0; pindexFirst && i < blockstogoback; i++)
        pindexFirst = pindexFirst->pprev;

    assert(pindexFirst);

    return CalculateNextWorkRequired(pindexLast, pindexFirst);

}

unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, const CBlockIndex* pindexFirst)
{
    const CChainParams& params = Params();
    int64_t nFirstBlockTime = pindexFirst->GetBlockTime();
    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    LogPrintf("CalculateNextWorkRequired [%ld] [%ld] [%ld] [%ld] [%d,%d]\n", pindexLast->GetBlockTime(),nFirstBlockTime,nActualTimespan,params.TargetTimespan(),pindexLast->nHeight,pindexFirst->nHeight);
    if (nActualTimespan < params.TargetTimespan()/4)
        nActualTimespan = params.TargetTimespan()/4;
    if (nActualTimespan > params.TargetTimespan()*4)
        nActualTimespan = params.TargetTimespan()*4;

    // Retarget
    uint256 bnNew;
    uint256 bnOld;
    bnNew.SetCompact(pindexLast->nBits);
    bnOld = bnNew;
    // Litecoin: intermediate uint256 can overflow by 1 bit
    bool fShift = bnNew.bits() > 235;
    if (fShift)
        bnNew >>= 1;
    bnNew *= nActualTimespan;
    bnNew /= params.TargetTimespan();
    if (fShift)
        bnNew <<= 1;

    const uint256 bnPowLimit = params.ProofOfWorkLimit();
    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    unsigned int nPow = bnNew.GetCompact();
    LogPrintf("CalculateNextWorkRequired [%u] ReTargert\n", nPow);
    return nPow;
}


bool CheckProofOfWork(uint256 hash, unsigned int nBits)
{
    bool fNegative;
    bool fOverflow;
    uint256 bnTarget;

    if (Params().SkipProofOfWorkCheck())
       return true;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > Params().ProofOfWorkLimit())
        return error("CheckProofOfWork() : nBits below minimum work");

    // Check proof of work matches claimed amount
    if (hash > bnTarget)
        return error("CheckProofOfWork() : hash doesn't match nBits\nhash = %s\nbnTa = %s", hash.GetHex(), bnTarget.GetHex());

    return true;
}

uint256 GetBlockTrust(const CBlockIndex& block)
{
    uint256 bnTarget;
    bool fNegative;
    bool fOverflow;
    bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);
    if (fNegative || fOverflow || bnTarget == 0)
        return 0;
    // We need to compute 2**256 / (bnTarget+1), but we can't represent 2**256
    // as it's too large for a uint256. However, as 2**256 is at least as large
    // as bnTarget+1, it is equal to ((2**256 - bnTarget - 1) / (bnTarget+1)) + 1,
    // or ~bnTarget / (nTarget+1) + 1.
    return block.IsProofOfStake() ? (~bnTarget / (bnTarget + 1)) + 1 : 1;
}

bool CheckBlockProofOfWork(const CBlockHeader *pblock)
{
    // There's an issue with blocks prior to the auxpow fork reporting an invalid chain ID.
    // As no version earlier than the 0.10 client a) has version 5+ blocks and b)
    //	has auxpow, anything that isn't a version 5+ block can be checked normally.

    if (pblock->GetBlockVersion() > 4)
    {
        if (!Params().AllowMinDifficultyBlocks() && (pblock->nVersion & BLOCK_VERSION_AUXPOW && pblock->GetChainID() != AUXPOW_CHAIN_ID))
            return error("CheckBlockProofOfWork() : block does not have our chain ID");

        if (pblock->auxpow.get() != NULL)
        {
            if (!pblock->auxpow->Check(pblock->GetHash(), pblock->GetChainID()))
                return error("CheckBlockProofOfWork() : AUX POW is not valid");
            // Check proof of work matches claimed amount
            if (!CheckProofOfWork(pblock->auxpow->GetParentBlockHash(), pblock->nBits))
                return error("CheckBlockProofOfWork() : AUX proof of work failed");
        }
        else
        {
            // Check proof of work matches claimed amount
            if (!CheckProofOfWork(pblock->GetHash(), pblock->nBits))
                return error("CheckBlockProofOfWork() : proof of work failed");
        }
    }
    else
    {
        // Check proof of work matches claimed amount
        if (!CheckProofOfWork(pblock->GetHash(), pblock->nBits))
            return error("CheckBlockProofOfWork() : proof of work failed");
    }
    return true;
}
