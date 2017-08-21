#pragma once

#include "uint256.h"
#include "serialize.h"
#include "block_info.h"

class pre_output
{
public:
    uint256 hash;
    uint32_t index; // index in pre_trans output, from 0

    pre_output() {}
    pre_output(const uint256 &h, uint32_t i) : hash(h), index(i) {}

    friend bool operator<(const pre_output& a, const pre_output& b)
    {
        int cmp = a.hash.compare(b.hash);
        return cmp < 0 || (cmp == 0 && a.index < b.index);
    }

    friend bool operator==(const pre_output& a, const pre_output& b)
    {
        return (a.hash == b.hash && a.index == b.index);
    }

    friend bool operator!=(const pre_output& a, const pre_output& b)
    {
        return !(a == b);
    }
};

class trans_input
{
public:
    pre_output pre_out;
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
        READWRITE(pre_out.hash);
        READWRITE(VARINT(pre_out.index));
        READWRITE(pubkey);
        READWRITE(sig);
    }
};

class trans_output
{
public:
    uint64_t amount;
    uint160 pub_hash;

public:
    trans_output();
    bool empty();
    void clear();

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void serialization(Stream& s, Operation ser_action)
    {
        READWRITE(VARINT(amount));
        READWRITE(pub_hash);
    }
};

class transaction
{
public:
    uint32_t version;
    std::vector<trans_input> input;
    std::vector<trans_output> output;

    // will init by check_sign_and_value()
    uint64_t fee;
    std::vector<block_tran_pos> input_tran_pos;
    mutable uint256 hash;

public:
    transaction();

    bool is_coin_base() const;

    const uint256& get_hash() const;

    bool sign();
    bool check_sign_and_value();

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
