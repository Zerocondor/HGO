#ifndef __HGO_EXCEPTION__
#define __HGO_EXCEPTION__
#include <string>
#include <stdexcept>
#include <exception>
#include <sstream>
#include "block.h"

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

}


#endif