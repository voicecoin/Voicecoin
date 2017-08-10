#include "wallet.h"
#include <iostream>
#include <vector>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include "file_stream.h"
#include "arith_uint256.h"
#include "blockchain.h"
#include "wallet.h"
#include "main_thread.h"

using namespace std;

int main()
{
    wallet::instance().init();
    block_chain::instance().init();

    main_thread::instance().start();

    main_thread::instance().get_balance();
    main_thread::instance().start_mining();

    while (true)
    {
        boost::this_thread::sleep(boost::posix_time::seconds(1));
    }
    
    system("pause");
	return 0;
}
