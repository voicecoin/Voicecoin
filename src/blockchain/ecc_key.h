#ifndef BCUS_EDD_KEY_H
#define BCUS_EDD_KEY_H

#include <vector>
#include <openssl/ec.h>
#include "uint256.h"

// secp256k1:
// const unsigned int PRIVATE_KEY_SIZE = 279;
// const unsigned int PUBLIC_KEY_SIZE  = 65;
// const unsigned int SIGNATURE_SIZE   = 72;

class ecc_key
{
public:
    ecc_key();
    ecc_key(const ecc_key &b);
    ecc_key &operator=(const ecc_key &b);
    ~ecc_key();

    void generate();

    bool set_priv_key(const std::vector<unsigned char> &key);
    std::vector<unsigned char> get_priv_key() const;

    bool set_pub_key(const std::vector<unsigned char> &key);
    std::vector<unsigned char> get_pub_key() const;

    bool sign(const uint256 &hash, std::vector<unsigned char> &sig);
    bool verify(const uint256 &hash, const std::vector<unsigned char> &sig);

    static bool sign(const std::vector<unsigned char> &priv_key, const uint256 &hash, std::vector<unsigned char> &sig);
    static bool verify(const std::vector<unsigned char> &pub_key, const uint256 &hash, const std::vector<unsigned char> &sig);

private:
    EC_KEY* pkey;
};

#endif // BCUS_EDD_KEY_H
