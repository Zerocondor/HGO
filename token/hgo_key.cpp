#include "hgo_key.h"

using namespace HGO::CRYPTO;

KeyPair::KeyPair()
{}

KeyPair::~KeyPair()
{
}

std::string KeyPair::getKey(const KEY_TYPE &type) const
{
    switch (type)
    {
        case  KEY_TYPE::PUBLIC:
            return getPublicKey();
            break;
        case  KEY_TYPE::PRIVATE:
            return getPrivateKey();
            break;
        default:
            return getPublicKey();
            break;
    }
}

std::string KeyPair::getPublicKey() const
{
    return  _pub_key;
}

std::string KeyPair::getPrivateKey() const
{
    return  _priv_key;
}

std::string KeyPair::getRawPublicKey() const
{
    
    EC_KEY * k = nullptr;
    BIO * bio = BIO_new(BIO_s_mem());
    BIO_write(bio, getPublicKey().c_str(), getPublicKey().size());
    BIO_seek(bio, 0);
    PEM_read_bio_EC_PUBKEY(bio, &k, nullptr, nullptr);
    BIO_free(bio);

    const EC_GROUP * grp = EC_KEY_get0_group(k);
    
    const EC_POINT * curvePoint = EC_KEY_get0_public_key(k);
    unsigned char buffer[2048];
    int sz = EC_POINT_point2oct(grp, curvePoint, POINT_CONVERSION_COMPRESSED, buffer, 2048, nullptr);
    
    EC_KEY_free(k);

    return std::string(reinterpret_cast<char *>(buffer), sz);
}

std::string KeyPair::sign(const std::string & data) const
{
    EC_KEY * k = EC_KEY_new();
    //_loadKey(&k, getPublicKey(), KEY_TYPE::PUBLIC);
    _loadKey(&k, getPrivateKey(), KEY_TYPE::PRIVATE);
    if(EC_KEY_can_sign(k) == 0)
    {
        EC_KEY_free(k);
        return "";
    }

    unsigned int sz = ECDSA_size(k);
    unsigned char * buffer = new unsigned char[sz];

    if(ECDSA_sign(0,reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), buffer, &sz, k) == 1)
    {
        std::string sig(reinterpret_cast<char *>(buffer), sz);
        delete[] buffer;
        EC_KEY_free(k);
        return sig;
    }
    EC_KEY_free(k);
    return "";
}

bool KeyPair::verify(const std::string & data, const std::string & signature) const
{
     EC_KEY * k = EC_KEY_new();
    _loadKey(&k, getPublicKey(), KEY_TYPE::PUBLIC);
    //_loadKey(&k, getPrivateKey(), KEY_TYPE::PRIVATE);
    bool verified = ECDSA_verify(0, reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), 
                    reinterpret_cast<const unsigned char*>(signature.c_str()), signature.size(), k) == 1;
    
    EC_KEY_free(k);
    return verified;
}

bool KeyPair::saveFile(const std::string &filename, const KEY_TYPE &type) const
{
    std::ofstream out(filename, std::ios_base::out | std::ios_base::binary);
    if(out)
    {
        out << getKey(type);
        out.close();
        return true;
    }
    return false;
}

bool KeyPair::loadFromFile(KeyPair & key, const std::string &filename, const KEY_TYPE &type)
{
    std::ifstream in(filename, std::ios_base::binary);
    if(in)
    {
        std::string sKey;
        if(in>>sKey)
        {
            return KeyPair::loadFromPEM(key, sKey, type);
        }
    }
    return false;
    
}

bool KeyPair::loadFromPEM(KeyPair & key, const std::string &pemStr, const KEY_TYPE &type)
{
    if(type == KEY_TYPE::PUBLIC) {
        key._pub_key = pemStr;
    } else {
        key._priv_key = pemStr;
        return !key.sign("Hugo").empty();
    }

    return true;
}

bool KeyPair::loadFromRAWPublicKey(KeyPair & key, const std::string &raw)
{
    EC_GROUP * grp = EC_GROUP_new_by_curve_name(NID_secp256k1);
    EC_POINT * curvePoint = EC_POINT_new(grp);
    EC_KEY * eckey = EC_KEY_new();
    EC_KEY_set_group(eckey, grp);
    BN_CTX *ctx = BN_CTX_new();
    assert(EC_POINT_oct2point(grp, curvePoint, reinterpret_cast<const unsigned char *>(raw.c_str()), raw.size() , ctx));
    assert(EC_KEY_set_public_key(eckey, curvePoint));

    BIO * bio = BIO_new(BIO_s_mem());
    PEM_write_bio_EC_PUBKEY(bio, eckey);
    char * buffer = nullptr;
    long sz = BIO_get_mem_data(bio, &buffer);

    std::string pemFormat(buffer, sz);
    
    BN_CTX_free(ctx);
    BIO_free(bio);
    EC_GROUP_free(grp);
    EC_POINT_free(curvePoint);

    return loadFromPEM(key, pemFormat, KEY_TYPE::PUBLIC);
}


KeyPair KeyPair::generate()
{
    EC_KEY * key = EC_KEY_new();
    EC_GROUP *grp = EC_GROUP_new_by_curve_name(NID_secp256k1);
    EC_KEY_set_group(key, grp);
    EC_KEY_generate_key(key);
    

    BIO *b = BIO_new(BIO_s_mem());
    PEM_write_bio_ECPrivateKey(b, key, nullptr, nullptr,0, nullptr, nullptr);

    char * buffer = nullptr;
    long size = BIO_get_mem_data(b, &buffer);
    std::string pKey(buffer, size);

    BIO_reset(b);
    PEM_write_bio_EC_PUBKEY(b, key);
    size = BIO_get_mem_data(b, &buffer);
    std::string pub(buffer, size);

    BIO_free(b);
    EC_GROUP_free(grp);
    EC_KEY_free(key);

    KeyPair pair;
    pair._pub_key = pub;
    pair._priv_key = pKey;
    return pair; 
}

void KeyPair::_loadKey(EC_KEY ** k, const std::string &pem, const KEY_TYPE &type) const
{
    BIO * bio = BIO_new(BIO_s_mem());
    BIO_write(bio, pem.c_str(), pem.size());
    BIO_seek(bio, 0);
    if(type == KEY_TYPE::PUBLIC) {
        PEM_read_bio_EC_PUBKEY(bio, k, nullptr, nullptr);
    } else {
        PEM_read_bio_ECPrivateKey(bio, k, nullptr, nullptr);
    }
    BIO_free(bio);
}