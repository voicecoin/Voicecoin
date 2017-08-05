#include "transaction.h"
#include "buff_stream.h"
#include "wallet.h"
#include "ecc_key.h"

trans_input::trans_input()
{
    clear();
}

bool trans_input::empty()
{
    return n == 0;
}

void trans_input::clear()
{
    pre_trans.clear();
    n = 0;
    pubkey.clear();
    sig.clear();
}

//////////////////////////////////////////////////////////////////////////

trans_output::trans_output()
{
    clear();
}

bool trans_output::empty()
{
    return value == 0;
}

void trans_output::clear()
{
    value = 0;
    pub_hash.clear();
}

//////////////////////////////////////////////////////////////////////////

transaction::transaction()
{
    clear();
}

uint256 transaction::get_hash()
{
    return serialize_hash(*this);
}

bool transaction::sign()
{
    for (size_t i = 0; i < input.size(); ++i)
    {
        transaction tmp = *this;
        for (size_t j = 0; j < tmp.input.size(); ++j)
        {
            tmp.input[j].pubkey.clear();
            tmp.input[j].sig.clear();
        }

        uint160 pub_hash; // TODO: find pubkey_hash by trans from wallet
        const wallet_key *key = wallet::instance().get_key(pub_hash);
        if (key == NULL)
        {
            return false;
        }
        input[i].pubkey = tmp.input[i].pubkey = key->pub_key;
        uint256 trans_hash = serialize_hash(tmp);

        if (!ecc_key::sign(key->priv_key, trans_hash, tmp.input[i].sig))
        {
            return false;
        }
        input[i].sig = tmp.input[i].sig;
    }
    return true;
}

bool transaction::check_sign_and_value()
{
    uint64_t in_value = 0;
    uint64_t out_value = 0;

    for (size_t i = 0; i < input.size(); ++i)
    {
        transaction tmp = *this;
        for (size_t j = 0; j < tmp.input.size(); ++j)
        {
            if (i != j)
                tmp.input[j].pubkey.clear();
            tmp.input[j].sig.clear();
        }

        uint160 pub_hash; // TODO: find pubkey_hash from trans list
        // in_value += pre_out;

        uint160 pub_hash2 = wallet_key::get_uint160(input[i].pubkey);
        if (pub_hash != pub_hash2)
        {
            return false;
        }

        uint256 trans_hash = serialize_hash(tmp);

        if (!ecc_key::verify(input[i].pubkey, trans_hash, input[i].sig))
        {
            return false;
        }
    }

    for (size_t i = 0; i < output.size(); ++i)
    {
        out_value += output[i].value;

    }

    if (in_value < out_value)
    {
        return false;
    }

    fee = in_value - out_value;

    return true;
}

bool transaction::empty()
{
    return output.empty();
}

void transaction::clear()
{
    version = 0;
    input.clear();
    output.clear();
    fee = 0;
}
