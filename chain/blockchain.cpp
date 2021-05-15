#include "blockchain.h"

using namespace HGO::CHAIN;
using namespace HGO::EXCEPTION;

Blockchain::Blockchain(const BLOCK_LIST & blocks)
    : _chain(blocks)
{
    verify();
}

void Blockchain::addBlock(Block blk) {
    if(_chain.empty())
    {
        blk._idx = 0;
        blk._hash = blk._calculateHash();
        _chain.push_back(blk);
    } else {
        blk._idx = getLastBlock()._idx + 1;
        blk._previous_hash = getLastBlock().getHash();
        blk._hash = blk._calculateHash();
        _chain.push_back(blk);
    }
}

bool Blockchain::verify() const {
    BLOCK_LIST::const_iterator it = _chain.cbegin();
    std::string prevHash = "";
    while(it != _chain.cend())
    {         
        if(it->getHash() != it->_calculateHash() || prevHash == it->_previous_hash)
        {
            throw InconsistentChain(*it);
        }
        prevHash = it->getHash();  
        ++it;
    }
    return true;
}

Blockchain & Blockchain::operator<<(const Block & blk)
{
    this->addBlock(blk);
    return *this;
}

const Block &Blockchain::getLastBlock() const
{
    if(_chain.empty()) {
        throw BlockchainException("Try to get last block from an empty chain");
    }

    return _chain.back();
}

const Blockchain::BLOCK_LIST & Blockchain::getChain() const
{
    return _chain;
}

std::ostream & HGO::CHAIN::operator<<(std::ostream &o, const Blockchain & bc)
{
    for(const auto & block : bc._chain)
    {
        o<< block;
    }
    return o.flush();
}