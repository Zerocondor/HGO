#include "block.h"
#include "../exceptions.h"

using namespace HGO::CHAIN;
using namespace HGO::EXCEPTION;

Block::Block(const std::string & data)
    :  _idx(0), _data(data), _hash(""), _previous_hash("")
{
    _sTime = std::time(nullptr);
}

const std::string & Block::getHash() const
{
    return _hash;
}

std::string Block::_calculateHash() const
{
    std::ostringstream oss;
    oss << _idx << _previous_hash << _sTime << _data;
    return sha256(oss.str());
}

const std::string &Block::getData() const
{
    return _data;
}

std::string Block::serialize() const
{
    std::ostringstream oss;
    oss<<BLOCK_HEADER
    <<"i:"<< _idx
    <<"t:"<<_sTime
    <<"p:"<<_previous_hash
    <<"h:"<<_hash
    <<"s:"<< _data.length()
    <<"d:"<<_data;
    return oss.str();
}

Block Block::unserialize(const std::string &serialized)
{
    if(serialized.substr(0, 5) != BLOCK_HEADER) {
        throw BlockchainException("Cannot unserialize this block, invalid header");
    }

    Block unserializedBlock;
    std::istringstream iss(std::string(serialized.cbegin() + 5, serialized.cend()));

    iss.ignore(2);
    iss >> unserializedBlock._idx;
    if(!iss)
    {
        throw BlockchainException("ID Block not found");
    }
    iss.ignore(2);
    iss >> unserializedBlock._sTime;
    if(!iss)
    {
        throw BlockchainException("Block time not found");
    }

    char prev_hash[65]{0};
    char hash[65]{0};
    char * data = nullptr;
    unsigned long sz;

    iss.ignore(2);
    if(unserializedBlock._idx > 0){
        iss.read(prev_hash, 64);
    }
    iss.ignore(2);
    iss.read(hash, 64);
    iss.ignore(2);
    iss>>sz;
    if(iss && sz)
    {
        iss.ignore(2);
        data = new char[sz + 1];
        memset(data, 0, sz + 1);
        iss.read(data, sz);
        unserializedBlock._data = std::string(data);
        delete[] data;
        data = nullptr;
    }

    unserializedBlock._previous_hash = prev_hash;
    unserializedBlock._hash = hash;

    return unserializedBlock;
}

std::ostream & HGO::CHAIN::operator<<(std::ostream & o, const HGO::CHAIN::Block &blk)
{
    o<<"========== Block N° "<<blk._idx<<" ========== \n"
    <<"\t |- Hash : " << blk._hash<<"\n"
    <<"\t |- PrevHash : " << blk._previous_hash<<"\n"
    <<"\t |- Data : " <<"\n"
    <<"\t\t" << blk._data <<"\n"
    <<"\t |- Time : "<<blk._sTime<<"\n";
    return o.flush();
}