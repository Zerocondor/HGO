#ifndef __HGO_TRANSACTION__
#define __HGO_TRANSACTION__
#include <chrono>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>
#include "hgo_key.h"


namespace HGO::TOKEN
{
    struct Transaction
    {
        enum Direction { IN, OUT } ;
        std::time_t time;
        std::string from;
        std::string to;
        std::string token;
        long double amount;

        std::string signature;
        std::string public_key;
                //For future implementation check if transaction from signature is valid return always true for the moment
        bool isValid() const;
        std::string getHash() const;
        std::string serialize() const;
        static Transaction unserialize(const std::string & serialized);
    };

    inline std::ostream &operator<<(std::ostream &o, const Transaction & transac)
    {
        o<<"Transaction :\n"
        <<"\t Time: " << transac.time <<"\n"
        <<"\t From: " << transac.from <<"\n"
        <<"\t To: " << transac.to<<"\n"
        <<"\t Amount: " 
        << std::setprecision(std::numeric_limits<long double>::digits10 + 1)<<transac.amount
        <<" " << transac.token<<"\n"<<std::resetiosflags(std::ios_base::showbase);
        return o.flush();
    }

}


#endif