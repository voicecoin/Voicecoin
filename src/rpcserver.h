// Copyright (c) 2017-2018 VoiceExpert Squall
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the GPL3 software license, see the accompanying
// file COPYING or http://www.gnu.org/licenses/gpl.html.

#ifndef BITCOIN_RPCSERVER_H
#define BITCOIN_RPCSERVER_H

#include "amount.h"
#include "rpcprotocol.h"
#include "uint256.h"

#include <list>
#include <map>
#include <stdint.h>
#include <string>

#include <boost/function.hpp>

class CBlockIndex;
class CNetAddr;
class CWallet;

class JSONRequest
{
public:
    UniValue id;
    std::string strMethod;
    UniValue params;

    JSONRequest() { id = NullUniValue; }
    void parse(const UniValue& valRequest);
};

class AcceptedConnection
{
public:
    virtual ~AcceptedConnection() {}

    virtual std::iostream& stream() = 0;
    virtual std::string peer_address_to_string() const = 0;
    virtual void close() = 0;
};

/** Start RPC threads */
void StartRPCThreads();
/**
 * Alternative to StartRPCThreads for the GUI, when no server is
 * used. The RPC thread in this case is only used to handle timeouts.
 * If real RPC threads have already been started this is a no-op.
 */
void StartDummyRPCThread();
/** Stop RPC threads */
void StopRPCThreads();
/** Query whether RPC is running */
bool IsRPCRunning();

/** 
 * Set the RPC warmup status.  When this is done, all RPC calls will error out
 * immediately with RPC_IN_WARMUP.
 */
void SetRPCWarmupStatus(const std::string& newStatus);
/* Mark warmup as done.  RPC calls will be processed from now on.  */
void SetRPCWarmupFinished();

/* returns the current warmup state.  */
bool RPCIsInWarmup(std::string *statusOut);

/**
 * Type-check arguments; throws JSONRPCError if wrong type given. Does not check that
 * the right number of arguments are passed, just that any passed are the correct type.
 */
void RPCTypeCheck(const UniValue& params,
                  const std::list<UniValue::VType>& typesExpected, bool fAllowNull=false);
/*
  Check for expected keys/value types in an Object.
*/
void RPCTypeCheckObj(const UniValue& o,
    const std::map<std::string, UniValue::VType>& typesExpected,
    bool fAllowNull = false,
    bool fStrict = false);

/**
 * Run func nSeconds from now. Uses boost deadline timers.
 * Overrides previous timer <name> (if any).
 */
void RPCRunLater(const std::string& name, boost::function<void(void)> func, int64_t nSeconds);

//! Convert boost::asio address to CNetAddr
extern CNetAddr BoostAsioToCNetAddr(boost::asio::ip::address address);

typedef UniValue(*rpcfn_type)(const UniValue& params, bool fHelp);

class CRPCCommand
{
public:
    std::string category;
    std::string name;
    rpcfn_type actor;
    bool okSafeMode;
    bool threadSafe;
    bool reqWallet;
};

/**
 * Bitcoin RPC command dispatcher.
 */
class CRPCTable
{
private:
    std::map<std::string, const CRPCCommand*> mapCommands;
public:
    CRPCTable();
    const CRPCCommand* operator[](std::string name) const;
    std::string help(std::string name) const;

    /**
    * Returns a list of registered commands
    * @returns List of registered commands.
    */
    std::vector<std::string> listCommands() const;

    /**
     * Execute a method.
     * @param method   Method to execute
     * @param params   Array of arguments (JSON objects)
     * @returns Result of the call.
     * @throws an exception (UniValue) when an error happens.
     */
    UniValue execute(const std::string &method, const UniValue &params) const;
};

extern const CRPCTable tableRPC;

/**
 * Utilities: convert hex-encoded Values
 * (throws error if not hex).
 */
extern uint256 ParseHashV(const UniValue& v, std::string strName);
extern uint256 ParseHashO(const UniValue& o, std::string strKey);
extern std::vector<unsigned char> ParseHexV(const UniValue& v, std::string strName);
extern std::vector<unsigned char> ParseHexO(const UniValue& o, std::string strKey);

extern void InitRPCMining();
extern void ShutdownRPCMining();

