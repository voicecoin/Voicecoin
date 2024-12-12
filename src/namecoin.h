#ifndef NAMECOIN_H
#define NAMECOIN_H

#include "db.h"
#include "base58.h"
#include "hooks.h"
#include "main.h"
#include "rpcprotocol.h"

static const unsigned int NAMEINDEX_CHAIN_SIZE = 1000;
static const int RELEASE_HEIGHT = 1<<16;

class CNameIndex
{
public:
    CDiskTxPos txPos;
    int nHeight;
    int op;
    CNameVal value;

    CNameIndex() : nHeight(0), op(0) {}

    CNameIndex(CDiskTxPos txPos, int nHeight, CNameVal value) :
        txPos(txPos), nHeight(nHeight), value(value) {}

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(txPos);
        READWRITE(nHeight);
        READWRITE(op);
        READWRITE(value);
    }
};

// CNameRecord is all the data that is saved (in nameindex.dat) with associated name
class CNameRecord
{
public:
    std::vector<CNameIndex> vtxPos;
    int nExpiresAt;
    int nLastActiveChainIndex;  // position in vtxPos of first tx in last active chain of name_new -> name_update -> name_update -> ....

    CNameRecord() : nExpiresAt(0), nLastActiveChainIndex(0) {}
    bool deleted()
    {
        if (!vtxPos.empty())
            return vtxPos.back().op == OP_NAME_DELETE;
        else return true;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(vtxPos);
        READWRITE(nExpiresAt);
        READWRITE(nLastActiveChainIndex);
    }
};

class CNameDB : public CDB
{
public:
    CNameDB(const char* pszMode="r+") : CDB("nameindexV2.dat", pszMode) {}

    bool WriteName(const CNameVal& name, const CNameRecord& rec)
    {
        return Write(make_pair(std::string("namei"), name), rec);
    }

    bool ReadName(const CNameVal& name, CNameRecord& rec);

    bool ExistsName(const CNameVal& name)
    {
        return Exists(make_pair(std::string("namei"), name));
    }

    bool EraseName(const CNameVal& name)
    {
        return Erase(make_pair(std::string("namei"), name));
    }

    bool ScanNames(const CNameVal& name, unsigned int nMax,
            std::vector<
                std::pair<
                    CNameVal,
                    std::pair<CNameIndex, int>
                >
            > &nameScan
            );
    bool DumpToTextFile();
};

extern std::map<CNameVal, std::set<uint256> > mapNamePending;

int IndexOfNameOutput(const CTransaction& tx);
bool GetNameCurrentAddress(const CNameVal& name, CBitcoinAddress& address, std::string& error);
std::string stringFromNameVal(const CNameVal& nameVal);
CNameVal nameValFromString(const std::string& str);
std::string stringFromOp(int op);

CAmount GetNameOpFee(const CBlockIndex* pindex, const int nRentalDays, int op, const CNameVal& name, const CNameVal& value);

bool DecodeNameTx(const CTransaction& tx, NameTxInfo& nti, bool checkAddressAndIfIsMine = false);
void GetNameList(const CNameVal& nameUniq, std::map<CNameVal, NameTxInfo> &mapNames, std::map<CNameVal, NameTxInfo> &mapPending);
bool GetNameValue(const CNameVal& name, CNameVal& value);
bool SignNameSignature(const CKeyStore& keystore, const CTransaction& txFrom, CMutableTransaction& txTo, unsigned int nIn, int nHashType=SIGHASH_ALL);

struct NameTxReturn
{
     bool ok;
     std::string err_msg;
     RPCErrorCode err_code;
     std::string address;
     uint256 hex;   // Transaction hash in hex
};
NameTxReturn name_operation(const int op, const CNameVal& name, CNameVal value, const int nRentalDays, const string& strAddress, const string& strValueType);


struct nameTempProxy
{
    unsigned int nTime;
    CNameVal name;
    int op;
    uint256 hash;
    CNameIndex ind;
};

#endif
