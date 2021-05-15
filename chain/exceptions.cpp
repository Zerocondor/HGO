#include "exceptions.h"

using namespace HGO::EXCEPTION;

BlockchainException::BlockchainException(const std::string &m) 
: m_what(m)
{}

const char * BlockchainException::what() const noexcept
{
    return m_what.c_str();
}

BlockchainException::~BlockchainException() noexcept {}

InconsistentChain::InconsistentChain(const HGO::CHAIN::Block &blk)
{
    std::ostringstream oss;
    oss<< "Chain is inconsistent from this Block \n"
    <<"\t Block ID : "<<blk.getBlockID() << " with hash : "<<blk.getHash();
    m_what = oss.str();
}