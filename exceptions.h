#ifndef __HGO_EXCEPTION__
#define __HGO_EXCEPTION__
#include <string>
#include <stdexcept>
#include <exception>
#include <sstream>
#include <errno.h>

#include "chain/block.h"
#include "token/wallet.h"

namespace HGO::EXCEPTION
{
    
    class BlockchainException : public std::exception
    {
        public:
            BlockchainException() = default;
            explicit BlockchainException(const std::string &s);
            virtual const char * what() const noexcept override;
            virtual ~BlockchainException() noexcept;
        protected:
            std::string m_what;

    };

    class InconsistentChain : public BlockchainException
    {
        public:
            InconsistentChain(const HGO::CHAIN::Block &blk);
    };

    class WalletError : public BlockchainException
    {
        public:
            WalletError(const std::string &msg);
    };

    class ProtocolError : public BlockchainException
    {
        public:
            ProtocolError(const int & _errno, const std::string &msg = "");
    };

    class P2PError : public BlockchainException
    {
        public:
            P2PError(const std::string &msg);
    };

}


#endif