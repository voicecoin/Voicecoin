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
#include "loghelper.h"

using namespace std;
using namespace bcus;

void parse_cmdln(const char *cmdln, vector<string> &vcmd)
{
    const char *p = cmdln;
    bool stat_blank = true;
    const char *str;
    while (*p != 0)
    {
        if (stat_blank)
        {
            if (isblank(*p))
            {
                ++p;
            }
            else
            {
                stat_blank = false;
                str = p;
                ++p;
            }
        }
        else
        {
            if (isblank(*p))
            {
                stat_blank = true;
                string s(str, p - str);
                vcmd.push_back(s);
            }
            else
            {
                ++p;
            }
        }
    }

    if (!stat_blank)
    {
        string s(str, p - str);
        vcmd.push_back(s);
    }

//     cout << "cmd: ";
//     for (auto i = vcmd.begin(); i != vcmd.end(); ++i)
//     {
//         cout << *i << ", ";
//     }
//     cout << endl;
}

int main()
{
    SET_LOG_FILE("conin.log");
    SET_LOG_LEVEL("debug");

//     block b;
//     file_stream fs(block::get_block_file_name(1), "rb+");
//     fs >> b;

    XLOG(XLOG_INFO, "system start...\n");

    wallet::instance().init();
    block_chain::instance().init();

    main_thread::instance().start();

    main_thread::instance().start_mining();

    cc_server_thread::get_instance()->start_server("0.0.0.0", 43333);
    cc_server_thread::get_instance()->start();

    char cmdln[1024] = { 0 };
    while (true)
    {
        cout << ">";
        cin.getline(cmdln, 1023);

        vector<string> vcmd;
        parse_cmdln(cmdln, vcmd);
        if (vcmd.size() == 0) continue;

        if (vcmd[0] == "getbalance")
        {
            main_thread::instance().get_balance();
        }
        else if (vcmd[0] == "showwallet")
        {
            main_thread::instance().show_wallet();
        }
        else if (vcmd[0] == "generatekey")
        {
            main_thread::instance().generate_key();
        }
        else if (vcmd[0] == "sendmoney")
        {
            if (vcmd.size() != 3)
            {
                cout << "sendmoney addr amount" << endl;
                continue;
            }

            //main_thread::instance().send_money(vcmd[1], atoll(vcmd[2].c_str()));
            main_thread::instance().send_money("$JML2AMeN4vfBLVNchmsZj8nuaebB1ZgDj", atoll(vcmd[2].c_str()));
        }

        boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(1));
    }
 
    system("pause");
	return 0;
}
