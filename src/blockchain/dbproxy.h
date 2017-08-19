#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include "dbwrapper.h"
#include "block_info.h"
#include "wallet.h"
#include "transaction.h"

class tran_pos_db : public dbwrapper
{
public:
    tran_pos_db();

    bool write_block_info(const block_info &block_info);
    bool load_block_info();

    bool write_tran_pos(
        const std::vector<std::pair<uint256, block_tran_pos>> &tran_pos_array);
    bool read_tran_pos(const uint256 &tranid, block_tran_pos &tran_pos);

};

class wallet_db : public dbwrapper
{
public:
    wallet_db();

    bool write_wallet(const uint160 &pub_hash, const wallet_key_ptr &key);
    bool load_wallet();

    bool write_default_key(const uint160 &pub_hash);
    bool read_default_key(uint160 &pub_hash);

    bool write_transaction(const transaction &tran);
    bool load_transacton();
};
