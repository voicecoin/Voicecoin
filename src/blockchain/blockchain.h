#ifndef BCUS_BLOCK_CHAIN_H
#define BCUS_BLOCK_CHAIN_H

#include <stdint.h>
#include "uint256.h"
#include "block.h"

class block_info
{
public:
    uint64_t timestamp;
    uint32_t bits;
    uint32_t height;
    block_info *pre_info;
    uint256 hash;

    block_info() : timestamp(0), bits(0), height(0), pre_info(0) {}
};

class block_chain
{
public:
    block_chain();
    bool generate_block();

private:
    uint32_t get_next_wook_proof();

private: 
    block_info *curent_block_;
    uint256 start_work_proof_;
    std::map<uint256, transaction_ptr> trans_;
    uint160 coinbase_pub_hash_;
};

#endif // BCUS_BLOCK_CHAIN_H
