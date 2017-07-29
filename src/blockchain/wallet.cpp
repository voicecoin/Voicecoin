#include "wallet.h"
#include "ecc_key.h"
#include "base58.h"
#include "hash.h"
#include "util.h"

wallet_key::wallet_key()
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

const wallet_key *wallet::generate_key()
{
    wallet_key *k = new wallet_key;
    ecc_key ecc;
    ecc.generate();
    k->pub_key = ecc.get_pub_key();
    k->priv_key = ecc.get_priv_key();
    keys.insert(make_pair(k->get_uint160(), wallet_key_ptr(k)));
    
    return k;
}

const wallet_key *wallet::get_key(const uint160 &pub_hash)
{
    std::map<uint160, wallet_key_ptr>::iterator itr = keys.find(pub_hash);
    if (itr == keys.end())
        return NULL;
    return itr->second.get();
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