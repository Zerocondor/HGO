#include "block.h"


HGO::CHAIN::Block::Block(const std::string & data)
    :  _idx(0), _data(data), _hash(""), _previous_hash("")
{
    _sTime = std::time(nullptr);
}

const std::string & HGO::CHAIN::Block::getHash() const
{
    return _hash;
}

std::string HGO::CHAIN::Block::_calculateHash() const
{
    std::ostringstream oss;
    oss << _idx << _previous_hash << _sTime << _data;
    return sha256(oss.str());
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