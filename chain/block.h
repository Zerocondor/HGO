#ifndef __HGO_BLOCK__
#define __HGO_BLOCK__
#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <stdexcept>
#include <sha256/sha256.h>

namespace HGO::CHAIN
{
    class Blockchain;

    class Block
    {
        friend Blockchain;
        friend std::ostream &operator<<(std::ostream &, const Block &);

        public:
            using BLOCK_INDEX = unsigned long long;
            Block(const std::string &data);
            const std::string & getHash() const;
            inline BLOCK_INDEX getBlockID() const {return _idx;} 

        protected:

            std::string _calculateHash() const;

            BLOCK_INDEX _idx;
            std::string _data;
            std::string _hash;
            std::string _previous_hash;
            std::time_t _sTime;

    };

    std::ostream & operator<<(std::ostream & o, const Block &blk);

}



#endif