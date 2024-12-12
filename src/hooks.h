// Copyright (c) 2010-2011 Vincent Durham
// Distributed under the GPL3 software license, see the accompanying
// file license.txt or http://www.gnu.org/licenses/gpl.html.

#ifndef BITCOIN_HOOKS_H
#define BITCOIN_HOOKS_H

class CScript;
class CTransaction;
class CBlockIndex;
class CTxOut;
struct nameTempProxy;

#include <map>
#include <vector>
#include <string>
using namespace std;

class CHooks
{
public:
    virtual bool IsNameFeeEnough(const CTransaction& tx, const CAmount& txFee) = 0;
    virtual bool CheckInputs(const CTransaction& tx, const CBlockIndex* pindexBlock, std::vector<nameTempProxy> &vName, const CDiskTxPos& pos, const CAmount& txFee) = 0;
    virtual bool DisconnectInputs(const CTransaction& tx) = 0;
    virtual bool ConnectBlock(CBlockIndex* pindex, const std::vector<nameTempProxy> &vName) = 0;
    virtual bool ExtractAddress(const CScript& script, std::string& address) = 0;
    virtual void AddToPendingNames(const CTransaction& tx) = 0;
    virtual bool RemoveNameScriptPrefix(const CScript& scriptIn, CScript& scriptOut) = 0;
    virtual bool IsNameScript(CScript scr) = 0;
    virtual bool getNameValue(const string& sName, string& sValue) = 0;
    virtual bool DumpToTextFile() = 0;
};

extern CHooks* InitHook();
extern std::string GetDefaultDataDirSuffix();
extern CHooks* hooks;

#endif
