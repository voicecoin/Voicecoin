#define _CRT_SECURE_NO_WARNINGS
#include "blockchain.h"
#include "arith_uint256.h"
#include "hash.h"
#include <iostream>
#include <cstring>
#include <vector>
#include <time.h>
#include "file_stream.h"
#include <boost/filesystem.hpp>
#include "main_thread.h"
#include "wallet.h"
#include "loghelper.h"
#ifdef _MSC_VER
#include <windows.h>
#endif

namespace bcus {

block_chain &block_chain::instance()
{
    static block_chain bc;
    return bc;
}

block_chain::block_chain()
{
    start_work_proof_.set_hex("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    max_trans_in_block_ = 2000;
}

std::string block_chain::get_app_path()
{
#ifdef _MSC_VER
    char path[_MAX_PATH] = { 0 };
    ::GetModuleFileName(NULL, path, sizeof(path) - 1);
    *(strrchr(path, '\\') + 1) = 0;
#else
    const int MAX_PATH_MAX = 512;
    char path[MAX_PATH_MAX] = { 0 };
    int rslt = readlink("/proc/self/exe", path, MAX_PATH_MAX);
    if (rslt < 0 || rslt >= MAX_PATH_MAX)
    {
        return   NULL;
    }
    path[rslt] = 0;
    *(strrchr(path, '/') + 1) = 0;
#endif
    return path;
}

void block_chain::init()
{
    tran_pos_db_ = new tran_pos_db;
    coinbase_pub_hash_ = wallet::instance().get_defult_key();
    tran_pos_db_->load_block_info();
    tran_pos_db_->read_best_chain(best_chain_hash_);
}

block_info *block_chain::insert_block_info(const uint256 &block_hash, int height/* = -1*/)
{
    std::map<uint256, block_info *>::iterator itr = block_info_.find(block_hash);
    if (itr != block_info_.end())
    {
        return itr->second;
    }
    block_info *new_info = new block_info;
    itr = block_info_.insert(std::make_pair(block_hash, new_info)).first;
    new_info->hash = (uint256 *)&(itr->first);
    if (height != -1)
    {
        new_info->height = height;
    }

    return new_info;
}

block_info *block_chain::get_curent_block()
{
    std::map<uint256, block_info *>::iterator itr = block_info_.find(best_chain_hash_);
    return (itr == block_info_.end() ? NULL : itr->second);
}

block *block_chain::prepare_block()
{
    block_info *curent_block = get_curent_block();

    block *blk = new block;
    blk->header.version = 1;
    blk->header.timestamp = time(0);
    blk->header.hash_prev_block = (curent_block == NULL ? 0 : *curent_block->hash);
    blk->header.bits = get_next_wook_proof(curent_block);
    blk->header.nonce = 0;

    uint32_t block_height = (curent_block == NULL ? 0 : curent_block->height + 1);

    blk->trans.push_back(transaction_ptr(new transaction));
    blk->trans[0]->input.resize(1);
    blk->trans[0]->output.resize(1);
    blk->trans[0]->output[0].pub_hash = coinbase_pub_hash_;
    blk->trans[0]->output[0].amount = get_coin_base_amount(block_height);

    int count = 0;
//     while (trans_.size() > 0)
//     {
//         std::map<uint256, transaction_ptr>::iterator itr = trans_.begin();
//         blk->trans[0]->output[0].amount += itr->second->fee;
//         blk->trans.push_back(itr->second);
//         trans_.erase(itr);
//         if (++count > max_trans_in_block_)
//             break;
//     }

    for (std::map<uint256, transaction_ptr>::iterator itr = trans_.begin();
        itr != trans_.end(); ++itr)
    {
        blk->trans[0]->output[0].amount += itr->second->fee;
        blk->trans.push_back(itr->second);
        if (++count > max_trans_in_block_)
            break;
    }

    return blk;
}

bool block_chain::generate_block(block *blk)
{
    generating_block_ = true;
    arith_uint256 target;
    target.set_compact(blk->header.bits);
    XLOG(XLOG_INFO, "block_chain::%s, [%s]\n",
        __FUNCTION__, bcus::string_helper::time_to_string("%Y-%m-%d %H:%M:%S", time(0)).c_str());
    XLOG(XLOG_INFO, "block_chain::%s, target[%s]\n",
        __FUNCTION__, target.get_hex().c_str());
    //std::cout << bcus::string_helper::time_to_string("%Y-%m-%d %H:%M:%S", time(0)) << std::endl;
    //std::cout << "target: " << target.get_hex() << std::endl;
    uint256 nonce_ex;

    while (generating_block_)
    {
        blk->header.hash_merkle_root = blk->build_merkle_tree();

        while (generating_block_)
        {
            arith_uint256 blk_hash = uint_to_arith256(blk->get_hash());
            if (blk_hash <= target)
            {
                main_thread::instance().accept_new_block(blk);
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
                    nonce_ex.clear();
                }
            }
        }

        nonce_ex++;
        nonce_ex.to_vch(blk->trans[0]->input[0].pubkey);
    }

    return false;
}

void block_chain::stop_generate_block()
{
    generating_block_ = false;
}

bool block_chain::accept_block(block *blk)
{
    // check block and all transaction
    if (blk->header.hash_merkle_root != blk->build_merkle_tree())
    {
        XLOG(XLOG_WARNING, "block_chain::%s merkle_tree check failed\n", __FUNCTION__);
        return false;
    }
    uint32_t block_height = 1;
    block_info *pre_block = NULL;
    if (block_info_.size() != 0)
    {
        std::map<uint256, block_info *>::iterator itr_pre = block_info_.find(blk->header.hash_prev_block);
        if (itr_pre == block_info_.end())
        {
            XLOG(XLOG_WARNING, "block_chain::%s get pre block_info failed\n", __FUNCTION__);
            return false;
        }

        pre_block = itr_pre->second;
        block_height = pre_block->height + 1;
    }
    if (blk->header.bits != get_next_wook_proof(pre_block))
    {
        XLOG(XLOG_WARNING, "block_chain::%s wook_proof check failed\n", __FUNCTION__);
        return false;
    }

    arith_uint256 target;
    target.set_compact(blk->header.bits);
    arith_uint256 blk_hash = uint_to_arith256(blk->get_hash());
    if (blk_hash > target)
    {
        XLOG(XLOG_WARNING, "block_chain::%s blk_hash check failed\n", __FUNCTION__);
        return false;
    }

    std::map<pre_output, uint256> spends;
    int64_t coin_base_amount = get_coin_base_amount(block_height);
    for (size_t i = 0; i < blk->trans.size(); ++i)
    {
        if (i == 0)
        {
            if (!blk->trans[i]->is_coin_base() || blk->trans[i]->output.size() != 1)
            {
                XLOG(XLOG_WARNING, "block_chain::%s first tran check failed\n", __FUNCTION__);
                return false;
            }
            continue;
        }
        else if (blk->trans[i]->is_coin_base())
        {
            XLOG(XLOG_WARNING, "block_chain::%s tran check failed\n", __FUNCTION__);
            return false;
        }

        for (std::vector<trans_input>::iterator itr = blk->trans[i]->input.begin();
            itr != blk->trans[i]->input.end(); ++itr)
        {
            if (!spends.insert(std::make_pair(itr->pre_out, blk->trans[i]->get_hash())).second)
            {
                XLOG(XLOG_WARNING, "block_chain::%s double tran check failed\n", __FUNCTION__);
                return false;
            }
        }

        if (!blk->trans[i]->check_sign_and_value())
        {
            XLOG(XLOG_WARNING, "block_chain::%s check_sign_and_value failed\n", __FUNCTION__);
            return false;
        }
        coin_base_amount += blk->trans[i]->fee;
    }

    if (blk->trans[0]->output[0].amount != coin_base_amount)
    {
        XLOG(XLOG_WARNING, "block_chain::%s amount check failed\n", __FUNCTION__);
        return false;
    }

    XLOG(XLOG_INFO, "block_chain::%s, [%s]\n",
        __FUNCTION__, bcus::string_helper::time_to_string("%Y-%m-%d %H:%M:%S", time(0)).c_str());
    XLOG(XLOG_INFO, "block_chain::%s, result[%s]\n",
        __FUNCTION__, blk->get_hash().get_hex().c_str());

    // write block
    uint32_t block_id = get_new_block_id();
    if (block_id == 0)
    {
        XLOG(XLOG_WARNING, "block_chain::%s get block id failed\n", __FUNCTION__);
        return false;
    }
    file_stream fs(block::get_block_file_name(block_id));
    fs << *blk;

    // write block_info db
    block_info *new_info = insert_block_info(blk->get_hash(), block_height);
    if (new_info->height != 1)
        new_info->pre_info = insert_block_info(blk->header.hash_prev_block);
    new_info->timestamp = blk->header.timestamp;
    new_info->bits = blk->header.bits;
    new_info->block_id = block_id;
    tran_pos_db_->write_block_info(*new_info);

    // write tran_db
    add_block_to_chain(new_info, blk);

    trans_spends_.clear();
    std::map<uint256, transaction_ptr> trans = trans_;
    trans_.clear();
    for (std::map<uint256, transaction_ptr>::iterator itr = trans.begin();
        itr != trans.end(); ++itr)
    {
        add_new_transaction(*itr->second);
    }

    return true;
}

bool block_chain::add_block_to_chain(block_info *new_info, block *blk)
{
    uint32_t curr_height = 0;
    block_info *curr_block = get_curent_block();
    if (curr_block != NULL)
        curr_height = curr_block->height;

    if (new_info->height > curr_height)
    {
        std::vector<std::pair<uint256, block_tran_pos>> tran_pos_array;
        if (new_info->height == 1 ||
            new_info->pre_info->hash == curr_block->hash)
        {
            if (!connect_block(new_info, blk, tran_pos_array))
            {
                return false;
            }
        }
        else
        {
            block_info* pfork = curr_block;
            block_info* plonger = new_info;
            while (pfork != plonger)
            {
                if ((pfork = pfork->pre_info) == NULL)
                {
                    return false;
                }
                while (plonger->height > pfork->height)
                {
                    if ((plonger = plonger->pre_info) == NULL)
                    {
                        return false;
                    }
                }
            }

            for (block_info* pindex = curr_block; pindex != pfork; pindex = pindex->pre_info)
            {
                block blk;
                file_stream fs(block::get_block_file_name(pindex->block_id), "rb+");
                fs >> blk;
                disconnect_block(&blk, tran_pos_array);
            }

            std::vector<block_info*> vconnect;
            for (block_info* pindex = new_info; pindex != pfork; pindex = pindex->pre_info)
                vconnect.push_back(pindex);

            for (auto itr = vconnect.rbegin(); itr != vconnect.rend(); ++itr)
            {
                block blk;
                file_stream fs(block::get_block_file_name((*itr)->block_id), "rb+");
                fs >> blk;
                if (!connect_block(*itr, &blk, tran_pos_array))
                {
                    return false;
                }
            }
        }

        best_chain_hash_ = *new_info->hash;
        tran_pos_db_->write_best_chain(best_chain_hash_);
        tran_pos_db_->write_tran_pos(tran_pos_array);
    }
    return true;
}

bool block_chain::connect_block(block_info *blkinfo, block *blk,
    std::vector<std::pair<uint256, block_tran_pos>> &tran_pos_array)
{
    block_tran_pos pos;
    pos.block_id = blkinfo->block_id;
    pos.tran_pos += get_serialize_size(blk->header);
    pos.tran_pos += get_size_of_var_int((uint64_t)(blk->trans.size()));

    for (std::vector<transaction_ptr>::iterator itr = blk->trans.begin();
        itr != blk->trans.end(); ++itr)
    {
        if (itr != blk->trans.begin())
        {
            if ((*itr)->input.size() != (*itr)->input_tran_pos.size())
            {
                XLOG(XLOG_WARNING, "block_chain::%s input size check failed\n", __FUNCTION__);
                return false;
            }
            // update input pos info
            for (size_t i = 0; i < (*itr)->input.size(); ++i)
            {
                (*itr)->input_tran_pos[i].spents[(*itr)->input[i].pre_out.index] = pos;
                tran_pos_array.push_back(std::make_pair((*itr)->input[i].pre_out.hash, (*itr)->input_tran_pos[i]));
            }
        }

        // insert this transaction pos info
        pos.spents.resize((*itr)->output.size());
        tran_pos_array.push_back(std::make_pair((*itr)->get_hash(), pos));

        pos.tran_pos += get_serialize_size(*(*itr));

        wallet::instance().save_mine_transaction(*(*itr));

        trans_.erase((*itr)->get_hash());
    }

    return true;
}

bool block_chain::disconnect_block(block *blk,
    std::vector<std::pair<uint256, block_tran_pos>> &tran_pos_array)
{
    for (std::vector<transaction_ptr>::iterator itr = blk->trans.begin();
        itr != blk->trans.end(); ++itr)
    {
        if (!(*itr)->check_sign_and_value()) // force init input_tran_pos
        {
            return false;
        }
        if ((*itr)->input.size() != (*itr)->input_tran_pos.size())
        {
            XLOG(XLOG_WARNING, "block_chain::%s input size check failed\n", __FUNCTION__);
            return false;
        }
        for (size_t i = 0; i < (*itr)->input.size(); ++i)
        {
            (*itr)->input_tran_pos[i].spents[(*itr)->input[i].pre_out.index] = block_tran_pos();
            tran_pos_array.push_back(std::make_pair((*itr)->input[i].pre_out.hash, (*itr)->input_tran_pos[i]));
        }

        // disconnect wallet
        wallet::instance().disconnect_transaction(*(*itr));
    }

    return true;
}

int64_t block_chain::get_coin_base_amount(uint32_t height)
{
    static const uint64_t FIRST_AMOUNT = 100000000000;
    static const uint64_t FROM_AMOUT = 1000000;
    static const uint32_t DEC_BLOCK_COUNT = 200000;

    if (height == 0)
        return FIRST_AMOUNT;

    uint64_t amount = FROM_AMOUT;
    while (height >= DEC_BLOCK_COUNT)
    {
        height -= DEC_BLOCK_COUNT;
        amount = (amount * 2 / 3);
    }
    return amount;
}

uint32_t block_chain::get_next_wook_proof(block_info *curent_block)
{
    const static int TARGET_BLOCK_NUM = 1000;
    //const static int TARGET_TIME_SPAN = TARGET_BLOCK_NUM * 10 * 60;
    const static int TARGET_TIME_SPAN = TARGET_BLOCK_NUM * 5 * 60;

    if (curent_block == NULL || curent_block->pre_info == NULL)
        return uint_to_arith256(start_work_proof_).get_compact();

    block_info *first_block = curent_block;
    for (int i = 0; first_block->pre_info != NULL && i < 1000; ++i)
        first_block = first_block->pre_info;

    int block_count = curent_block->height - first_block->height;
    int total_time = (int)(curent_block->timestamp - first_block->timestamp);

    total_time = (int)(float(TARGET_BLOCK_NUM * total_time) / (float)block_count);

    if (total_time < TARGET_TIME_SPAN / 4)
        total_time = TARGET_TIME_SPAN / 4;
    if (total_time > TARGET_TIME_SPAN * 4)
        total_time = TARGET_TIME_SPAN * 4;

    arith_uint256 newbit;
    newbit.set_compact(curent_block->bits);
    newbit *= total_time;
    newbit /= TARGET_TIME_SPAN;

    //if (newbit > uint_to_arith256(start_work_proof_))
        newbit = uint_to_arith256(start_work_proof_);

    return newbit.get_compact();
}

bool block_chain::read_tran_pos(const uint256 &tranid, block_tran_pos &tran_pos)
{
    return tran_pos_db_->read_tran_pos(tranid, tran_pos);
}

bool block_chain::add_new_transaction(transaction &tran, bool from_me)
{
    if (!tran.check_sign_and_value())
    {
        XLOG(XLOG_WARNING, "wallet::%s check_sign_and_value failed\n", __FUNCTION__);
        return false;
    }

    if (trans_.find(tran.get_hash()) != trans_.end())
    {
        XLOG(XLOG_WARNING, "wallet::%s double spent\n", __FUNCTION__);
        return false;
    }
    for (std::vector<trans_input>::iterator itr = tran.input.begin();
        itr != tran.input.end(); ++itr)
    {
        std::map<pre_output, uint256>::iterator ispend = trans_spends_.find(itr->pre_out);
        if (ispend != trans_spends_.end())
        {
            XLOG(XLOG_WARNING, "wallet::%s double spent\n", __FUNCTION__);
            return false;
        }
    }

    if (from_me)
    {
        // TODO: send to network
    }

    transaction_ptr ptr(new transaction(tran));
    trans_.insert(std::make_pair(tran.get_hash(), ptr));

    for (std::vector<trans_input>::iterator itr = tran.input.begin();
        itr != tran.input.end(); ++itr)
    {
        trans_spends_.insert(std::make_pair(itr->pre_out, tran.get_hash()));
    }

    return true;
}

uint32_t block_chain::get_new_block_id()
{
    static uint32_t block_id = 1;
    while (true)
    {
        FILE* file = fopen(block::get_block_file_name(block_id).c_str(), "ab");
        if (file == NULL)
        {
            return 0;
        }
        fseek(file, 0, SEEK_END);
        if (ftell(file) > 0)
        {
            fclose(file);
            block_id++;
            continue;
        }
        fclose(file);
        return block_id;
    }
}

}
