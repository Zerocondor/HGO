#include "transaction.h"
#include <sha256/sha256.h>

using namespace HGO::TOKEN;

//For future implementation check if transaction from signature is valid return always true for the moment
bool Transaction::isValid() const { 
    if(signature.empty() || public_key.empty())
        return false;
    
    HGO::CRYPTO::KeyPair keys;
    HGO::CRYPTO::KeyPair::loadFromRAWPublicKey(keys, public_key);
    if(from == sha256(keys.getRawPublicKey()))
    {
        return keys.verify(getHash(), signature);
    } else {
        return false;
    }
    return false;
}


std::string Transaction::getHash() const
{
    std::ostringstream oss;
    oss << from << to << token << amount;
    return sha256(oss.str());
}

std::string Transaction::serialize() const {
    std::ostringstream oss;
    oss<<"c:"<<token<<"t:"<<time
    <<"a:"<<std::setprecision(std::numeric_limits<long double>::digits10 + 1)<<amount
    <<"f:"<<from<<"r:"<<to
    <<"k:";
    std::ios_base::fmtflags fl = oss.flags();
   
    for(const unsigned char &c : public_key)
    {
        oss<<std::hex<<std::setfill('0')<<std::setw(2)<<(int)c;
    }
    oss.flags(fl);   
    oss<<"s:";

    for(const unsigned char &c : signature)
    {
        oss<<std::hex<<std::setfill('0')<<std::setw(2)<<(int)c;
    }
    oss.flags(fl);
    return oss.str();
}

Transaction Transaction::unserialize(const std::string & serialized) {
    std::istringstream in(serialized);
    Transaction tx;
    char buffer[200]{0};
    in.ignore(2).read(buffer, 3);
    tx.token = std::string(buffer);
    memset(buffer, 0, 3);
    in.ignore(2);
    in >> tx.time;

    in.ignore(2);
    in>> tx.amount;

    in.ignore(2).read(buffer, 64);
    tx.from = std::string(buffer, 64);
    memset(buffer, 0, 64);

    in.ignore(2).read(buffer, 64);
    tx.to = std::string(buffer, 64);
    memset(buffer, 0, 64);

    in.ignore(2).read(buffer, 66);
    tx.public_key = "";
 
    for(int i = 0; i < 33; ++i)
    {
        unsigned char byte = std::stoi(std::string(buffer + (i * 2),buffer + (i * 2) + 2),0, 16);
        tx.public_key += byte;
    }
    std::cout.flush();
    memset(buffer, 0, 66);

    in.ignore(3, ':');
    std::string signature;
    in>>signature;
    tx.signature = "";
    std::cout.flush();
    for(int i = 0; i < signature.size() / 2; ++i)
    {
        std::string hex(signature.cbegin() + (i * 2),signature.cbegin() + (i * 2) + 2);
        unsigned char byte = std::stoi(hex,0,16);
        tx.signature += byte;
    }
    
    return tx;
}