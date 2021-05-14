#ifndef __HGO_BLOCKCHAIN__
#define __HGO_BLOCKCHAIN__
#include <iostream>
#include <vector>
#include <stdexcept>
#include "block.h"

namespace HGO::CHAIN
{

    class Blockchain
    {
        friend std::ostream &operator<<(std::ostream&, const Blockchain &);

        public:
            using BLOCK_LIST = std::vector<Block>;
            
        public:
            Blockchain() = default;
            Blockchain(const BLOCK_LIST & blocks);
            void addBlock(Block blk);
            bool verify() const;
            Blockchain &operator<<(const Block & blk);
            const Block &getLastBlock() const;
            const BLOCK_LIST &getChain() const;

        protected:
            BLOCK_LIST _chain;
            
    };

    std::ostream &operator<<(std::ostream & o, const Blockchain &bc);
}

#endif