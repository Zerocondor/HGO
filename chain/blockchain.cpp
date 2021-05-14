#include "blockchain.h"

using namespace HGO::CHAIN;

Blockchain::Blockchain(const BLOCK_LIST & blocks)
    : _chain(blocks)
{
    if(!verify())
    {
        throw std::logic_error("Chain is inconsistent");
    }
}

void Blockchain::addBlock(Block blk) {
    if(_chain.empty())
    {
        blk._idx = 0;
        blk._calculateHash();
        _chain.push_back(blk);
    } else {
        blk._idx = getLastBlock()._idx + 1;
        blk._previous_hash = getLastBlock().getHash();
        blk._calculateHash();
        _chain.push_back(blk);
    }
}

bool Blockchain::verify() const {
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
        throw std::logic_error("Chain is empty");
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