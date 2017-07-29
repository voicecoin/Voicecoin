#ifndef BCUS_WALLET_H
#define BCUS_WALLET_H

#include "serialize.h"
#include "uint256.h"

class wallet_key
{
public:
    std::vector<unsigned char> pub_key;
    std::vector<unsigned char> priv_key;

public:
    wallet_key();

    bool empty();
    void clear();

    std::string get_address() const;
    static std::string get_address(const std::vector<unsigned char>& pub_key);

    uint160 get_uint160() const;
    static uint160 get_uint160(const std::vector<unsigned char>& pub_key);
    static uint160 get_uint160(const std::string &addr);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void serialization(Stream& s, Operation ser_action)
    {
        READWRITE(pub_key);
        READWRITE(priv_key);
    }
};

typedef std::shared_ptr<wallet_key> wallet_key_ptr;

class wallet
{
public:
    typedef std::map<uint160, wallet_key_ptr>::const_iterator const_iterator;
    std::map<uint160, wallet_key_ptr> keys;

public:
    static wallet &instance();
    const wallet_key *generate_key();

    const wallet_key *get_key(const uint160 &pub_hash);
    const_iterator cbegin() { return keys.cbegin(); }
    const_iterator cend() { return keys.cend(); }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void serialization(Stream& s, Operation ser_action)
    {
        READWRITE(keys);
    }
};

#endif // BCUS_WALLET_H
