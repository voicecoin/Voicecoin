#ifndef BCUS_WALLET_H
#define BCUS_WALLET_H

#include "serialize.h"

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
    typedef std::map<std::string, wallet_key_ptr>::const_iterator const_iterator;
    std::map<std::string, wallet_key_ptr> keys;

public:
    wallet() {};

    const wallet_key *generate_key();

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
