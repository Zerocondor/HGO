#ifndef __HGO_WALLET__
#define __HGO_WALLET__
#include <string>

namespace HGO::TOKEN
{
    class Wallet
    {
        public:
            Wallet() = default;
            
            bool unlockWallet(const std::string & walletFile = "");
            bool send(const std::string & walletAddress, long double amount = 0.0);

        protected:
            long double _balance;
            std::string _address;

    };
}


#endif