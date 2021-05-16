#include "blockchain.h"

using namespace HGO::CHAIN;
using namespace HGO::EXCEPTION;

Blockchain::Blockchain()
{
    _events.run();
}

Blockchain::Blockchain(const BLOCK_LIST & blocks)
    : _chain(blocks)
{
    verify();
    _events.run();
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

    EVENTS::NewBlockEvent event;
    _events.dispatchEvent(event);
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

EVENTS::ChainEventManager &Blockchain::eventManager()
{
    return _events;
}

Blockchain & Blockchain::operator<<(const Block & blk)
{
    addBlock(blk);
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

bool Blockchain::requestTransaction(const HGO::TOKEN::Transaction & tx)
{
    _txBuffer.push_front(tx);
    
    EVENTS::NewTransactionEvent event;
    _events.dispatchEvent(event);

    _createTransactionBlock();
    return true;
}

void Blockchain::_createTransactionBlock()
{
    if(_txBuffer.size() == MIN_TX_PER_BLOCK)
    {
        std::string blk_data = "TXNs:";
        for(size_t i = 0; i < MIN_TX_PER_BLOCK; ++i)
        {
            std::string txSerialized = _txBuffer.back().serialize();
            blk_data += "tx:" + std::to_string(txSerialized.size()) + txSerialized;
            _txBuffer.pop_back();
        }
        Block newBlock(blk_data);
        addBlock(newBlock);
    }
}

Blockchain::T_LIST Blockchain::parseTransactions(const std::string &rawTx)
{
    
    T_LIST result;
    if(rawTx.substr(0,5) == "TXNs:")
    {
        std::istringstream input(rawTx);
        input.ignore(5);
        while(input)
        {
            input.ignore(3);
            
            std::size_t sz;
            input >> sz;
            
            if(input && sz > 0)
            {
                char * buffer = new char[sz + 1]{0};
                input.read(buffer, sz);
                HGO::TOKEN::Transaction tx = HGO::TOKEN::Transaction::unserialize(std::string(buffer));
                result.push_back(tx);
                delete[] buffer;
                buffer = nullptr;
            }
        }
    }
    return result;
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