extern int64_t nWalletUnlockTime;
extern CAmount AmountFromValue(const UniValue& value);
extern UniValue ValueFromAmount(const CAmount& amount);
extern double GetDifficulty(const CBlockIndex* blockindex = NULL);
extern std::string HelpRequiringPassphrase();
extern std::string HelpExampleCli(std::string methodname, std::string args);
extern std::string HelpExampleRpc(std::string methodname, std::string args);

extern void EnsureWalletIsUnlocked();
extern void relockWalletAfterDuration(CWallet *wallet, int64_t nSeconds);

extern UniValue getconnectioncount(const UniValue& params, bool fHelp); // in rpcnet.cpp
extern UniValue getpeerinfo(const UniValue& params, bool fHelp);
extern UniValue ping(const UniValue& params, bool fHelp);
extern UniValue addnode(const UniValue& params, bool fHelp);
extern UniValue getaddednodeinfo(const UniValue& params, bool fHelp);
extern UniValue getnettotals(const UniValue& params, bool fHelp);

extern UniValue dumpprivkey(const UniValue& params, bool fHelp); // in rpcdump.cpp
extern UniValue importprivkey(const UniValue& params, bool fHelp);
extern UniValue importaddress(const UniValue& params, bool fHelp);
extern UniValue dumpwallet(const UniValue& params, bool fHelp);
extern UniValue importwallet(const UniValue& params, bool fHelp);
extern UniValue reencodeoldprivkey(const UniValue& params, bool fHelp);

extern UniValue getgenerate(const UniValue& params, bool fHelp); // in rpcmining.cpp
extern UniValue setgenerate(const UniValue& params, bool fHelp);
extern UniValue getnetworkhashps(const UniValue& params, bool fHelp);
extern UniValue gethashespersec(const UniValue& params, bool fHelp);
extern UniValue getmininginfo(const UniValue& params, bool fHelp);
extern UniValue prioritisetransaction(const UniValue& params, bool fHelp);
extern UniValue getblocktemplate(const UniValue& params, bool fHelp);
extern UniValue submitblock(const UniValue& params, bool fHelp);
extern UniValue estimatefee(const UniValue& params, bool fHelp);
extern UniValue estimatepriority(const UniValue& params, bool fHelp);
extern UniValue getauxblock(const UniValue& params, bool fHelp);

extern UniValue getnewaddress(const UniValue& params, bool fHelp); // in rpcwallet.cpp
extern UniValue getaccountaddress(const UniValue& params, bool fHelp);
extern UniValue getrawchangeaddress(const UniValue& params, bool fHelp);
extern UniValue setaccount(const UniValue& params, bool fHelp);
extern UniValue getaccount(const UniValue& params, bool fHelp);
extern UniValue getaddressesbyaccount(const UniValue& params, bool fHelp);
extern UniValue sendtoaddress(const UniValue& params, bool fHelp);
extern UniValue signmessage(const UniValue& params, bool fHelp);
extern UniValue verifymessage(const UniValue& params, bool fHelp);
extern UniValue getreceivedbyaddress(const UniValue& params, bool fHelp);
extern UniValue getreceivedbyaccount(const UniValue& params, bool fHelp);
extern UniValue getbalance(const UniValue& params, bool fHelp);
extern UniValue getunconfirmedbalance(const UniValue& params, bool fHelp);
extern UniValue movecmd(const UniValue& params, bool fHelp);
extern UniValue sendfrom(const UniValue& params, bool fHelp);
extern UniValue sendmany(const UniValue& params, bool fHelp);
extern UniValue addmultisigaddress(const UniValue& params, bool fHelp);
extern UniValue createmultisig(const UniValue& params, bool fHelp);
extern UniValue listreceivedbyaddress(const UniValue& params, bool fHelp);
extern UniValue listreceivedbyaccount(const UniValue& params, bool fHelp);
extern UniValue listtransactions(const UniValue& params, bool fHelp);
extern UniValue listaddressgroupings(const UniValue& params, bool fHelp);
extern UniValue listaccounts(const UniValue& params, bool fHelp);
extern UniValue listsinceblock(const UniValue& params, bool fHelp);
extern UniValue gettransaction(const UniValue& params, bool fHelp);
extern UniValue backupwallet(const UniValue& params, bool fHelp);
extern UniValue keypoolrefill(const UniValue& params, bool fHelp);
extern UniValue walletpassphrase(const UniValue& params, bool fHelp);
extern UniValue walletpassphrasechange(const UniValue& params, bool fHelp);
extern UniValue walletlock(const UniValue& params, bool fHelp);
extern UniValue encryptwallet(const UniValue& params, bool fHelp);
extern UniValue validateaddress(const UniValue& params, bool fHelp);
extern UniValue getinfo(const UniValue& params, bool fHelp);
extern UniValue getwalletinfo(const UniValue& params, bool fHelp);
extern UniValue getblockchaininfo(const UniValue& params, bool fHelp);
extern UniValue getnetworkinfo(const UniValue& params, bool fHelp);
extern UniValue setmocktime(const UniValue& params, bool fHelp);

