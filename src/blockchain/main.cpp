#include "wallet.h"
#include <iostream>
#include <vector>
#include "file_stream.h"
#include "arith_uint256.h"
#include "blockchain.h"
#include <time.h>

#pragma comment(lib,"libeay32.lib")

using namespace std;

int main()
{ 
    block_chain bc;

    while (true)
        bc.generate_block();

    
    system("pause");
	return 0;
}

