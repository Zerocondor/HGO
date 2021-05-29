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
        false;
    }
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
    oss.write(public_key.c_str(), public_key.size());
    oss<<"s:"<<signature.size()<<" ";
    oss.write(signature.c_str(), signature.size());
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

    in.ignore(2).read(buffer, 32);
    tx.from = std::string(buffer, 32);
    memset(buffer, 0, 32);

    in.ignore(2).read(buffer, 32);
    tx.to = std::string(buffer, 32);
    memset(buffer, 0, 32);

    in.ignore(2).read(buffer, 33);
    tx.public_key = std::string(buffer, 33);
    memset(buffer, 0, 33);

    in.ignore(2);
    std::string::size_type sz;
    in>> sz;
    in.get(buffer,sz);
    tx.signature = std::string(buffer, sz);

    return tx;
}