extern UniValue getrawtransaction(const UniValue& params, bool fHelp); // in rcprawtransaction.cpp
extern UniValue listunspent(const UniValue& params, bool fHelp);
extern UniValue lockunspent(const UniValue& params, bool fHelp);
extern UniValue listlockunspent(const UniValue& params, bool fHelp);
extern UniValue createrawtransaction(const UniValue& params, bool fHelp);
extern UniValue decoderawtransaction(const UniValue& params, bool fHelp);
extern UniValue decodescript(const UniValue& params, bool fHelp);
extern UniValue signrawtransaction(const UniValue& params, bool fHelp);
extern UniValue sendrawtransaction(const UniValue& params, bool fHelp);

extern UniValue getblockcount(const UniValue& params, bool fHelp); // in rpcblockchain.cpp
extern UniValue getbestblockhash(const UniValue& params, bool fHelp);
extern UniValue getdifficulty(const UniValue& params, bool fHelp);
extern UniValue settxfee(const UniValue& params, bool fHelp);
extern UniValue getmempoolinfo(const UniValue& params, bool fHelp);
extern UniValue getrawmempool(const UniValue& params, bool fHelp);
extern UniValue getblockhash(const UniValue& params, bool fHelp);
extern UniValue getblock(const UniValue& params, bool fHelp);
extern UniValue gettxoutsetinfo(const UniValue& params, bool fHelp);
extern UniValue gettxout(const UniValue& params, bool fHelp);
extern UniValue verifychain(const UniValue& params, bool fHelp);
extern UniValue getchaintips(const UniValue& params, bool fHelp);
extern UniValue invalidateblock(const UniValue& params, bool fHelp);
extern UniValue reconsiderblock(const UniValue& params, bool fHelp);
extern UniValue gettxlistfor(const UniValue& params, bool fHelp);

extern UniValue name_new(const UniValue& params, bool fHelp); // in namecoin.cpp
extern UniValue name_update(const UniValue& params, bool fHelp);
extern UniValue name_delete(const UniValue& params, bool fHelp);
extern UniValue sendtoname(const UniValue& params, bool fHelp);
extern UniValue name_list(const UniValue& params, bool fHelp);
extern UniValue name_scan(const UniValue& params, bool fHelp);
extern UniValue name_filter(const UniValue& params, bool fHelp);
extern UniValue name_show(const UniValue& params, bool fHelp);
extern UniValue name_history(const UniValue& params, bool fHelp);
extern UniValue name_mempool(const UniValue& params, bool fHelp);
extern UniValue name_debug(const UniValue& params, bool fHelp);
extern UniValue name_dump(const UniValue& params, bool fHelp);

extern UniValue getcheckpoint(const UniValue& params, bool fHelp); // ppcoin
extern UniValue reservebalance(const UniValue& params, bool fHelp);
extern UniValue makekeypair(const UniValue& params, bool fHelp);
extern UniValue sendalert(const UniValue& params, bool fHelp);

// in rest.cpp
extern bool HTTPReq_REST(AcceptedConnection *conn,
                  std::string& strURI,
                  std::map<std::string, std::string>& mapHeaders,
                  bool fRun);

#endif // BITCOIN_RPCSERVER_H
