#pragma once

#include <stdint.h>
#include <map>
#include <string>
#include "uint256.h"
#include "block.h"
#include "block_info.h"
#include "dbproxy.h"

namespace bcus {

class block_chain
{
public:
    static block_chain &instance();

    void init();

    block *prepare_block(); // main thread

    bool generate_block(block *blk); // block thread
    void stop_generate_block();

    bool accept_block(block *blk); // return to main thread

    std::string get_app_path();

    block_info *insert_block_info(const uint256 &block_hash, int height = -1);

    bool read_tran_pos(const uint256 &tranid, block_tran_pos &tran_pos);


    bool add_new_transaction(transaction &tran, bool from_me = false);

private:
    block_chain();
    int64_t get_coin_base_amount(uint32_t height);
    uint32_t get_next_wook_proof(block_info *curent_block);
    block_info *get_curent_block();
    uint32_t get_new_block_id();

private: 
    uint256 start_work_proof_;
    int max_trans_in_block_;
    std::map<uint256, transaction_ptr> trans_; // new trans
    std::map<pre_output, uint256> trans_spends_;
    uint160 coinbase_pub_hash_;
    std::map<uint256, block_info *> block_info_;
    std::map<uint32_t, block_info *> block_by_hight_;
    tran_pos_db *tran_pos_db_;
    bool generating_block_;
};

}