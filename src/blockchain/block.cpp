#include "block.h"

#include "block.h"

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
    nonce.setulong(0);
}

//////////////////////////////////////////////////////////////////////////

block::block()
{
    clear();
}

bool block::empty()
{
    return block_header::empty();
}

void block::clear()
{
    block_header::clear();
    trans.clear();
}
