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
        if(it->getHash() != it->_calculateHash() || prevHash != it->_previous_hash)
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

void Blockchain::save(const std::string &path) const
{
    if(_chain.empty())
    {
        throw BlockchainException("Chain is Empty, cannot serialize it !");
    }

    std::ofstream file(path, std::ios_base::out | std::ios_base::binary);
    if(!file)
        throw BlockchainException("Unable to use this file to save");

    for(const auto & blk : _chain)
    {
        file<<blk.serialize()<<"\n";
    }
    file.close();
}

Blockchain Blockchain::load(const std::string &path)
{

    std::ifstream file(path,  std::ios_base::in | std::ios_base::binary);
    if(!file)
        throw BlockchainException("Unable to read the blockchain file : " + path);

    std::string block;
    Blockchain::BLOCK_LIST lst;
    while(std::getline(file, block))
    {
        lst.push_back(Block::unserialize(block));
    }
    file.close();
    return Blockchain(lst);
}

std::ostream & HGO::CHAIN::operator<<(std::ostream &o, const Blockchain & bc)
{
    for(const auto & block : bc._chain)
    {
        o<< block;
    }
    return o.flush();
}