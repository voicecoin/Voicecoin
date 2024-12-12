// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the GPL3 software license, see the accompanying
// file COPYING or http://www.gnu.org/licenses/gpl.html.

#include "checkpoints.h"

#include "chainparams.h"
#include "key.h"
#include "main.h"
#include "pubkey.h"
#include "timedata.h"
#include "txdb.h"
#include "uint256.h"

#include <stdint.h>

#include <boost/foreach.hpp>


/** Synchronized checkpoint public key */
std::string strMasterPubKey = "";

namespace Checkpoints {

    /**
     * How many times we expect transactions after the last checkpoint to
     * be slower. This number is a compromise, as it can't be accurate for
     * every system. When reindexing from a fast disk with a slow CPU, it
     * can be up to 20, while when downloading from a slow network with a
     * fast multicore CPU, it won't be much higher than 1.
     */
    static const double SIGCHECK_VERIFICATION_FACTOR = 5.0;

    bool fEnabled = true;

    bool CheckBlock(int nHeight, const uint256& hash)
    {
        if (!fEnabled)
            return true;

        const MapCheckpoints& checkpoints = *Params().Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    //! Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex, bool fSigchecks) {
        if (pindex==NULL)
            return 0.0;

        int64_t nNow = time(NULL);

        double fSigcheckVerificationFactor = fSigchecks ? SIGCHECK_VERIFICATION_FACTOR : 1.0;
        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkpoint, and
        // fSigcheckVerificationFactor per transaction after.

        const CCheckpointData &data = Params().Checkpoints();

        if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
            double nCheapBefore = pindex->nChainTx;
            double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
            double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore;
            fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
        } else {
            double nCheapBefore = data.nTransactionsLastCheckpoint;
            double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
            double nExpensiveAfter = (nNow - pindex->GetBlockTime())/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
            fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
        }

        return fWorkBefore / (fWorkBefore + fWorkAfter);
    }

    int GetTotalBlocksEstimate()
    {
        if (!fEnabled)
            return 0;

        const MapCheckpoints& checkpoints = *Params().Checkpoints().mapCheckpoints;

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint()
    {
        if (!fEnabled)
            return NULL;

        const MapCheckpoints& checkpoints = *Params().Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            BlockMap::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }

    uint256 GetLatestHardenedCheckpoint()
    {
        const MapCheckpoints& checkpoints = *Params().Checkpoints().mapCheckpoints;
        return (checkpoints.rbegin()->second);
    }

} // namespace Checkpoints



