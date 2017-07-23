#ifndef BCUS_TRANSACTION_H
#define BCUS_TRANSACTION_H

#include "uint256.h"
#include "serialize.h"

class trans_input
{
public:
    uint256 pre_trans;
    uint64_t n;
    std::vector<unsigned char> pubkey;
    std::vector<unsigned char> sig;

public:
    trans_input();
    bool empty();
    void clear();

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void serialization(Stream& s, Operation ser_action)
    {
        READWRITE(pre_trans);
        READWRITE(VARINT(n));
        READWRITE(pubkey);
        READWRITE(sig);
    }
};

class trans_output
{
public:
    uint64_t n;
    uint160 pubkey;

public:
    trans_output();
    bool empty();
    void clear();

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void serialization(Stream& s, Operation ser_action)
    {
        READWRITE(VARINT(n));
        READWRITE(pubkey);
    }
};

class transaction
{
public:
    uint32_t version;
    std::vector<trans_input> input;
    std::vector<trans_output> output;

public:
    transaction();
    bool empty();
    void clear();

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void serialization(Stream& s, Operation ser_action)
    {
        READWRITE(VARINT(version));
        READWRITE(input);
        READWRITE(output);
    }
};

typedef std::shared_ptr<transaction> transaction_ptr;

#endif // BCUS_TRANSACTION_H
