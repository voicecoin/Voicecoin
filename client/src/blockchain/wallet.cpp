#define _CRT_SECURE_NO_WARNINGS
#include "wallet.h"
#include "ecc_key.h"
#include "base58.h"
#include "hash.h"
#include "util.h"
#include "time.h"
#include "dbproxy.h"
#include <iostream>
#include <set>
#include "blockchain.h"
#include "loghelper.h"

namespace bcus {

wallet_key::wallet_key()
    : create_time(0)
{
}

bool wallet_key::empty()
{
    return pub_key.empty();
}

void wallet_key::clear()
{
    pub_key.clear();
    priv_key.clear();
}

std::string wallet_key::get_address() const
{
    return get_address(pub_key);
}

std::string wallet_key::get_address(const std::vector<unsigned char>& pub_key)
{
    uint160 hash160 = hash_helper::hash160(pub_key);
    std::vector<unsigned char> vch;
    vch.insert(vch.end(), UBEGIN(hash160), UEND(hash160));
    return "$" + base58::encode_check(vch);
}

std::string wallet_key::get_address(const uint160 &pub_hash)
{
    std::vector<unsigned char> vch;
    vch.insert(vch.end(), UBEGIN(pub_hash), UEND(pub_hash));
    return "$" + base58::encode_check(vch);
}

uint160 wallet_key::get_uint160() const
{
    return get_uint160(pub_key);
}

uint160 wallet_key::get_uint160(const std::vector<unsigned char>& pub_key)
{
    return hash_helper::hash160(pub_key);
}

uint160 wallet_key::get_uint160(const std::string &addr)
{
    std::string str = addr.substr(1);
    std::vector<unsigned char> vch;
    if (!base58::decode_check(str, vch))
        return uint160();
    return uint160(vch);
}

//////////////////////////////////////////////////////////////////////////
//

wallet &wallet::instance()
{
    static wallet w;
    return w;
}

bool wallet::init()
{
    wallet_db_ = new wallet_db;
    if (!wallet_db_->load_wallet())
    {
        return false;
    }

    wallet_db_->load_transacton();

    wallet_db_->read_default_key(default_key_);

    if (keys.size() == 0)
    {
        const wallet_key *k = generate_key();
        set_defult_key(k->get_uint160());
        wallet_db_->write_default_key(k->get_uint160());
    }

    return true;
}

const wallet_key *wallet::generate_key()
{
    wallet_key *k = new wallet_key;
    ecc_key ecc;
    ecc.generate();
    k->create_time = time(0);
    k->pub_key = ecc.get_pub_key();
    k->priv_key = ecc.get_priv_key();
    const_iterator itr = keys.insert(std::make_pair(k->get_uint160(), wallet_key_ptr(k))).first;

    wallet_db_->write_wallet(k->get_uint160(), itr->second);

    return k;
}

const wallet_key *wallet::get_key(const uint160 &pub_hash)
{
    std::map<uint160, wallet_key_ptr>::iterator itr = keys.find(pub_hash);
    if (itr == keys.end())
        return NULL;
    return itr->second.get();
}

bool wallet::set_defult_key(const uint160 &pub_hash)
{
    if (keys.find(pub_hash) == keys.end())
    {
        return false;
    }

    default_key_ = pub_hash;
    wallet_db_->write_default_key(default_key_);

    return true;
}

const uint160 wallet::get_defult_key() const
{
    return default_key_;
}

bool wallet::is_mine(const uint160 &pub_hash)
{
    return keys.find(pub_hash) != keys.end();
}

void wallet::save_mine_transaction(const transaction &tran)
{
    if (!is_mine_transaction(tran))
    {
        return;
    }

    add_mine_transaction(tran);
}

bool wallet::is_mine_transaction(const transaction &tran)
{
    if (!tran.is_coin_base())
    {
        for (size_t i = 0; i < tran.input.size(); ++i)
        {
            std::map<uint256, wallet_tran>::iterator itr = trans.find(tran.input[i].pre_out.hash);
            if (itr != trans.end() && itr->second.output.size() > tran.input[i].pre_out.index)
            {
                if (is_mine(itr->second.output[tran.input[i].pre_out.index].pub_hash))
                {
                    return true;
                }
            }
        }
    }

    for (size_t i = 0; i < tran.output.size(); ++i)
    {
        if (is_mine(tran.output[i].pub_hash))
        {
            return true;
        }
    }
    return false;
}

void wallet::add_mine_transaction(const transaction &tran)
{
    XLOG(XLOG_INFO, "add_mine_transaction tran[%d %s]\n", tran.unix_time, tran.get_hash().to_string().c_str());
    trans.insert(std::make_pair(tran.get_hash(), wallet_tran(tran)));
    wallet_db_->write_transaction(tran);
    if (!tran.is_coin_base())
    {
        for (size_t i = 0; i < tran.input.size(); ++i)
        {
            trans_spends.insert(std::make_pair(tran.input[i].pre_out, tran.get_hash()));
        }
    }
}

void wallet::add_mine_transaction(const wallet_tran &tran, bool save_db)
{
    trans.insert(std::make_pair(tran.get_hash(), wallet_tran(tran)));
    if (save_db)
    {
        wallet_db_->write_transaction(tran);
    }
    if (!tran.is_coin_base())
    {
        for (size_t i = 0; i < tran.input.size(); ++i)
        {
            trans_spends.insert(std::make_pair(tran.input[i].pre_out, tran.get_hash()));
        }
    }
}

void wallet::disconnect_transaction(const transaction &tran)
{
    trans.erase(tran.get_hash());
    wallet_db_->erase_transaction(tran.get_hash());
    if (!tran.is_coin_base())
    {
        for (size_t i = 0; i < tran.input.size(); ++i)
        {
            trans_spends.erase(tran.input[i].pre_out);
        }
    }
}

bool wallet::is_spent(const uint256 &hash, int index)
{
    pre_output pout;
    pout.hash = hash;
    pout.index = index;

    std::multimap<pre_output, uint256>::iterator itr = trans_spends.find(pout);
    if (itr != trans_spends.end())
    {
        return true;
    }
    return false;
}

int64_t wallet::get_balance()
{
    int64_t balance = 0;
    for (std::map<uint256, wallet_tran>::iterator itr = trans.begin(); itr != trans.end(); ++itr)
    {
        wallet_tran &tran = itr->second;
        if (tran.spend_time != 0)
        {
            bool add_left = true;
            for (size_t i = 0; i < tran.output.size(); ++i)
            {
                if (is_spent(itr->first, i))
                {
                    add_left = false;
                    break;
                }
            }
            if (add_left)
                balance += tran.left_value;
        }
        else
        {
            for (size_t i = 0; i < tran.output.size(); ++i)
            {
                if (is_mine(tran.output[i].pub_hash) && !is_spent(itr->first, i))
                {
                    balance += tran.output[i].amount;
                }
            }
        }

    }
    std::cout << "balance: " << balance << std::endl;
    return balance;
}

bool wallet::send_money(const uint160 &pub_hash, int64_t value)
{
    transaction new_tran;
    int64_t amount = 0;
    std::set<wallet_tran *> spend_tran;
    for (std::map<uint256, wallet_tran>::iterator itr = trans.begin(); itr != trans.end(); ++itr)
    {
        wallet_tran &tran = itr->second;
        if (tran.spend_time != 0)
        {
            // TODO: check timeout
            continue;
        }

        // ͬһtrans�е������Լ��ı�һ�𻨵�
        for (size_t i = 0; i < tran.output.size(); ++i)
        {
            if (is_mine(tran.output[i].pub_hash) && !is_spent(itr->first, i))
            {
                trans_input input;
                input.pre_out.hash = itr->first;
                input.pre_out.index = i;
                new_tran.input.push_back(input);
                amount += tran.output[i].amount;

                spend_tran.insert(&tran);
            }
        }
        if (amount >= value)
        {
            break;
        }
    }

    if (amount < value)
    {
        XLOG(XLOG_WARNING, "wallet::%s, balance no enough[%lld]\n", __FUNCTION__, amount);
        return false;
    }

    trans_output output;
    output.amount = value;
    output.pub_hash = pub_hash;
    new_tran.output.push_back(output);

    if (amount - value > 0)
    {
        trans_output output2;
        output2.amount = amount - value;
        output2.pub_hash = default_key_;
        new_tran.output.push_back(output2);
    }

    if (!new_tran.sign())
    {
        XLOG(XLOG_WARNING, "wallet::%s, sign failed\n", __FUNCTION__);
        return false;
    }

    if (!block_chain::instance().add_new_transaction(new_tran, true))
    {
        XLOG(XLOG_WARNING, "wallet::%s add transaction failed\n", __FUNCTION__);
        return false;
    }

    for (std::set<wallet_tran *>::iterator itr = spend_tran.begin();
        itr != spend_tran.end(); ++itr)
    {
        (*itr)->spend_time = time(0);

        if (amount - value > 0)
        {
            auto itr2 = itr;
            ++itr2;
            if (itr2 == spend_tran.end())
            {
                (*itr)->left_value = amount - value;
            }
        }
        
        wallet_db_->write_transaction(*(*itr));
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////

wallet_tran::wallet_tran()
{
    clear();
}

wallet_tran::wallet_tran(const transaction &r)
    : transaction(r)
{
    spend_time = 0;
    left_value = 0;
}

bool wallet_tran::empty()
{
    return transaction::empty();
}

void wallet_tran::clear()
{
    spend_time = 0;
    left_value = 0;
    transaction::clear();
}

}

/*

static int test()
{
    {
        wallet w;
        const wallet_key *k = w.generate_key();
        cout << k->get_address() << endl;
        w.generate_key();

        file_stream fs("wallet", "wb");
        if (!fs)
        {
            cout << "open wallet failed" << endl;
            system("pause");
        }
        fs << w;
    }
    cout << "=======read==========================" << endl;
    {
        wallet w;
        file_stream fs("wallet", "rb");
        if (!fs)
        {
            cout << "open wallet failed2" << endl;
            system("pause");
        }
        fs >> w;
        for (wallet::const_iterator itr = w.cbegin(); itr != w.cend(); ++itr)
        {
            cout << itr->second->get_address() << endl;
        }
    }

    system("pause");
    return 0;
}
*/