namespace CheckpointsSync {

// ppcoin: synchronized checkpoint (centrally broadcasted)
uint256 hashSyncCheckpoint = 0;
uint256 hashPendingCheckpoint = 0;
CSyncCheckpoint checkpointMessage;
CSyncCheckpoint checkpointMessagePending;
uint256 hashInvalidCheckpoint = 0;

// ppcoin: only descendant of current sync-checkpoint is allowed
bool ValidateSyncCheckpoint(uint256 hashCheckpoint)
{
    if (!mapBlockIndex.count(hashSyncCheckpoint))
        return error("ValidateSyncCheckpoint: block index missing for current sync-checkpoint %s", hashSyncCheckpoint.ToString());
    if (!mapBlockIndex.count(hashCheckpoint))
        return error("ValidateSyncCheckpoint: block index missing for received sync-checkpoint %s", hashCheckpoint.ToString());

    CBlockIndex* pindexSyncCheckpoint = mapBlockIndex[hashSyncCheckpoint];
    CBlockIndex* pindexCheckpointRecv = mapBlockIndex[hashCheckpoint];

    if (pindexCheckpointRecv->nHeight <= pindexSyncCheckpoint->nHeight)
    {
        // Received an older checkpoint, trace back from current checkpoint
        // to the same height of the received checkpoint to verify
        // that current checkpoint should be a descendant block
        if (!chainActive.Contains(pindexCheckpointRecv))
        {
            hashInvalidCheckpoint = hashCheckpoint;
            return error("ValidateSyncCheckpoint: new sync-checkpoint %s is conflicting with current sync-checkpoint %s", hashCheckpoint.ToString(), hashSyncCheckpoint.ToString());
        }
        return false; // ignore older checkpoint
    }

    // Received checkpoint should be a descendant block of the current
    // checkpoint. Trace back to the same height of current checkpoint
    // to verify.
    CBlockIndex* pindex = pindexCheckpointRecv;
    while (pindex->nHeight > pindexSyncCheckpoint->nHeight)
        if (!(pindex = pindex->pprev))
            return error("ValidateSyncCheckpoint: pprev2 null - block index structure failure");
    if (pindex->GetBlockHash() != hashSyncCheckpoint)
    {
        hashInvalidCheckpoint = hashCheckpoint;
        return error("ValidateSyncCheckpoint: new sync-checkpoint %s is not a descendant of current sync-checkpoint %s", hashCheckpoint.ToString(), hashSyncCheckpoint.ToString());
    }
    return true;
}

bool WriteSyncCheckpoint(const uint256& hashCheckpoint)
{
    if (!pblocktree->WriteSyncCheckpoint(hashCheckpoint))
    {
        return error("WriteSyncCheckpoint(): failed to write to txdb sync checkpoint %s", hashCheckpoint.ToString());
    }
    FlushStateToDisk();
    hashSyncCheckpoint = hashCheckpoint;
    return true;
}

bool AcceptPendingSyncCheckpoint()
{
    if (strMasterPubKey == "") return false;  // no public key == no checkpoints

    LOCK(cs_main);
    bool havePendingCheckpoint = hashPendingCheckpoint != 0 && mapBlockIndex.count(hashPendingCheckpoint);
    if (!havePendingCheckpoint)
        return false;

    if (!ValidateSyncCheckpoint(hashPendingCheckpoint))
    {
        hashPendingCheckpoint = 0;
        checkpointMessagePending.SetNull();
        return false;
    }

    if (!chainActive.Contains(mapBlockIndex[hashPendingCheckpoint]))
        return false;

    // voicecoin: checkpoint needs to be a block with 32 confirmation
    if (mapBlockIndex[hashPendingCheckpoint]->nHeight > chainActive.Height() - 32)
        return false;

    if (!WriteSyncCheckpoint(hashPendingCheckpoint))
        return error("AcceptPendingSyncCheckpoint(): failed to write sync checkpoint %s", hashPendingCheckpoint.ToString());
    hashPendingCheckpoint = 0;
    checkpointMessage = checkpointMessagePending;
    checkpointMessagePending.SetNull();
    LogPrintf("AcceptPendingSyncCheckpoint : sync-checkpoint at %s\n", hashSyncCheckpoint.ToString());
    // relay the checkpoint
    if (!checkpointMessage.IsNull())
    {
        BOOST_FOREACH(CNode* pnode, vNodes)
            checkpointMessage.RelayTo(pnode);
    }
    return true;
}


// Automatically select a suitable sync-checkpoint
uint256 AutoSelectSyncCheckpoint()
{
    static int32_t  s_depth = -1;
    static uint32_t s_slots, s_node_no;
    if (s_depth < 0)
    {
        s_depth   = GetArg("-checkpointdepth", 174 * 5); // default is 5 days backward deep
        s_slots   = GetArg("-checkpointslots", 1); // quantity of check slots, def=1
        s_node_no = GetArg("-checkpointnode", 0); // Number of current slot,  def=0
    }

    const CBlockIndex *pindex = chainActive.Tip();

    // Get hash of current block stamp in specific depth
    for (int32_t i = 0; i < s_depth; i++)
        pindex = pindex->pprev;

    const CBlockIndex *rc = pindex;

    // Get H-selector from checkpointed stable area +32+6 deeper
    for (int i = 0; i < 38; i++)
        pindex = pindex->pprev;

    uint256 h = pindex->GetBlockHash();

    // Preserve analyze current printer node from public block hash
    uint256 hx = Hash(CSyncCheckpoint::strMasterPrivKey.begin(), CSyncCheckpoint::strMasterPrivKey.end(), h.GetDataPtr(), h.GetDataPtr() + 256 / 32);

    return (hx.GetDataPtr()[0] % s_slots == s_node_no) ? rc->GetBlockHash() : 0;
#if 0
    // Proof-of-work blocks are immediately checkpointed
    // to defend against 51% attack which rejects other miners block

    // Select the last proof-of-work block
    const CBlockIndex *pindex = GetLastBlockIndex(pindexBest, false);
    // Search forward for a block within max span and maturity window
    while (pindex->pnext && (pindex->GetBlockTime() + CHECKPOINT_MAX_SPAN <= pindexBest->GetBlockTime() || pindex->nHeight + std::min(6, nCoinbaseMaturity - 20) <= pindexBest->nHeight))
        pindex = pindex->pnext;
    return pindex->GetBlockHash();
#endif
}

// Check against synchronized checkpoint
bool CheckSync(const CBlockIndex* pindexNew)
{
    if (strMasterPubKey == "") return true;  // no public key == no checkpoints

    LOCK(cs_main);
    assert(pindexNew != NULL);
    if (pindexNew->nHeight == 0) return true;                // genesis cannot be checked against previous block
    const uint256& hashBlock = pindexNew->GetBlockHash();
    int nHeight = pindexNew->nHeight;

    // sync-checkpoint should always be accepted block
    assert(mapBlockIndex.count(hashSyncCheckpoint));
    const CBlockIndex* pindexSync = mapBlockIndex[hashSyncCheckpoint];
    assert(chainActive.Contains(pindexSync));

    if (nHeight > pindexSync->nHeight)
    {
        // trace back to first block in our chainActive
        const CBlockIndex* pindex = pindexNew;
        while (pindex->nHeight > pindexSync->nHeight && !chainActive.Contains(pindex))
            if (!(pindex = pindex->pprev))
                return error("CheckSync: pprev null - block index structure failure");

        // at this point we could have:
        // 1. found block in our blockchain
        // 2. reached pindexSync->nHeight without finding it
        if (!chainActive.Contains(pindex))
            return false; // only descendant of sync-checkpoint can pass check
    }
    if (nHeight == pindexSync->nHeight && hashBlock != hashSyncCheckpoint)
        return false; // same height with sync-checkpoint
    if (nHeight < pindexSync->nHeight && !mapBlockIndex.count(hashBlock))
        return false; // lower height than sync-checkpoint

    return true;
}

// ppcoin: reset synchronized checkpoint to last hardened checkpoint
bool ResetSyncCheckpoint()
{
    LOCK(cs_main);
    if (!WriteSyncCheckpoint(Params().HashGenesisBlock()))
        return error("ResetSyncCheckpoint: failed to write sync checkpoint %s", Params().HashGenesisBlock().ToString());
    LogPrintf("ResetSyncCheckpoint: sync-checkpoint reset to %s\n", hashSyncCheckpoint.ToString());
    return true;
}

bool SetCheckpointPrivKey(std::string strPrivKey)
{
    // Test signing a sync-checkpoint with genesis block
    CSyncCheckpoint checkpoint;
    checkpoint.hashCheckpoint = Params().HashGenesisBlock();
    CDataStream sMsg(SER_NETWORK, PROTOCOL_VERSION);
    sMsg << (CUnsignedSyncCheckpoint)checkpoint;
    checkpoint.vchMsg = std::vector<unsigned char>(sMsg.begin(), sMsg.end());

    std::vector<unsigned char> vchPrivKey = ParseHex(strPrivKey);
    CKey key;
    if(!key.SetPrivKey(CPrivKey(vchPrivKey.begin(), vchPrivKey.end()), false))
        LogPrintf("SetCheckpointPrivKey(): failed at key.SetPrivKey\n"); // if key is not correct openssl may crash
    if (!key.Sign(Hash(checkpoint.vchMsg.begin(), checkpoint.vchMsg.end()), checkpoint.vchSig))
        return false;

    // Test signing successful, proceed
    CSyncCheckpoint::strMasterPrivKey = strPrivKey;
    return true;
}

bool SendSyncCheckpoint(uint256 hashCheckpoint)
{
    if (hashCheckpoint == 0)
        return true; // don't send dummy checkpoint

    CSyncCheckpoint checkpoint;
    checkpoint.hashCheckpoint = hashCheckpoint;
    CDataStream sMsg(SER_NETWORK, PROTOCOL_VERSION);
    sMsg << (CUnsignedSyncCheckpoint)checkpoint;
    checkpoint.vchMsg = std::vector<unsigned char>(sMsg.begin(), sMsg.end());

    if (CSyncCheckpoint::strMasterPrivKey.empty())
        return error("SendSyncCheckpoint: Checkpoint master key unavailable.");
    std::vector<unsigned char> vchPrivKey = ParseHex(CSyncCheckpoint::strMasterPrivKey);
    CKey key;
    key.SetPrivKey(CPrivKey(vchPrivKey.begin(), vchPrivKey.end()), false); // if key is not correct openssl may crash
    if (!key.Sign(Hash(checkpoint.vchMsg.begin(), checkpoint.vchMsg.end()), checkpoint.vchSig))
        return error("SendSyncCheckpoint: Unable to sign checkpoint, check private key?");

    if(!checkpoint.ProcessSyncCheckpoint())
    {
        LogPrintf("WARNING: SendSyncCheckpoint: Failed to process checkpoint.\n");
        return false;
    }

    // Relay checkpoint
    {
        LOCK(cs_vNodes);
        BOOST_FOREACH(CNode* pnode, vNodes)
            checkpoint.RelayTo(pnode);
    }
    return true;
}

// Is the sync-checkpoint too old?
bool IsSyncCheckpointTooOld(unsigned int nSeconds)
{
    if (strMasterPubKey == "") return false;  // no public key == no checkpoints

    LOCK(cs_main);
    // sync-checkpoint should always be accepted block
    assert(mapBlockIndex.count(hashSyncCheckpoint));
    const CBlockIndex* pindexSync = mapBlockIndex[hashSyncCheckpoint];
    //assert(chainActive.Contains(pindexSync));  //disabled for reconsiderblock function

    return (pindexSync->GetBlockTime() + nSeconds < GetAdjustedTime());
}

}  // namespace CheckpointsSync




