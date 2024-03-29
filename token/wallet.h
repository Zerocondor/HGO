#ifndef __HGO_WALLET__
#define __HGO_WALLET__
#include <string>
#include <fstream>
#include <chrono>
#include <vector>
#include "../chain/blockchain.h"
#include "hgo_key.h"
#include "transaction.h"
#include <sha256/sha256.h>

namespace HGO::TOKEN
{
    class Wallet
    {
        using BLOCKCHAIN = HGO::CHAIN::Blockchain;
        using T_LIST = std::vector<Transaction>;

        public:
            constexpr static char const * ANONYMOUS_ADDRESS = "HGO000000000000000000ANONYMOUS";
            Wallet(BLOCKCHAIN & chain);
            
            virtual bool unlockWallet(const std::string & walletFile = "");
            virtual bool send(const std::string & walletAddress, long double amount = 0.0);
            virtual Transaction buildTransaction(const std::string & walletAddress, long double amount = 0.0) const;
            virtual void update(const HGO::CHAIN::Block & block);
            long double getBalance() const;
            std::string getAddress() const;
            std::string getToken() const;
            T_LIST getTransactions() const;
            HGO::CRYPTO::KeyPair getKeys() const;
            void signTransaction(Transaction & tx) const;

            virtual ~Wallet();
            
            static bool createWallet(const std::string walletFile);
        protected:
            virtual void _retrieveTransactions();

            const std::string TOKEN_NAME = "HGO";
            long double _balance;
            std::string _address;
            BLOCKCHAIN & _chain;
            T_LIST _transactions;
            
            HGO::CRYPTO::KeyPair _key;
    };


    std::ostream &operator<<(std::ostream &o, const Wallet & wallet);
    
}


#endif