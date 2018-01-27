// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the GPL3 software license, see the accompanying
// file COPYING or http://www.gnu.org/licenses/gpl.html.

#include "chainparams.h"

#include "random.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

using namespace std;
using namespace boost::assign;

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"

/**
 * Main network
 */

//! Convert the pnSeeds6 array into usable address objects.
static void convertSeed6(std::vector<CAddress> &vSeedsOut, const SeedSpec6 *data, unsigned int count)
{
    // It'll only connect to one or two seed nodes because once it connects,
    // it'll get a pile of addresses with newer timestamps.
    // Seed nodes are given a random 'last seen time' of between one and two
    // weeks ago.
    const int64_t nOneWeek = 7*24*60*60;
    for (unsigned int i = 0; i < count; i++)
    {
        struct in6_addr ip;
        memcpy(&ip, data[i].addr, sizeof(ip));
        CAddress addr(CService(ip, data[i].port));
        addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

static Checkpoints::MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        ( 0,     uint256("0x01"))
        ;

static const Checkpoints::CCheckpointData data = {
        &mapCheckpoints,
        1455207714, // * UNIX timestamp of last checkpoint block
        365081,     // * total number of transactions between genesis and last checkpoint
                    //   (the tx=... number in the SetBestChain debug.log lines)
        500.0       // * estimated number of transactions per day after checkpoint
    };

static Checkpoints::MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        ( 0, uint256("0x01"))
        ;
static const Checkpoints::CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        0,
        0,
        0
    };

static Checkpoints::MapCheckpoints mapCheckpointsRegtest =
        boost::assign::map_list_of
        ( 0, uint256("0x01"))
        ;
static const Checkpoints::CCheckpointData dataRegtest = {
        &mapCheckpointsRegtest,
        0,
        0,
        0
    };

class CMainParams : public CChainParams {
public:
    CMainParams() {
        networkID = CBaseChainParams::MAIN;
        strNetworkID = "main";
        /** 
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0] = 0xc9;
        pchMessageStart[1] = 0xc8;
        pchMessageStart[2] = 0xc7;
        pchMessageStart[3] = 0xc6;
        nDefaultPort = 9876;
        bnProofOfWorkLimit = ~uint256(0) >> 4;
        bnInitialHashTarget = ~uint256(0) >> 4;
        nEnforceBlockUpgradeMajority = 750;
        nRejectBlockOutdatedMajority = 950;
        nToCheckBlockUpgradeMajority = 1000;
        nMinerThreads = 0;
	nTargetTimespan = 7 * 24 * 60 * 60; // one week
        nTargetSpacing = 2.5 * 60 ;           // 2.5 minutes. affects network code only

        // ppcoin: PoS spacing = nStakeTargetSpacing
        //         PoW spacing = depends on how much PoS block are between last two PoW blocks, with maximum value = nTargetSpacingMax
        nCoinbaseMaturity = 100;                      // coinbase transaction outputs can only be spent after this number of new blocks
        nStakeTargetSpacing = 10 * 60;                // 10 minutes
        nTargetSpacingMax = 12 * nStakeTargetSpacing; // 2 hours
        nStakeMinAge = 60 * 60 * 24 * 30;             // minimum age for coin age
        nStakeMaxAge = 60 * 60 * 24 * 90;             // stake age of full weight
        nStakeModifierInterval = 6 * 60 * 60;         // time to elapse before new modifier is computed
        nMaxTipAge = 24 * 60 * 60;

        const char* pszTimestamp = "2018 Voice Coin Team created this coin at 1/27";
        std::vector<CTxIn> vin;
        vin.resize(1);
        vin[0].scriptSig = CScript() << 486604799 << CScriptNum(9999) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        std::vector<CTxOut> vout;
        vout.resize(1);
        vout[0].nValue = 0;
        vout[0].scriptPubKey.clear();
	CMutableTransaction txNew(1, 1516936775, vin, vout, 0);
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = 1;
	genesis.nTime    = 1516936776;
        genesis.nBits    = bnProofOfWorkLimit.GetCompact();
        genesis.nNonce   = 0u;
			
        hashGenesisBlock = genesis.GetHash();
        
#if 1
	while (hashGenesisBlock > bnInitialHashTarget)
	{
	    genesis.nNonce++;
	    hashGenesisBlock = genesis.GetHash();
	}
#endif
        LogPrintf("generateone CMainParamshashGenesisBlock %s %s %d\n",hashGenesisBlock.GetHex().c_str(),bnInitialHashTarget.GetHex().c_str(),
		genesis.nNonce);
	

       // assert(hashGenesisBlock == uint256("0x01"));
       // assert(genesis.hashMerkleRoot == uint256("0x01"));

        vSeeds.push_back(CDNSSeedData("voicecoin.com", "seed.voicecoin.com"));
        vSeeds.push_back(CDNSSeedData("voicecoin.net", "seed.voicecoin.net"));

        base58Prefixes[PUBKEY_ADDRESS] = list_of(132);     // voicecoin: addresses begin with 'v'
        base58Prefixes[SCRIPT_ADDRESS] = list_of(115);;    // voicecoin: addresses begin with 'o'
        base58Prefixes[SECRET_KEY] = list_of(34);          // voicecoin: addresses begin with 'i'
        base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x84)(0x88)(0xB2)(0x1E);  
        base58Prefixes[EXT_SECRET_KEY] = list_of(0x84)(0x88)(0xAD)(0xE4);  

        fRequireRPCPassword = true;
        fMiningRequiresPeers = false;
        fAllowMinDifficultyBlocks = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fSkipProofOfWorkCheck = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        nBIP34Height = 212806;
        nBIP65Height = 212920;
        nBIP66Height = 212806;
        nMMHeight = 219809;
    }

    const Checkpoints::CCheckpointData& Checkpoints() const 
    {
        return data;
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CMainParams {
public:
    CTestNetParams() {
        networkID = CBaseChainParams::TESTNET;
        strNetworkID = "test";
        pchMessageStart[0] = 0xc9;
        pchMessageStart[1] = 0xc8;
        pchMessageStart[2] = 0xc7;
        pchMessageStart[3] = 0xc6;
        nDefaultPort = 9776;
        bnProofOfWorkLimit = ~uint256(0) >> 28;
        bnInitialHashTarget = ~uint256(0) >> 29;
        nEnforceBlockUpgradeMajority = 51;
        nRejectBlockOutdatedMajority = 75;
        nToCheckBlockUpgradeMajority = 100;
        nMinerThreads = 0;
        nCoinbaseMaturity = 1;
        nStakeMinAge = 60 * 60 * 24;        // test net min age is 1 day
        nStakeModifierInterval = 60 * 20;   // test net modifier interval is 20 minutes
        nMaxTipAge = 0x7fffffff;

        //! Modify the testnet genesis block so the timestamp is valid for a later start.
        genesis.nBits = bnProofOfWorkLimit.GetCompact();
        genesis.nNonce = 18330017;
        hashGenesisBlock = genesis.GetHash();
       // assert(hashGenesisBlock == uint256("0x0000000810da236a5c9239aa1c49ab971de289dbd41d08c4120fc9c8920d2212"));

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("testvoicecoin.com", "seed.testvoicecoin.com"));

        std::vector<unsigned char> pubkey_address = list_of(135);
        base58Prefixes[PUBKEY_ADDRESS] = pubkey_address;
        std::vector<unsigned char> script_address = list_of(137);
        base58Prefixes[SCRIPT_ADDRESS] = script_address;
        std::vector<unsigned char> secret_key = list_of(140);
        base58Prefixes[SECRET_KEY] = secret_key;
        std::vector<unsigned char> ext_public_key = list_of(0x23)(0x35)(0x87)(0xCF);
        base58Prefixes[EXT_PUBLIC_KEY] = ext_public_key;
        std::vector<unsigned char> ext_secret_key = list_of(0x23)(0x35)(0x83)(0x94);
        base58Prefixes[EXT_SECRET_KEY] = ext_secret_key;

        convertSeed6(vFixedSeeds, pnSeed6_test, ARRAYLEN(pnSeed6_test));

        fRequireRPCPassword = true;
        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        nBIP34Height = 141;
        nBIP65Height = 368;
        nBIP66Height = 141;
        nMMHeight = 2189;
    }
    const Checkpoints::CCheckpointData& Checkpoints() const 
    {
        return dataTestnet;
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CTestNetParams {
public:
    CRegTestParams() {
        networkID = CBaseChainParams::REGTEST;
        strNetworkID = "regtest";
        pchMessageStart[0] = 0xc9;
        pchMessageStart[1] = 0xc8;
        pchMessageStart[2] = 0xc7;
        pchMessageStart[3] = 0xc6;

	base58Prefixes[PUBKEY_ADDRESS] = list_of(84);  
        base58Prefixes[SCRIPT_ADDRESS] = list_of(87);   
        base58Prefixes[SECRET_KEY] =     list_of(90);  
        base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x1C)(0x66)(0x77)(0xAA);
        base58Prefixes[EXT_SECRET_KEY] = list_of(0x1C)(0x66)(0x87)(0xBA);


        nEnforceBlockUpgradeMajority = 750;
        nRejectBlockOutdatedMajority = 950;
        nToCheckBlockUpgradeMajority = 1000;
        nMinerThreads = 1;
        bnProofOfWorkLimit = ~uint256(0) >> 1;

        nMaxTipAge = 24 * 60 * 60;
        hashGenesisBlock = genesis.GetHash();
        nDefaultPort = 9676;
       // assert(hashGenesisBlock == uint256("0x01"));

        vFixedSeeds.clear(); //! Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();  //! Regtest mode doesn't have any DNS seeds.

        fRequireRPCPassword = false;
        fMiningRequiresPeers = false;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;
    }
    const Checkpoints::CCheckpointData& Checkpoints() const 
    {
        return dataRegtest;
    }
};
static CRegTestParams regTestParams;

/**
 * Unit test
 */
class CUnitTestParams : public CMainParams, public CModifiableParams {
public:
    CUnitTestParams() {
        networkID = CBaseChainParams::UNITTEST;
        strNetworkID = "unittest";
        nDefaultPort = 9576;
        vFixedSeeds.clear(); //! Unit test mode doesn't have any fixed seeds.
        vSeeds.clear();  //! Unit test mode doesn't have any DNS seeds.

        fRequireRPCPassword = false;
        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fAllowMinDifficultyBlocks = false;
        fMineBlocksOnDemand = true;
    }

    const Checkpoints::CCheckpointData& Checkpoints() const 
    {
        // UnitTest share the same checkpoints as MAIN
        return data;
    }

    //! Published setters to allow changing values in unit test cases
    virtual void setEnforceBlockUpgradeMajority(int anEnforceBlockUpgradeMajority)  { nEnforceBlockUpgradeMajority=anEnforceBlockUpgradeMajority; }
    virtual void setRejectBlockOutdatedMajority(int anRejectBlockOutdatedMajority)  { nRejectBlockOutdatedMajority=anRejectBlockOutdatedMajority; }
    virtual void setToCheckBlockUpgradeMajority(int anToCheckBlockUpgradeMajority)  { nToCheckBlockUpgradeMajority=anToCheckBlockUpgradeMajority; }
    virtual void setDefaultConsistencyChecks(bool afDefaultConsistencyChecks)  { fDefaultConsistencyChecks=afDefaultConsistencyChecks; }
    virtual void setAllowMinDifficultyBlocks(bool afAllowMinDifficultyBlocks) {  fAllowMinDifficultyBlocks=afAllowMinDifficultyBlocks; }
    virtual void setSkipProofOfWorkCheck(bool afSkipProofOfWorkCheck) { fSkipProofOfWorkCheck = afSkipProofOfWorkCheck; }
};
static CUnitTestParams unitTestParams;


static CChainParams *pCurrentParams = 0;

CModifiableParams *ModifiableParams()
{
   assert(pCurrentParams);
   assert(pCurrentParams==&unitTestParams);
   return (CModifiableParams*)&unitTestParams;
}

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams &Params(CBaseChainParams::Network network) {
    switch (network) {
        case CBaseChainParams::MAIN:
            return mainParams;
        case CBaseChainParams::TESTNET:
            return testNetParams;
        case CBaseChainParams::REGTEST:
            return regTestParams;
        case CBaseChainParams::UNITTEST:
            return unitTestParams;
        default:
            assert(false && "Unimplemented network");
            return mainParams;
    }
}

void SelectParams(CBaseChainParams::Network network) {
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

bool SelectParamsFromCommandLine()
{
    CBaseChainParams::Network network = NetworkIdFromCommandLine();
    if (network == CBaseChainParams::MAX_NETWORK_TYPES)
        return false;

    SelectParams(network);
    return true;
}
