#ifndef BITCOIN_BLOCK_H
#define BITCOIN_BLOCK_H

#include "slice.h"
#include <map>

namespace bcus {


struct bitcoin_block {
    int nVersion;
    //uint256 hashPrevBlock;
    //uint256 hashMerkleRoot;
    unsigned int nTime;
    unsigned int nBits;
    unsigned int nNonce;
};

}

#endif
