#include "wallet.h"
#include "ecc_key.h"
#include "base58.h"
#include "hash.h"
#include "util.h"
#include "time.h"
#include "dbproxy.h"
#include <iostream>

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
    base58::decode_check(str, vch);
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
    const_iterator itr = keys.insert(make_pair(k->get_uint160(), wallet_key_ptr(k))).first;

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
            std::map<uint256, transaction>::iterator itr = trans.find(tran.input[i].pre_out.hash);
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

void wallet::add_mine_transaction(const transaction &tran, bool save_db)
{
    trans.insert(std::make_pair(tran.get_hash(), tran));
    if (save_db)
        wallet_db_->write_transaction(tran);
    if (!tran.is_coin_base())
    {
        for (size_t i = 0; i < tran.input.size(); ++i)
        {
            trans_spends.insert(std::make_pair(tran.input[i].pre_out, tran.get_hash()));
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
    for (std::map<uint256, transaction>::iterator itr = trans.begin(); itr != trans.end(); ++itr)
    {
        transaction &tran = itr->second;
        for (size_t i = 0; i < tran.output.size(); ++i)
        {
            if (is_mine(tran.output[i].pub_hash) && !is_spent(itr->first, i))
            {
                balance += tran.output[i].amount;
            }
        }
    }
    std::cout << "balance: " << balance << std::endl;
    return balance;
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