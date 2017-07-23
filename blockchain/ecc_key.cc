#include "ecc_key.h"
#include <openssl/obj_mac.h>
#include <openssl/ecdsa.h>

ecc_key::ecc_key()
{
    pkey = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (pkey == NULL)
        throw std::runtime_error("ecc_key::ecc_key() : EC_KEY_new_by_curve_name failed");
}

ecc_key::ecc_key(const ecc_key &b)
{
    pkey = EC_KEY_dup(b.pkey);
    if (pkey == NULL)
        throw std::runtime_error("ecc_key::ecc_key(const ecc_key&) : EC_KEY_dup failed");
}

ecc_key &ecc_key::operator=(const ecc_key &b)
{
    if (!EC_KEY_copy(pkey, b.pkey))
        throw std::runtime_error("ecc_key::operator=(const ecc_key&) : EC_KEY_copy failed");
    return (*this);
}

ecc_key::~ecc_key()
{
    EC_KEY_free(pkey);
}

void ecc_key::generate()
{
    if (!EC_KEY_generate_key(pkey))
        throw std::runtime_error("ecc_key::generate() : EC_KEY_generate_key failed");
}

bool ecc_key::set_priv_key(const std::vector<unsigned char> &key)
{
    const unsigned char* pbegin = &key[0];
    if (!d2i_ECPrivateKey(&pkey, &pbegin, key.size()))
        return false;
    return true;
}
std::vector<unsigned char> ecc_key::get_priv_key() const
{
    unsigned int nSize = i2d_ECPrivateKey(pkey, NULL);
    if (!nSize)
        throw std::runtime_error("ecc_key::get_priv_key() : i2d_ECPrivateKey failed");
    std::vector<unsigned char> vchPrivKey(nSize, 0);
    unsigned char* pbegin = &vchPrivKey[0];
    if (i2d_ECPrivateKey(pkey, &pbegin) != nSize)
        throw std::runtime_error("ecc_key::get_priv_key() : i2d_ECPrivateKey returned unexpected size");
    return vchPrivKey;
}

bool ecc_key::set_pub_key(const std::vector<unsigned char> &key)
{
    const unsigned char* pbegin = &key[0];
    if (!o2i_ECPublicKey(&pkey, &pbegin, key.size()))
        return false;
    return true;
}

std::vector<unsigned char> ecc_key::get_pub_key() const
{
    unsigned int nSize = i2o_ECPublicKey(pkey, NULL);
    if (!nSize)
        throw std::runtime_error("ecc_key::get_pub_key() : i2o_ECPublicKey failed");
    std::vector<unsigned char> vchPubKey(nSize, 0);
    unsigned char* pbegin = &vchPubKey[0];
    if (i2o_ECPublicKey(pkey, &pbegin) != nSize)
        throw std::runtime_error("ecc_key::get_pub_key() : i2o_ECPublicKey returned unexpected size");
    return vchPubKey;
}

bool ecc_key::sign(const uint256 &hash, std::vector<unsigned char> &sig)
{
    sig.clear();
    unsigned char pchSig[10000];
    unsigned int nSize = 0;
    uint256 h = hash;
    if (!ECDSA_sign(0, (unsigned char*)&h, sizeof(hash), pchSig, &nSize, pkey))
        return false;
    sig.resize(nSize);
    memcpy(&sig[0], pchSig, nSize);
    return true;
}

bool ecc_key::verify(const uint256 &hash, const std::vector<unsigned char> &sig)
{
    // -1 = error, 0 = bad sig, 1 = good
    uint256 h = hash;
    if (ECDSA_verify(0, (unsigned char*)&h, sizeof(hash), &sig[0], sig.size(), pkey) != 1)
        return false;
    return true;
}

bool ecc_key::sign(const std::vector<unsigned char> &priv_key, const uint256 &hash, std::vector<unsigned char> &sig)
{
    ecc_key key;
    if (!key.set_priv_key(priv_key))
        return false;
    return key.sign(hash, sig);
}

bool ecc_key::verify(const std::vector<unsigned char> &pub_key, const uint256 &hash, const std::vector<unsigned char> &sig)
{
    ecc_key key;
    if (!key.set_pub_key(pub_key))
        return false;
    return key.verify(hash, sig);
}
