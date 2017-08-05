#include "blockchain.h"
#include "arith_uint256.h"
#include "hash.h"
#include <iostream>
#include <cstring>
#include <time.h>
#include "file_stream.h"
#include <boost/filesystem.hpp>
#ifdef _MSC_VER
#include <windows.h>
#endif

block_chain &block_chain::instance()
{
    static block_chain bc;
    return bc;
}

block_chain::block_chain()
{
    start_work_proof_.set_hex("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
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

void block_chain::init_db()
{
    block_info_db_ = new block_info_db;
    tran_pos_db_ = new tran_pos_db;
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
        block_by_hight_.insert(std::make_pair(height, new_info));
    }
    
    return new_info;
}

block_info *block_chain::get_curent_block()
{
    std::map<uint32_t, block_info *>::reverse_iterator itr = block_by_hight_.rbegin();
    return (itr == block_by_hight_.rend() ? NULL : itr->second);
}

block *block_chain::prepare_block()
{
    block_info *curent_block = get_curent_block();

    block *blk = new block;
    blk->header.version = 1;
    blk->header.timestamp = time(0);
    blk->header.hash_prev_block = (curent_block == NULL ? 0 : *curent_block->hash);
    blk->header.bits = get_next_wook_proof();
    blk->header.nonce = 0;
    blk->header.height = (curent_block == NULL ? 0 : curent_block->height + 1);

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

    return blk;
}

bool block_chain::generate_block(block *blk)
{
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
                return accept_block(blk);
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
                    blk->trans[0]->input[0].pre_trans = 0;
                }
            }
        }
        blk->trans[0]->input[0].pre_trans++;
    }

    return false;
}

bool block_chain::accept_block(block *blk)
{
    // write block_info db
    block_info *new_info = insert_block_info(blk->get_hash(), blk->header.height);
    if (new_info->height != 0)
        new_info->pre_info = insert_block_info(blk->header.hash_prev_block);
    new_info->timestamp = blk->header.timestamp;
    new_info->bits = blk->header.bits;
    block_info_db_->write_block_info(*new_info);

    std::cout << string_helper::time_to_string("%Y-%m-%d %H:%M:%S", time(0)) << std::endl;
    std::cout << "result: " << blk->get_hash().get_hex() << std::endl;

    // write block
    std::string path = get_app_path() + "block";
    boost::filesystem::create_directories(path);
    char filename[64] = { 0 };
    sprintf(filename, "/blk%08d.dat", blk->header.height);
    file_stream fs(path + filename);
    fs << *blk;
    
    // write tran_db
    std::vector<std::pair<uint256, block_tran_pos>> tran_pos_array;
    block_tran_pos pos;
    pos.block_id = blk->header.height;
    pos.tran_pos += get_serialize_size(blk->header);
    pos.tran_pos += get_serialize_size(blk->trans.size());

    for (std::vector<transaction_ptr>::iterator itr = blk->trans.begin();
        itr != blk->trans.end(); ++itr)
    {
        tran_pos_array.push_back(std::make_pair((*itr)->get_hash(), pos));
        pos.tran_pos += get_serialize_size(*(*itr));
    }
    tran_pos_db_->write_tran_pos(tran_pos_array);

    return true;
}

uint32_t block_chain::get_next_wook_proof()
{
    block_info *curent_block = get_curent_block();
    if (curent_block == NULL || curent_block->pre_info == NULL)
        return uint_to_arith256(start_work_proof_).get_compact();
    
    block_info *first_block = curent_block;
    for (int i = 0; first_block->pre_info != NULL && i < 1000; ++i)
        first_block = first_block->pre_info;

    const int TARGET_BLOCK_NUM = 1000;
    //const int TARGET_TIME_SPAN = TARGET_BLOCK_NUM * 10 * 60;
    const int TARGET_TIME_SPAN = TARGET_BLOCK_NUM * 5 * 60;

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

    if (newbit > uint_to_arith256(start_work_proof_))
        newbit = uint_to_arith256(start_work_proof_);

    return newbit.get_compact();
}
