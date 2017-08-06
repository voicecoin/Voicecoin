#ifndef BCUS_BLOCK_H
#define BCUS_BLOCK_H

#include <stdint.h>
#include <uint256.h>
#include "transaction.h"

class block_header
{
public:
    uint32_t version;
    uint64_t timestamp;
    uint256 hash_prev_block;
    uint256 hash_merkle_root;
    uint32_t bits;
    uint32_t nonce;

    uint32_t height; // only new block

public:
    block_header();

    bool empty();
    void clear();

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void serialization(Stream& s, Operation ser_action)
    {
        READWRITE(version);
        READWRITE(timestamp);
        READWRITE(hash_prev_block);
        READWRITE(hash_merkle_root);
        READWRITE(bits);
        READWRITE(nonce);
    }
};

class block
{
public:
    block_header header;
    std::vector<transaction_ptr> trans;

    std::vector<uint256> merkle_tree;

public:
    block();

    uint256 get_hash(); // only header

    uint256 build_merkle_tree();

    bool empty();
    void clear();

    static std::string get_block_file_name(int height);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void serialization(Stream& s, Operation ser_action)
    {
        READWRITE(header);
        READWRITE(trans);
    }
};

#endif // BCUS_BLOCK_H
