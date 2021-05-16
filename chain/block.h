#ifndef __HGO_BLOCK__
#define __HGO_BLOCK__
#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <sha256/sha256.h>
#include <cstring>


namespace HGO::CHAIN
{
    class Blockchain;

    class Block
    {
        friend Blockchain;
        friend std::ostream &operator<<(std::ostream &, const Block &);
        static constexpr char const * BLOCK_HEADER = "<<BLK";

        public:
            using BLOCK_INDEX = unsigned long long;
            
            Block() = default;
            Block(const std::string &data);
            const std::string & getHash() const;
            const std::string & getData() const;
            inline BLOCK_INDEX getBlockID() const {return _idx;}
            std::string serialize() const;
            static Block unserialize(const std::string &serialized); 

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