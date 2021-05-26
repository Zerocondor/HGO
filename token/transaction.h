#ifndef __HGO_TRANSACTION__
#define __HGO_TRANSACTION__
#include <chrono>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>

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

        //For future implementation check if transaction from signature is valid return always true for the moment
        inline bool isValid() const { return true; } 

        inline std::string serialize() const {
            std::ostringstream oss;
            oss<<"c:"<<token<<"t:"<<time
            <<"a:"<<std::setprecision(std::numeric_limits<long double>::digits10 + 1)<<amount
            <<"f:"<<from<<"r:"<<to;
            return oss.str();
        }
        static inline Transaction unserialize(const std::string & serialized) {
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

            in.ignore(2).read(buffer, 30);
            tx.from = std::string(buffer);
            memset(buffer, 0, 30);
            in.ignore(2).read(buffer, 30);
            tx.to = std::string(buffer);
            memset(buffer, 0, 30);
            return tx;
        }
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