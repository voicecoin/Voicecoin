#include <iostream>
#include <vector>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include "wallet.h"
#include "file_stream.h"
#include "arith_uint256.h"
#include "blockchain.h"
#include "wallet.h"
#include "main_thread.h"
#include "cc_server_thread.h"
#include "cc_client_thread.h"

using namespace std;

int main()
{
    wallet::instance().init();
    block_chain::instance().init();

    //main_thread::instance().start();

    //main_thread::instance().get_balance();
    //main_thread::instance().show_wallet();
    //main_thread::instance().start_mining();

     while (true)
     {
         boost::this_thread::sleep(boost::posix_time::seconds(1));
     }
 
     //cc_server_thread::get_instance()->start_server("0.0.0.0", 43333);
     //cc_server_thread::get_instance()->start();

    system("pause");
	return 0;
}
