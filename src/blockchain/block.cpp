#define _CRT_SECURE_NO_WARNINGS
#include "block.h"
#include "buff_stream.h"
#include "util.h"
#include <boost/filesystem.hpp>
#include "blockchain.h"

namespace bcus {

block_header::block_header()
{
    clear();
}

bool block_header::empty()
{
    return bits == 0;
}

void block_header::clear()
{
    version = 0;
    timestamp = 0;
    hash_prev_block.clear();
    hash_merkle_root.clear();
    bits = 0;
    nonce = 0;
}

//////////////////////////////////////////////////////////////////////////

block::block()
{
    clear();
}

uint256 block::get_hash()
{
    return serialize_hash(header);
}

uint256 block::build_merkle_tree()
{
    merkle_tree.clear();
    for (std::vector<transaction_ptr>::iterator itr = trans.begin();
        itr != trans.end(); ++itr)
        merkle_tree.push_back((*itr)->get_hash());
    int j = 0;
    for (int size = trans.size(); size > 1; size = (size + 1) / 2)
    {
        for (int i = 0; i < size; i += 2)
        {
            int i2 = BCUS_MIN(i + 1, size - 1);
            merkle_tree.push_back(hash_helper::hash(
                BEGIN(merkle_tree[j + i]), END(merkle_tree[j + i]),
                BEGIN(merkle_tree[j + i2]), END(merkle_tree[j + i2])));
        }
        j += size;
    }
    return (merkle_tree.empty() ? 0 : merkle_tree.back());
}

bool block::empty()
{
    return header.empty();
}

void block::clear()
{
    header.clear();
    trans.clear();
}

std::string block::get_block_file_name(int block_id)
{
    std::string path = block_chain::instance().get_app_path() + "block";
    boost::filesystem::create_directories(path);
    char filename[64] = { 0 };
    sprintf(filename, "/blk%08d.dat", block_id);
    return path + filename;
}

}