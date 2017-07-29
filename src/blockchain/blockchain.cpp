#include "blockchain.h"
#include "arith_uint256.h"
#include "hash.h"
#include <iostream>
#include <time.h>

block_chain::block_chain() : curent_block_(NULL)
{
    start_work_proof_.set_hex("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
}

bool block_chain::generate_block()
{
    block *blk = new block;
    blk->header.version = 1;
    blk->header.timestamp = time(0);
    blk->header.hash_prev_block = (curent_block_ == NULL ? 0 : curent_block_->hash);
    blk->header.bits = get_next_wook_proof();
    blk->header.nonce = 0;

    blk->trans.push_back(transaction_ptr(new transaction));
    blk->trans[0]->input.resize(1);
    blk->trans[0]->output.resize(1);
    blk->trans[0]->output[0].pub_hash = coinbase_pub_hash_;
    blk->trans[0]->output[0].value = 10000;

    for (std::map<uint256, transaction_ptr>::iterator itr = trans_.begin();
        itr != trans_.end(); ++itr)
    {
        blk->trans[0]->output[0].value += itr->second->fee;
        blk->trans.push_back(itr->second);
    }

    arith_uint256 target;
    target.set_compact(blk->header.bits);
    std::cout << string_helper::time_to_string("%Y-%m-%d %H:%M:%S", time(0)) << std::endl;
    std::cout << "target: " << target.get_hex() << std::endl;

    while (true)
    {
        blk->header.hash_merkle_root = blk->build_merkle_tree();

        while (true)
        {
            arith_uint256 blk_hash = uint_to_arith256(blk->get_hash());
            if (blk_hash <= target)
            {
                block_info *new_info = new block_info;
                new_info->timestamp = blk->header.timestamp;
                new_info->bits = blk->header.bits;
                new_info->height = (curent_block_ == NULL ? 0 : curent_block_->height + 1);
                new_info->hash = arith_to_uint256(blk_hash);
                new_info->pre_info = curent_block_;
                curent_block_ = new_info;
                
                std::cout << string_helper::time_to_string("%Y-%m-%d %H:%M:%S", time(0)) << std::endl;
                std::cout << "result: " << blk_hash.get_hex() << std::endl;
                
                delete blk;
                return true;
            }

            blk->header.nonce++;
            if (blk->header.nonce & 0x3ffff)
            {
                if (blk->header.nonce == 0xffffffff)
                {
                    break;
                }
                time_t t = time(0);
                if (blk->header.timestamp != t)
                {
                    blk->header.timestamp = t;
                    blk->header.nonce = 0;
                }
            }
        }
        blk->trans[0]->input[0].pre_trans++;
        std::cout << "add extra nonce" << std::endl;
    }

    return false;
}

uint32_t block_chain::get_next_wook_proof()
{
    if (curent_block_ == NULL || curent_block_->pre_info == NULL)
        return uint_to_arith256(start_work_proof_).get_compact();
    
    block_info *first_block = curent_block_;
    for (int i = 0; first_block->pre_info != NULL && i < 1000; ++i)
        first_block = first_block->pre_info;

    const int TARGET_BLOCK_NUM = 1000;
    //const int TARGET_TIME_SPAN = TARGET_BLOCK_NUM * 10 * 60;
    const int TARGET_TIME_SPAN = TARGET_BLOCK_NUM * 5 * 60;

    int block_count = curent_block_->height - first_block->height;
    int total_time = (int)(curent_block_->timestamp - first_block->timestamp);

    total_time = (int)(float(TARGET_BLOCK_NUM * total_time) / (float)block_count);

    if (total_time < TARGET_TIME_SPAN / 4)
        total_time = TARGET_TIME_SPAN / 4;
    if (total_time > TARGET_TIME_SPAN * 4)
        total_time = TARGET_TIME_SPAN * 4;

    arith_uint256 newbit;
    newbit.set_compact(curent_block_->bits);
    newbit *= total_time;
    newbit /= TARGET_TIME_SPAN;

    if (newbit > uint_to_arith256(start_work_proof_))
        newbit = uint_to_arith256(start_work_proof_);

    return newbit.get_compact();
}
