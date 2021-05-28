#ifndef __HGO_KEY__
#define __HGO_KEY__

#include <string>
#include <fstream>
#include <memory>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/ecdsa.h>

namespace HGO::CRYPTO
{
    class KeyPair
    {
        
        public:
            enum class KEY_TYPE {
                PUBLIC,
                PRIVATE
            };
            KeyPair();
            ~KeyPair();
            std::string getKey(const KEY_TYPE &type) const;
            std::string getPublicKey() const;
            std::string getPrivateKey() const;
            std::string sign(const std::string & data) const;
            bool verify(const std::string & data, const std::string & signature) const;

            bool saveFile(const std::string &filename, const KEY_TYPE &type) const;

            static bool loadFromFile(KeyPair & key, const std::string &filename, const KEY_TYPE &type);
            static KeyPair generate();
        protected:
            void _loadKey(EC_KEY ** k, const std::string &pem, const KEY_TYPE &type) const;
            std::string _pub_key;
            std::string _priv_key;

    };
}

#endif