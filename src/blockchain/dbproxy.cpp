#define _CRT_SECURE_NO_WARNINGS
#include "dbproxy.h"
#include "blockchain.h"
#include "wallet.h"

namespace bcus {

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

//////////////////////////////////////////////////////////////////////////

static const char TRAN_DB_BLOCK = 'b';
static const char TRAN_DB_TRAN = 't';

tran_pos_db::tran_pos_db()
    : dbwrapper((block_chain::instance().get_app_path() + "transdb").c_str())
{
}

bool tran_pos_db::write_block_info(const block_info &block_info)
{
    block_info_disk bidb;
    bidb.timestamp = block_info.timestamp;
    bidb.bits = block_info.bits;
    bidb.height = block_info.height;
    bidb.pre_block = (block_info.pre_info != 0 ? *block_info.pre_info->hash : 0);

    return write(std::make_pair(TRAN_DB_BLOCK, *block_info.hash), bidb);
}

bool tran_pos_db::load_block_info()
{
    boost::shared_ptr<db_iterator> pcursor(new_iterator());
    for (pcursor->seek(std::make_pair(TRAN_DB_BLOCK, uint256()));
        pcursor->valid(); pcursor->next())
    {
        std::pair<char, uint256> key;
        block_info_disk info;
        if (pcursor->get_key(key) && pcursor->get_value(info))
        {
            if (key.first != TRAN_DB_BLOCK) continue;
            block_info *new_block = block_chain::instance().insert_block_info(key.second, info.height);
            if (new_block->height != 0)
                new_block->pre_info = block_chain::instance().insert_block_info(key.second);
            new_block->timestamp = info.timestamp;
            new_block->bits = info.bits;
            new_block->height = info.height;
        }
    }
    return true;
}

bool tran_pos_db::write_tran_pos(
    const std::vector<std::pair<uint256, block_tran_pos>> &tran_pos_array)
{
    db_batch batch(*this);
    for (std::vector<std::pair<uint256, block_tran_pos>>::const_iterator
        itr = tran_pos_array.begin(); itr != tran_pos_array.end(); ++itr)
    {
        batch.write(std::make_pair(TRAN_DB_TRAN, itr->first), itr->second);
    }
    return write_batch(batch, true);
}

bool tran_pos_db::read_tran_pos(const uint256 &tranid, block_tran_pos &tran_pos)
{
    return read(std::make_pair(TRAN_DB_TRAN, tranid), tran_pos);
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
    boost::shared_ptr<db_iterator> pcursor(new_iterator());
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

bool wallet_db::write_transaction(const wallet_tran &tran)
{
    return write(std::make_pair(WALLET_DB_TRAN, tran.get_hash()), tran);
}

void wallet_db::erase_transaction(const uint256 &tran_hash)
{
    erase(std::make_pair(WALLET_DB_TRAN, tran_hash));
}

bool wallet_db::load_transacton()
{
    boost::shared_ptr<db_iterator> pcursor(new_iterator());

    for (pcursor->seek(std::make_pair(WALLET_DB_TRAN, uint256()));
        pcursor->valid(); pcursor->next())
    {
        std::pair<char, uint256> key;
        wallet_tran tran;
        if (pcursor->get_key(key) && pcursor->get_value(tran) &&
            key.first == WALLET_DB_TRAN)
        {
            wallet::instance().add_mine_transaction(tran, false);
        }
    }

    return true;
}

}