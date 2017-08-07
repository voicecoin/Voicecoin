#include "dbproxy.h"
#include "blockchain.h"
#include "wallet.h"

class block_info_disk
{
public:
    uint64_t timestamp;
    uint32_t bits;
    uint32_t height;
    uint256 pre_block;

public:
    block_info_disk() : timestamp(0), bits(0), height(0) {}

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void serialization(Stream& s, Operation ser_action)
    {
        READWRITE(timestamp);
        READWRITE(bits);
        READWRITE(height);
        READWRITE(pre_block);
    }
};

block_info_db::block_info_db()
    : dbwrapper((block_chain::instance().get_app_path() + "blockdb").c_str())
{
}

bool block_info_db::write_block_info(const block_info &block_info)
{
    block_info_disk bidb;
    bidb.timestamp = block_info.timestamp;
    bidb.bits = block_info.bits;
    bidb.height = block_info.height;
    bidb.pre_block = (block_info.pre_info != 0 ? *block_info.pre_info->hash : 0);

    return write(*block_info.hash, bidb);
}

bool block_info_db::load_block_info()
{
    std::unique_ptr<db_iterator> pcursor(new_iterator());
    for (pcursor->seek_to_first(); pcursor->valid(); pcursor->next())
    {
        uint256 hash;
        block_info_disk info;
        if (pcursor->get_key(hash) && pcursor->get_value(info))
        {
            block_info *new_block = block_chain::instance().insert_block_info(hash, info.height);
            if (new_block->height != 0)
                new_block->pre_info = block_chain::instance().insert_block_info(hash);
            new_block->timestamp = info.timestamp;
            new_block->bits = info.bits;
            new_block->height = info.height;
        }
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////

tran_pos_db::tran_pos_db()
    : dbwrapper((block_chain::instance().get_app_path() + "transdb").c_str())
{
}

bool tran_pos_db::write_tran_pos(
    const std::vector<std::pair<uint256, block_tran_pos>> &tran_pos_array)
{
    db_batch batch(*this);
    for (std::vector<std::pair<uint256, block_tran_pos>>::const_iterator
        itr = tran_pos_array.begin(); itr != tran_pos_array.end(); ++itr)
    {
        batch.write(itr->first, itr->second);
    }
    return write_batch(batch, true);
}

bool tran_pos_db::read_tran_pos(const uint256 &tranid, block_tran_pos &tran_pos)
{
    return read(tranid, tran_pos);
}

//////////////////////////////////////////////////////////////////////////

static const char WALLET_DB_KEY = 'k';
static const char WALLET_DB_TRAN = 't';

wallet_db::wallet_db()
    : dbwrapper((block_chain::instance().get_app_path() + "walletdb").c_str())
{
}

bool wallet_db::write_wallet(const uint160 &pub_hash, const wallet_key_ptr &key)
{
    return write(std::make_pair(WALLET_DB_KEY, pub_hash), key);
}

bool wallet_db::load_wallet()
{
    std::unique_ptr<db_iterator> pcursor(new_iterator());
    for (pcursor->seek(std::make_pair(WALLET_DB_KEY, uint160()));
        pcursor->valid(); pcursor->next())
    {
        std::pair<char, uint160> key;
        wallet_key *wkey = new wallet_key;
        if (pcursor->get_key(key) && pcursor->get_value(*wkey) &&
            key.first == WALLET_DB_KEY)
        {
            wallet::instance().keys.insert(std::make_pair(key.second, wallet_key_ptr(wkey)));
        }
    }

    return true;
}

bool wallet_db::write_default_key(const uint160 &pub_hash)
{
    return write(std::string("default_key"), pub_hash);
}

bool wallet_db::read_default_key(uint160 &pub_hash)
{
    return read(std::string("default_key"), pub_hash);
}

bool wallet_db::write_transaction(const transaction &tran)
{
    return write(std::make_pair(WALLET_DB_TRAN, tran.get_hash()), tran);
}

bool wallet_db::load_transacton()
{
    std::unique_ptr<db_iterator> pcursor(new_iterator());

    for (pcursor->seek(std::make_pair(WALLET_DB_TRAN, uint256()));
        pcursor->valid(); pcursor->next())
    {
        std::pair<char, uint256> key;
        transaction tran;
        if (pcursor->get_key(key) && pcursor->get_value(tran) &&
            key.first == WALLET_DB_TRAN)
        {
            wallet::instance().add_mine_transaction(tran);
        }
    }

    return true;
}