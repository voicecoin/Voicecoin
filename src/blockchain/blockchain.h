#ifndef BCUS_BLOCK_CHAIN_H
#define BCUS_BLOCK_CHAIN_H

#include <stdint.h>
#include <map>
#include <string>
#include "uint256.h"
#include "block.h"
#include "block_info.h"
#include "dbproxy.h"

class block_chain
{
public:
    static block_chain &instance();

    void init_db();

    block *prepare_block(); // main thread

    bool generate_block(block *blk); // block thread

    bool accept_block(block *blk); // return to main thread

    std::string get_app_path();

    block_info *insert_block_info(const uint256 &block_hash, int height = -1);

    bool read_tran_pos(const uint256 &tranid, block_tran_pos &tran_pos);

private:
    block_chain();
    uint32_t get_next_wook_proof(block_info *curent_block);
    block_info *get_curent_block();

private: 
    uint256 start_work_proof_;
    std::map<uint256, transaction_ptr> trans_; // new trans
    uint160 coinbase_pub_hash_;
    std::map<uint256, block_info *> block_info_;
    std::map<uint32_t, block_info *> block_by_hight_;
    block_info_db *block_info_db_;
    tran_pos_db *tran_pos_db_;
};

#endif // BCUS_BLOCK_CHAIN_H
