#include "wallet.h"
#include <iostream>
#include <vector>
#include "file_stream.h"
#include "arith_uint256.h"
#include "blockchain.h"
#include <time.h>

using namespace std;

int main()
{
    block_chain::instance().init_db();
    while (true)
    {
        block *blk = block_chain::instance().prepare_block();
        block_chain::instance().generate_block(blk);
    }
    
    system("pause");
	return 0;
}