// voicecoin: sync-checkpoint master key
std::string CSyncCheckpoint::strMasterPrivKey = "";

// ppcoin: verify signature of sync-checkpoint message
bool CSyncCheckpoint::CheckSignature()
{
    CPubKey key(ParseHex(strMasterPubKey));
    if (!key.Verify(Hash(vchMsg.begin(), vchMsg.end()), vchSig))
        return error("CSyncCheckpoint::CheckSignature() : verify signature failed");

    // Now unserialize the data
    CDataStream sMsg(vchMsg, SER_NETWORK, PROTOCOL_VERSION);
    sMsg >> *(CUnsignedSyncCheckpoint*)this;
    return true;
}

// ppcoin: process synchronized checkpoint
bool CSyncCheckpoint::ProcessSyncCheckpoint()
{
    if (strMasterPubKey == "") return false;  // no public key == no checkpoints

    if (!CheckSignature())
        return false;

    LOCK(cs_main);
    if (!mapBlockIndex.count(hashCheckpoint))
    {
        LogPrintf("Missing headers for received sync-checkpoint %s\n", hashCheckpoint.ToString());
        return false;
    }

    if (!CheckpointsSync::ValidateSyncCheckpoint(hashCheckpoint))
        return false;

    bool pass = chainActive.Contains(mapBlockIndex[hashCheckpoint]) &&
                mapBlockIndex[hashCheckpoint]->nHeight <= chainActive.Height() - 32;
    if (!pass)
    {
        // We haven't received the checkpoint chain, keep the checkpoint as pending
        CheckpointsSync::hashPendingCheckpoint = hashCheckpoint;
        CheckpointsSync::checkpointMessagePending = *this;
        LogPrintf("ProcessSyncCheckpoint: pending for sync-checkpoint %s\n", hashCheckpoint.ToString());
        return false;
    }

    if (!CheckpointsSync::WriteSyncCheckpoint(hashCheckpoint))
        return error("ProcessSyncCheckpoint(): failed to write sync checkpoint %s", hashCheckpoint.ToString());
    CheckpointsSync::checkpointMessage = *this;
    CheckpointsSync::hashPendingCheckpoint = 0;
    CheckpointsSync::checkpointMessagePending.SetNull();
    LogPrintf("ProcessSyncCheckpoint: sync-checkpoint at height=%d hash=%s\n", mapBlockIndex[hashCheckpoint]->nHeight, hashCheckpoint.ToString());
    return true;
}

std::string CUnsignedSyncCheckpoint::ToString() const
{
    return strprintf(
                "CSyncCheckpoint(\n"
                "    nVersion       = %d\n"
                "    hashCheckpoint = %s\n"
                ")\n", nVersion, hashCheckpoint.ToString().c_str());
}

void CUnsignedSyncCheckpoint::print() const
{
    LogPrintf("%s", ToString());
}

uint256 CSyncCheckpoint::GetHash() const
{
    return SerializeHash(*this);
}

bool CSyncCheckpoint::RelayTo(CNode *pnode) const
{
    // returns true if wasn't already sent
    if (pnode->hashCheckpointKnown != hashCheckpoint)
    {
        pnode->hashCheckpointKnown = hashCheckpoint;
        pnode->PushMessage("checkpoint", *this);
        return true;
    }
    return false;
}
