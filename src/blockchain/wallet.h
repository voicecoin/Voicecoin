#pragma once

#include "serialize.h"
#include "uint256.h"
#include "transaction.h"

namespace bcus {

class wallet_key
{
public:
    uint64_t create_time;
    std::vector<unsigned char> pub_key;
    std::vector<unsigned char> priv_key;

public:
    wallet_key();

    bool empty();
    void clear();

    std::string get_address() const;
    static std::string get_address(const std::vector<unsigned char>& pub_key);
    static std::string get_address(const uint160 &pub_hash);

    uint160 get_uint160() const;
    static uint160 get_uint160(const std::vector<unsigned char>& pub_key);
    static uint160 get_uint160(const std::string &addr);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void serialization(Stream& s, Operation ser_action)
    {
        READWRITE(create_time);
        READWRITE(pub_key);
        READWRITE(priv_key);
    }
};

typedef boost::shared_ptr<wallet_key> wallet_key_ptr;

class wallet_db;
class wallet_tran;

class wallet
{
    friend class wallet_db;
public:
    typedef std::map<uint160, wallet_key_ptr>::const_iterator const_iterator;

public:
    static wallet &instance();

    bool init();

    const wallet_key *generate_key();

    const wallet_key *get_key(const uint160 &pub_hash);

    bool set_defult_key(const uint160 &pub_hash);
    const uint160 get_defult_key() const;

    const_iterator cbegin() { return keys.cbegin(); }
    const_iterator cend() { return keys.cend(); }

    bool is_mine(const uint160 &pub_hash);
    void save_mine_transaction(const transaction &tran);
    void disconnect_transaction(const transaction &tran);

    bool is_spent(const uint256 &hash, int index);

    int64_t get_balance();

    bool send_money(const uint160 &pub_hash, int64_t value);

private:
    wallet() {}
    bool is_mine_transaction(const transaction &tran);
    void add_mine_transaction(const transaction &tran);
    void add_mine_transaction(const wallet_tran &tran, bool save_db = false);

private:
    std::map<uint160, wallet_key_ptr> keys;
    std::map<uint256, wallet_tran> trans;
    std::map<pre_output, uint256> trans_spends;
    uint160 default_key_;
    wallet_db *wallet_db_;
};

class wallet_tran : public transaction
{
public:
    int64_t spend_time;
    int64_t left_value;

public:
    wallet_tran();
    wallet_tran(const transaction &r);

    bool empty();
    void clear();

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void serialization(Stream& s, Operation ser_action)
    {
        READWRITE(*(transaction*)this);
        READWRITE(spend_time);
        READWRITE(left_value);
    }

};

}