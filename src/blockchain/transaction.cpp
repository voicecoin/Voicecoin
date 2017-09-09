#include "transaction.h"
#include "buff_stream.h"
#include "wallet.h"
#include "ecc_key.h"
#include "blockchain.h"
#include "file_stream.h"

namespace bcus {

trans_input::trans_input()
{
    clear();
}

bool trans_input::empty()
{
    return pre_out.index == 0;
}

void trans_input::clear()
{
    pre_out.hash.clear();
    pre_out.index = 0;
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
    return amount == 0;
}

void trans_output::clear()
{
    amount = 0;
    pub_hash.clear();
}

//////////////////////////////////////////////////////////////////////////

transaction::transaction()
{
    clear();
}

bool transaction::is_coin_base() const
{
    return (input.size() == 1 && input[0].pre_out.hash.empty());
}

const uint256& transaction::get_hash() const
{
    if (hash.empty())
        hash = serialize_hash(*this);
    return hash;
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

        block_tran_pos tran_pos;
        if (!block_chain::instance().read_tran_pos(input[i].pre_out.hash, tran_pos))
        {
            return false;
        }

        transaction pre_tran;
        file_stream fs(block::get_block_file_name(tran_pos.block_id));
        fs.seek(tran_pos.tran_pos);
        fs >> pre_tran;

        if (input[i].pre_out.index >= pre_tran.output.size())
        {
            return false;
        }

        uint160 pub_hash = pre_tran.output[input[i].pre_out.index].pub_hash;
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

    input_tran_pos.resize(input.size());
    for (size_t i = 0; i < input.size(); ++i)
    {
        transaction tmp = *this;

        for (size_t j = 0; j < tmp.input.size(); ++j)
        {
            if (i != j)
                tmp.input[j].pubkey.clear();
            tmp.input[j].sig.clear();
        }

        if (!block_chain::instance().read_tran_pos(input[i].pre_out.hash, input_tran_pos[i]))
        {
            return false;
        }

        if (input[i].pre_out.index >= input_tran_pos[i].spents.size() ||
            input_tran_pos[i].spents[input[i].pre_out.index] != block_tran_pos(0, 0))
        {
            return false;
        }

        transaction pre_tran;
        file_stream fs(block::get_block_file_name(input_tran_pos[i].block_id));
        fs.seek(input_tran_pos[i].tran_pos);
        fs >> pre_tran;

        if (input[i].pre_out.index >= pre_tran.output.size())
        {
            return false;
        }

        uint160 pub_hash = pre_tran.output[input[i].pre_out.index].pub_hash;
        in_value += pre_tran.output[input[i].pre_out.index].amount;

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
        out_value += output[i].amount;
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
    input_tran_pos.clear();
}

}
